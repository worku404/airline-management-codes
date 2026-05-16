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

std::vector<AirportInfo> list_all_airports();

const AirportInfo* find_airport_by_iata(const std::string& iata_code);

// ---------------------------------------------------------------------------
std::vector<AirportInfo> find_airports_by_city_name(const std::string& city_name);


std::string get_airport_display_string(const AirportInfo& airport);
