/*
Yeabsera Mengesha
ETS1495/17
*/
#pragma once
#include <ctime>
#include <string>
#include <vector>
#include "common_types.h"

// Flight: one scheduled commercial flight.
// Seat counts are stored directly here — no separate inventory module.
struct Flight {
    std::string flight_id;
    std::string origin_iata;
    std::string destination_iata;
    std::time_t departure_time;
    std::time_t arrival_time;
    Money       base_price;
    std::string status;
    int economy_seats;     // remaining economy seats available
    int business_seats;    // remaining business seats available
    int first_seats;       // remaining first class seats available
    int economy_capacity;  // total economy seats (set once at creation)
    int business_capacity; // total business seats
    int first_capacity;    // total first class seats
};

// SearchCriteria: the filter parameters used when searching for flights
struct SearchCriteria {
    std::string origin;
    std::string destination;
    std::time_t date_window_start;
    std::time_t date_window_end;
};

Status                     add_flight(const Flight& flight);
std::vector<Flight>        search_flights(const SearchCriteria& criteria);
Flight*                    find_flight(const std::string& flight_id);
Status                     set_flight_status(const std::string& flight_id, const std::string& new_status);
Status                     decrement_seat(const std::string& flight_id, SeatClass seat_class);
const std::vector<Flight>& get_flight_registry();
int                        count_flights_with_status(const std::string& status);
