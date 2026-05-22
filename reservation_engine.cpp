/*
Yonas Dereje
ETS1558/17
*/

#include "reservation_engine.h"

#include <limits>
#include <random>
#include <unordered_map>

#include "flight_manager.h"
#include "inventory_service.h"
#include "revenue_service.h"


namespace {
std::unordered_map<std::string, ReservationRecord> g_reservations;
long long g_recorded_revenue = 0;

// Generates a random 6-character alphanumeric PNR.
std::string generate_pnr() {
    static const char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 35);
    std::string pnr;
    for (int i = 0; i < 6; ++i) {
        pnr.push_back(kAlphabet[dist(rng)]);
    }
    return pnr;
}

// Assigns sequential seat numbers based on flight and class (E1, E2, B1, F1, ...).
std::string generate_seat_number(const std::string& flight_id, SeatClass seat_class) {
    static std::unordered_map<std::string, int> counters;
    std::string prefix = "E";
    if (seat_class == SeatClass::Business) prefix = "B";
    else if (seat_class == SeatClass::First) prefix = "F";
    return prefix + std::to_string(++counters[flight_id + prefix]);
}

// Records booking revenue with overflow protection.
Status record_revenue(const Money& amount) {
    if (amount.amount_cents > 0 &&
        g_recorded_revenue > std::numeric_limits<long long>::max() - amount.amount_cents) {
        return make_failure("Revenue total overflow");
    }
    g_recorded_revenue += amount.amount_cents;
    return make_success();
}
}

// Creates a new flight booking and reservation record.
BookingResult book_flight(const BookingRequest& request) {
    const Flight* flight = find_flight(request.flight_id);
    std::string seat_number = request.seat_number;

    Status status = check_availability(request.flight_id, request.preferred_class);
    if (!status.success) {
        return {"", {0, "USD"}, status};
    }

    InventoryUpdate update{request.flight_id, 0, 0, 0};
    if (request.preferred_class == SeatClass::Economy)       update.economy_delta  = -1;
    else if (request.preferred_class == SeatClass::Business) update.business_delta = -1;
    else                                                      update.first_delta    = -1;

    status = update_inventory(update);
    if (!status.success) {
        return {"", {0, "USD"}, status};
    }

    InventorySnapshot snapshot = get_inventory_snapshot(request.flight_id);
    int remaining = snapshot.economy_available;
    int capacity  = snapshot.economy_capacity;
    if (request.preferred_class == SeatClass::Business) {
        remaining = snapshot.business_available;
        capacity  = snapshot.business_capacity;
    } else if (request.preferred_class == SeatClass::First) {
        remaining = snapshot.first_available;
        capacity  = snapshot.first_capacity;
    }

    Status pricing_status = make_success();
    Money total_cost = calculate_dynamic_price(flight->base_price, remaining, capacity, pricing_status);
    if (!pricing_status.success) {
        update_inventory({request.flight_id,
                          update.economy_delta  * -1,
                          update.business_delta * -1,
                          update.first_delta    * -1});
        return {"", {0, "USD"}, pricing_status};
    }

    // With 36^6 (~2.1 billion) possible PNRs, collision is negligible at this scale.
    const std::string pnr = generate_pnr();
    if (g_reservations.count(pnr) > 0) {
        update_inventory({request.flight_id,
                          update.economy_delta  * -1,
                          update.business_delta * -1,
                          update.first_delta    * -1});
        return {"", {0, "USD"}, make_failure("Unable to generate unique PNR")};
    }

    if (seat_number.empty()) {
        seat_number = generate_seat_number(request.flight_id, request.preferred_class);
    }

    status = reserve_seat(request.flight_id, seat_number);
    if (!status.success) {
        update_inventory({request.flight_id,
                          update.economy_delta  * -1,
                          update.business_delta * -1,
                          update.first_delta    * -1});
        return {"", {0, "USD"}, status};
    }

    BookingRequest stored_request = request;
    stored_request.seat_number = seat_number;

    g_reservations.emplace(pnr, ReservationRecord{pnr, stored_request, total_cost, ReservationStatus::Reserved});

    Status revenue_status = record_revenue(total_cost);
    if (!revenue_status.success) {
        return {"", {0, "USD"}, revenue_status};
    }

    return {pnr, total_cost, make_success()};
}

// Finds a reservation record by PNR.
const ReservationRecord* find_reservation(const std::string& pnr_id) {
    auto it = g_reservations.find(pnr_id);
    if (it == g_reservations.end()) return nullptr;
    return &it->second;
}

// Marks a reservation as checked in.
Status mark_checked_in(const std::string& pnr_id) {
    auto it = g_reservations.find(pnr_id);
    if (it == g_reservations.end()) {
        return make_failure("Reservation not found");
    }
    it->second.status = ReservationStatus::CheckedIn;
    return make_success();
}

// Returns the total number of reservations.
int get_total_reservations() {
    return static_cast<int>(g_reservations.size());
}

// Returns the total recorded revenue in cents.
long long get_recorded_revenue() {
    return g_recorded_revenue;
}
