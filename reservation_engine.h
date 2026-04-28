#pragma once

#include <map>
#include <string>
#include <vector>

#include "common_types.h"

struct Passenger {
    std::string first_name;
    std::string last_name;
    std::string passport_number;
};

struct BookingRequest {
    std::string flight_id;
    Passenger passenger;
    SeatClass preferred_class;
    std::string seat_number;
};

struct BookingResult {
    std::string pnr_id;
    Money total_cost;
    Status status;
};

enum class ReservationStatus {
    Reserved,
    CheckedIn,
    Boarded
};

struct ReservationRecord {
    std::string pnr_id;
    BookingRequest request;
    Money total_cost;
    ReservationStatus status;
};

BookingResult create_booking(const BookingRequest& request);
const ReservationRecord* find_reservation(const std::string& pnr_id);
Status update_reservation_status(const std::string& pnr_id, ReservationStatus new_status);
std::vector<ReservationRecord> get_all_reservations();
std::vector<Money> get_booking_totals();
int get_total_reservations();
long long get_recorded_revenue();