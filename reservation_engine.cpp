/*
Yonas Dereje
ETS1558/17
*/
#include "reservation_engine.h"
#include "flight_manager.h"
#include "revenue_service.h"
#include <cstdlib>
#include <ctime>
#include <string>

static ReservationLog g_log;

// ─── ReservationLog member definitions ───────────────────────────────────────

ReservationLog::ReservationLog() {
    total_revenue  = 0;
    total_checkins = 0;
}

void ReservationLog::add(const ReservationRecord& record) {
    records.push_back(record);
    total_revenue += record.total_cost.amount_cents;
}

ReservationRecord* ReservationLog::find(const std::string& pnr) {
    for (int i = 0; i < (int)records.size(); i++) {
        if (records[i].pnr_id == pnr) {
            return &records[i];
        }
    }
    return nullptr;
}

Status ReservationLog::check_in(const std::string& pnr) {
    ReservationRecord* rec = find(pnr);
    if (rec == nullptr) {
        return make_failure("Reservation not found");
    }
    if (rec->status != ReservationStatus::Reserved) {
        return make_failure("Passenger is already checked in");
    }
    rec->status = ReservationStatus::CheckedIn;
    total_checkins++;
    return make_success();
}

int ReservationLog::count() {
    return (int)records.size();
}

// ─── File-scope helpers ───────────────────────────────────────────────────────

static std::string generate_pnr() {
    const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string pnr;
    for (int i = 0; i < 6; i++) {
        pnr += alphabet[rand() % 36];
    }
    return pnr;
}

static std::string assign_seat(const std::string& flight_id, SeatClass seat_class) {
    std::string prefix = "E";
    if (seat_class == SeatClass::Business) {
        prefix = "B";
    } else if (seat_class == SeatClass::First) {
        prefix = "F";
    }
    int count = 0;
    for (int i = 0; i < (int)g_log.records.size(); i++) {
        if (g_log.records[i].request.flight_id == flight_id &&
            g_log.records[i].request.preferred_class == seat_class) {
            count++;
        }
    }
    return prefix + std::to_string(count + 1);
}

// ─── Global API functions ─────────────────────────────────────────────────────

BookingResult book_flight(const BookingRequest& request) {
    // 1. Locate the flight
    Flight* flight = find_flight(request.flight_id);
    if (flight == nullptr) {
        return {"", {0, "USD"}, make_failure("Flight not found")};
    }

    // 2. Check seat availability
    int remaining = 0;
    int capacity  = 0;
    if (request.preferred_class == SeatClass::Economy) {
        remaining = flight->economy_seats;
        capacity  = flight->economy_capacity;
    } else if (request.preferred_class == SeatClass::Business) {
        remaining = flight->business_seats;
        capacity  = flight->business_capacity;
    } else {
        remaining = flight->first_seats;
        capacity  = flight->first_capacity;
    }
    if (remaining <= 0) {
        return {"", {0, "USD"}, make_failure("No seats available in this class")};
    }

    // 3. Calculate dynamic price
    Status pricing_status = make_success();
    Money total_cost = calculate_dynamic_price(flight->base_price, remaining, capacity, pricing_status);
    if (!pricing_status.success) {
        return {"", {0, "USD"}, pricing_status};
    }

    // 4. Generate a unique PNR
    std::string pnr = generate_pnr();
    int tries = 0;
    while (g_log.find(pnr) != nullptr && tries < 10) {
        pnr = generate_pnr();
        tries++;
    }
    if (g_log.find(pnr) != nullptr) {
        return {"", {0, "USD"}, make_failure("Could not generate unique PNR")};
    }

    // 5. Assign seat if not provided
    std::string seat = request.seat_number;
    if (seat.empty()) {
        seat = assign_seat(request.flight_id, request.preferred_class);
    }

    // 6. Decrement seat inventory
    Status inv = decrement_seat(request.flight_id, request.preferred_class);
    if (!inv.success) {
        return {"", {0, "USD"}, inv};
    }

    // 7. Persist the booking
    BookingRequest stored   = request;
    stored.seat_number      = seat;
    ReservationRecord record;
    record.pnr_id           = pnr;
    record.request          = stored;
    record.total_cost       = total_cost;
    record.status           = ReservationStatus::Reserved;
    g_log.add(record);

    // 8. Return success
    return {pnr, total_cost, make_success()};
}

const ReservationRecord* find_reservation(const std::string& pnr_id) {
    return g_log.find(pnr_id);
}

Status mark_checked_in(const std::string& pnr_id) {
    return g_log.check_in(pnr_id);
}

int get_total_reservations() {
    return g_log.count();
}

long long get_recorded_revenue() {
    return g_log.total_revenue;
}

int get_total_checkins() {
    return g_log.total_checkins;
}

ReservationLog& get_reservation_log() {
    return g_log;
}
