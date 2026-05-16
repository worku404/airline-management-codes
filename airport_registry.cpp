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
    const AirportInfo AIRPORTS[] = {
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
    
    // Calculate the number of airports in the array
    // sizeof(AIRPORTS) = total size of array
    // sizeof(AIRPORTS[0]) = size of one element
    // Division gives us the count
    constexpr size_t AIRPORT_COUNT = sizeof(AIRPORTS) / sizeof(AIRPORTS[0]);
}



std::vector<AirportInfo> list_all_airports() {
    // Convert the static array to a vector for easier handling
    // This creates a copy of all airports
    std::vector<AirportInfo> result;
    
    // Loop through all airports and add them to the vector
    for (size_t i = 0; i < AIRPORT_COUNT; ++i) {
        result.push_back(AIRPORTS[i]);
    }
    
    return result;
}


const AirportInfo* find_airport_by_iata(const std::string& iata_code) {
    to_upper(iata_code);
    for (size_t i = 0; i < AIRPORT_COUNT; ++i) {
        
        if (AIRPORTS[i].iata_code == iata_code) {
            return &AIRPORTS[i];
        }
    }
    
    // No match found
    return nullptr;
}


std::vector<AirportInfo> find_airports_by_city_name(
    const std::string& city_name) {
    to_upper(city_name);

    std::vector<AirportInfo> results;
    
    for (size_t i = 0; i < AIRPORT_COUNT; ++i) {
        
        if (AIRPORTS[i].city_name == city_name) {
            results.push_back(AIRPORTS[i]);
        }
    }
    
    return results;
}


std::string get_airport_display_string(const AirportInfo& airport) {
    
    std::string result;
    
    result += airport.city_name;           // "New York"
    result += " (";                        // " ("
    result += airport.iata_code;           // "JFK"
    result += ") - ";                      // ") - "
    result += airport.airport_name;        // "John F. Kennedy International..."
    
    return result;
}