#include "reservation_engine.h"

#include <algorithm>
#include <limits>
#include <random>

#include "flight_manager.h"
#include "inventory_service.h"
#include "revenue_service.h"
#include "validator.h"

namespace {
std::map<std::string, ReservationRecord> g_reservations;
long long g_recorded_revenue = 0;
constexpr int kMaxPnrGenerationAttempts = 10;

std::string generate_pnr() {
    static const char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 35);
    std::string pnr;
    pnr.reserve(6);
    for (int i = 0; i < 6; ++i) {
        pnr.push_back(kAlphabet[dist(rng)]);
    }
    return pnr;
}

std::string generate_seat_number(const std::string& flight_id, SeatClass seat_class) {
    static std::map<std::string, int> counters;
    std::string prefix = "E";
    if (seat_class == SeatClass::Business) {
        prefix = "B";
    } else if (seat_class == SeatClass::First) {
        prefix = "F";
    }
    int& counter = counters[flight_id + prefix];
    ++counter;
    return prefix + std::to_string(counter);
}

Status record_revenue(const Money& amount) {
    if (amount.amount_cents > 0 && g_recorded_revenue > std::numeric_limits<long long>::max() - amount.amount_cents) {
        return make_failure("REVENUE_OVERFLOW", "Revenue total overflow");
    }
    g_recorded_revenue += amount.amount_cents;
    return make_success();
}
}

BookingResult create_booking(const BookingRequest& request) {
    if (request.flight_id.empty()) {
        return {"", {0, "USD"}, make_failure("BOOKING_FLIGHT_MISSING", "Flight ID is required")};
    }
    if (request.passenger.first_name.empty() || request.passenger.last_name.empty()) {
        return {"", {0, "USD"}, make_failure("BOOKING_PASSENGER_MISSING", "Passenger name is required")};
    }
    if (!is_valid_passport(request.passenger.passport_number)) {
        return {"", {0, "USD"}, make_failure("BOOKING_PASSPORT_INVALID", "Invalid passport number")};
    }

    const Flight* flight = find_flight(request.flight_id);
    if (flight == nullptr) {
        return {"", {0, "USD"}, make_failure("BOOKING_FLIGHT_UNKNOWN", "Flight not found")};
    }

    std::string seat_number = request.seat_number;
    if (!seat_number.empty() && !is_valid_seat_number(seat_number)) {
        return {"", {0, "USD"}, make_failure("BOOKING_SEAT_INVALID", "Invalid seat number format")};
    }
    if (!seat_number.empty() && is_seat_taken(request.flight_id, seat_number)) {
        return {"", {0, "USD"}, make_failure("BOOKING_SEAT_TAKEN", "Seat already occupied")};
    }

    Status status = check_availability(request.flight_id, request.preferred_class);
    if (!status.success) {
        return {"", {0, "USD"}, status};
    }

    InventoryUpdate update{request.flight_id, 0, 0, 0};
    if (request.preferred_class == SeatClass::Economy) {
        update.economy_delta = -1;
    } else if (request.preferred_class == SeatClass::Business) {
        update.business_delta = -1;
    } else {
        update.first_delta = -1;
    }

    status = update_inventory(update);
    if (!status.success) {
        return {"", {0, "USD"}, status};
    }

    InventorySnapshot snapshot = get_inventory_snapshot(request.flight_id);
    int remaining = snapshot.economy_available;
    int capacity = snapshot.economy_capacity;
    if (request.preferred_class == SeatClass::Business) {
        remaining = snapshot.business_available;
        capacity = snapshot.business_capacity;
    } else if (request.preferred_class == SeatClass::First) {
        remaining = snapshot.first_available;
        capacity = snapshot.first_capacity;
    }

    Status pricing_status = make_success();
    Money total_cost = calculate_dynamic_price(flight->base_price, remaining, capacity, pricing_status);
    if (!pricing_status.success) {
        update_inventory({request.flight_id,
                          update.economy_delta * -1,
                          update.business_delta * -1,
                          update.first_delta * -1});
        return {"", {0, "USD"}, pricing_status};
    }

    std::string pnr;
    bool unique = false;
    for (int attempts = 0; attempts < kMaxPnrGenerationAttempts; ++attempts) {
        pnr = generate_pnr();
        if (g_reservations.find(pnr) == g_reservations.end()) {
            unique = true;
            break;
        }
    }
    if (!unique) {
        update_inventory({request.flight_id,
                          update.economy_delta * -1,
                          update.business_delta * -1,
                          update.first_delta * -1});
        return {"", {0, "USD"}, make_failure("BOOKING_PNR_COLLISION", "Unable to generate unique PNR")};
    }

    if (seat_number.empty()) {
        seat_number = generate_seat_number(request.flight_id, request.preferred_class);
    }

    status = reserve_seat(request.flight_id, seat_number);
    if (!status.success) {
        update_inventory({request.flight_id,
                          update.economy_delta * -1,
                          update.business_delta * -1,
                          update.first_delta * -1});
        return {"", {0, "USD"}, status};
    }

    BookingRequest stored_request = request;
    stored_request.seat_number = seat_number;

    ReservationRecord record{pnr, stored_request, total_cost, ReservationStatus::Reserved};
    g_reservations.emplace(pnr, record);
    Status revenue_status = record_revenue(total_cost);
    if (!revenue_status.success) {
        return {"", {0, "USD"}, revenue_status};
    }

    return {pnr, total_cost, make_success()};
}

const ReservationRecord* find_reservation(const std::string& pnr_id) {
    auto it = g_reservations.find(pnr_id);
    if (it == g_reservations.end()) {
        return nullptr;
    }
    return &it->second;
}

Status update_reservation_status(const std::string& pnr_id, ReservationStatus new_status) {
    auto it = g_reservations.find(pnr_id);
    if (it == g_reservations.end()) {
        return make_failure("RESERVATION_NOT_FOUND", "Reservation not found");
    }
    it->second.status = new_status;
    return make_success();
}

std::vector<ReservationRecord> get_all_reservations() {
    std::vector<ReservationRecord> records;
    records.reserve(g_reservations.size());
    for (const auto& entry : g_reservations) {
        records.push_back(entry.second);
    }
    return records;
}

std::vector<Money> get_booking_totals() {
    std::vector<Money> totals;
    totals.reserve(g_reservations.size());
    for (const auto& entry : g_reservations) {
        totals.push_back(entry.second.total_cost);
    }
    return totals;
}

int get_total_reservations() {
    return static_cast<int>(g_reservations.size());
}

long long get_recorded_revenue() {
    return g_recorded_revenue;
}