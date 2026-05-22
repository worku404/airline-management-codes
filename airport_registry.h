/*
Zeadonay Wejebu
ETS1609/17
*/
#pragma once
#include <string>
#include <vector>

// AirportInfo: holds the data for one airport in the registry
struct AirportInfo {
    std::string iata_code;    // 3-letter identifier: "ADD", "DXB", "LHR"
    std::string city_name;    // human-readable city: "Addis Ababa"
    std::string airport_name; // full official name
};

const std::vector<AirportInfo>& list_all_airports();
std::string get_airport_display_string(const AirportInfo& airport);
