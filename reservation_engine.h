/*
Yonas Dereje
ETS1558/17
*/
#pragma once
#include <string>
#include <vector>
#include "common_types.h"

// Passenger: personal identity details of the traveler
struct Passenger {
    std::string first_name;
    std::string last_name;
    std::string passport_number;
};

// BookingRequest: all input data needed to make a flight reservation
struct BookingRequest {
    std::string flight_id;
    Passenger   passenger;
    SeatClass   preferred_class;
    std::string seat_number;   // empty string = auto-assign
};

// BookingResult: what the system returns after attempting a booking
struct BookingResult {
    std::string pnr_id;
    Money       total_cost;
    Status      status;
};

// ReservationStatus: lifecycle state of a reservation
enum class ReservationStatus {
    Reserved,
    CheckedIn
};

// ReservationRecord: one complete booking stored in the system
struct ReservationRecord {
    std::string       pnr_id;
    BookingRequest    request;
    Money             total_cost;
    ReservationStatus status;
};

// ─── THE CLASS ───────────────────────────────────────────────────────────────
// ReservationLog: holds all reservations and running totals.
// This is the core class of the reservation system.
class ReservationLog {
public:
    std::vector<ReservationRecord> records;         // all bookings
    long long                      total_revenue;   // sum of all booking costs in cents
    int                            total_checkins;  // number of passengers checked in

    ReservationLog();                                            // constructor
    void               add(const ReservationRecord& record);    // add a new booking
    ReservationRecord* find(const std::string& pnr);            // search by PNR
    Status             check_in(const std::string& pnr);        // mark as checked in
    int                count();                                  // total reservations
};

// ─── Global API functions ────────────────────────────────────────────────────
BookingResult            book_flight(const BookingRequest& request);
const ReservationRecord* find_reservation(const std::string& pnr_id);
Status                   mark_checked_in(const std::string& pnr_id);
int                      get_total_reservations();
long long                get_recorded_revenue();
int                      get_total_checkins();
ReservationLog&          get_reservation_log();   // used by file_manager
