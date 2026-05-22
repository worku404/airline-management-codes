/*
Yeabsera Mengesha
ETS1495/17
*/

#pragma once

#include <ctime>
#include <string>
#include <vector>

#include "common_types.h"

struct Flight {
    std::string flight_id;
    std::string origin_iata;
    std::string destination_iata;
    std::time_t departure_time;
    std::time_t arrival_time;
    Money base_price;
    std::string status;
};

struct SearchCriteria {
    std::string origin;
    std::string destination;
    std::time_t date_window_start;
    std::time_t date_window_end;
};

// Normalizes a raw status input string into one of the four canonical values:
// "On Time", "Delayed", "Boarding", "Cancelled". Returns empty string if unrecognized.
std::string canonicalize_flight_status(const std::string& input);

Status add_flight(const Flight& flight);
std::vector<Flight> search_flights(const SearchCriteria& criteria);
const Flight* find_flight(const std::string& flight_id);
Status set_flight_status(const std::string& flight_id, const std::string& new_status);
const std::vector<Flight>& get_flight_registry();
int count_flights_with_status(const std::string& status);
