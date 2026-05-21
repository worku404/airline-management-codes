/*
Zeadonay Wejebu
ETS1609/17
*/


#include "airport_registry.h"
#include "common_types.h"

#include <algorithm>

// ============================================================================
// AIRPORT DATABASE: Hardcoded airport data for Phase 1
// ============================================================================
// 
// This is a static (global) array of all available airports in the system.
// Each airport has:
// - IATA code: 3-letter code used in flight search
// - City name: User-friendly city name for display/search
// - Airport name: Full official airport name
//
// Current airports include major hubs in Africa, Middle East, Europe, and US
// 
// FUTURE: This could be replaced with a database query when we add persistence
//
namespace {
    // Static array of all airports
    // Using a namespace-scoped array ensures it's only initialized once
    const std::vector<AirportInfo> AIRPORTS = {
        // ====================================================================
        // AFRICA
        // ====================================================================
        
        {"ADD", "Addis Ababa", "Addis Ababa Bole International Airport"},
        {"CAI", "Cairo", "Cairo International Airport"},
        {"JNB", "Johannesburg", "OR Tambo International Airport"},
        {"LOS", "Lagos", "Murtala Muhammed International Airport"},
        
        // ====================================================================
        // MIDDLE EAST
        // ====================================================================
        
        {"DXB", "Dubai", "Dubai International Airport"},
        {"AUH", "Abu Dhabi", "Abu Dhabi International Airport"},
        {"DXB", "Dubai", "Dubai International Airport"},  // Note: Some cities have multiple
        {"DOH", "Doha", "Hamad International Airport"},
        
        // ====================================================================
        // EUROPE
        // ====================================================================
        
        {"LHR", "London", "London Heathrow Airport"},
        {"LGW", "London", "London Gatwick Airport"},
        {"LCY", "London", "London City Airport"},
        {"CDG", "Paris", "Paris Charles de Gaulle Airport"},
        {"ORY", "Paris", "Paris Orly Airport"},
        {"FRA", "Frankfurt", "Frankfurt am Main Airport"},
        {"DUS", "Düsseldorf", "Düsseldorf International Airport"},
        {"AMS", "Amsterdam", "Amsterdam Airport Schiphol"},
        {"BCN", "Barcelona", "Barcelona-El Prat Airport"},
        {"MAD", "Madrid", "Adolfo Suárez Madrid-Barajas Airport"},
        {"MXP", "Milan", "Milan Malpensa Airport"},
        {"FCO", "Rome", "Leonardo da Vinci-Fiumicino Airport"},
        
        // ====================================================================
        // NORTH AMERICA
        // ====================================================================
        
        {"JFK", "New York", "John F. Kennedy International Airport"},
        {"LGA", "New York", "LaGuardia Airport"},
        {"EWR", "New York", "Newark Liberty International Airport"},
        {"LAX", "Los Angeles", "Los Angeles International Airport"},
        {"SFO", "San Francisco", "San Francisco International Airport"},
        {"ORD", "Chicago", "Chicago O'Hare International Airport"},
        {"MDW", "Chicago", "Chicago Midway International Airport"},
        {"ATL", "Atlanta", "Hartsfield-Jackson Atlanta International Airport"},
        {"DEN", "Denver", "Denver International Airport"},
        {"DFW", "Dallas", "Dallas/Fort Worth International Airport"},
        {"MIA", "Miami", "Miami International Airport"},
        {"BOS", "Boston", "Boston Logan International Airport"},
        {"SEA", "Seattle", "Seattle-Tacoma International Airport"},
        
        // ====================================================================
        // ASIA-PACIFIC
        // ====================================================================
        
        {"HND", "Tokyo", "Haneda Airport"},
        {"NRT", "Tokyo", "Narita International Airport"},
        {"ICN", "Seoul", "Incheon International Airport"},
        {"HKG", "Hong Kong", "Hong Kong International Airport"},
        {"SIN", "Singapore", "Singapore Changi Airport"},
        {"BKK", "Bangkok", "Suvarnabhumi Airport"},
        {"KUL", "Kuala Lumpur", "Kuala Lumpur International Airport"},
        {"CGK", "Jakarta", "Soekarno-Hatta International Airport"},
        {"SYD", "Sydney", "Sydney Kingsford Smith Airport"},
        {"MEL", "Melbourne", "Melbourne International Airport"},
        
        // ====================================================================
        // SOUTH AMERICA
        // ====================================================================
        
        {"GIG", "Rio de Janeiro", "Rio de Janeiro International Airport"},
        {"GRU", "São Paulo", "São Paulo International Airport"},
    };
    // No longer using fixed AIRPORT_COUNT since AIRPORTS is a vector.
}



// Function: list_all_airports
// Purpose: Returns a list of all registered airports in the system.
std::vector<AirportInfo> list_all_airports() {
    
    return AIRPORTS;
}

// Function: find_airport_by_iata
// Purpose: Finds a registered airport using its 3-letter IATA code.
const AirportInfo* find_airport_by_iata(const std::string& iata_code) {
    to_upper(iata_code);
    for (size_t i = 0; i < AIRPORTS.size(); ++i) {
        
        if (AIRPORTS[i].iata_code == iata_code) {
            return &AIRPORTS[i];
        }
    }
    
    // No match found
    return nullptr;
}

// Function: find_airports_by_city_name
// Purpose: Finds all registered airports located in a specific city.
std::vector<AirportInfo> find_airports_by_city_name(
    const std::string& city_name) {
    to_upper(city_name);

    std::vector<AirportInfo> results;
    
    for (size_t i = 0; i < AIRPORTS.size(); ++i) {
        
        if (AIRPORTS[i].city_name == city_name) {
            results.push_back(AIRPORTS[i]);
        }
    }
    
    return results;
}

// Function: get_airport_display_string
// Purpose: Simplifies airport details into a single readable string format.
std::string get_airport_display_string(const AirportInfo& airport) {
    return airport.city_name + " (" + airport.iata_code + ") - " + airport.airport_name;
}