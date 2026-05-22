/*
Zeadonay Wejebu
ETS1609/17
*/

#pragma once

#include <string>
#include <vector>

struct AirportInfo {
    std::string iata_code;      // "JFK", "DXB", etc.
    std::string city_name;      // "New York", "Dubai", etc.
    std::string airport_name;   // Full name for display
};

const std::vector<AirportInfo>& list_all_airports();

std::string get_airport_display_string(const AirportInfo& airport);
