/*
Yonas Dereje
ETS1558/17
*/
#pragma once

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
    Passenger   passenger;
    SeatClass   preferred_class;
    std::string seat_number;
};

struct BookingResult {
    std::string pnr_id;
    Money       total_cost;
    Status      status;
};

enum class ReservationStatus {
    Reserved,
    CheckedIn
};

struct ReservationRecord {
    std::string       pnr_id;
    BookingRequest    request;
    Money             total_cost;
    ReservationStatus status;
};

BookingResult              book_flight(const BookingRequest& request);
const ReservationRecord*   find_reservation(const std::string& pnr_id);
Status                     mark_checked_in(const std::string& pnr_id);
int                        get_total_reservations();
long long                  get_recorded_revenue();
