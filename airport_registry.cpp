/*
Zeadonay Wejebu
ETS1609/17
*/
#include "airport_registry.h"

static const std::vector<AirportInfo> AIRPORTS = {
    // Africa
    {"ADD", "Addis Ababa",   "Bole International Airport"},
    {"CAI", "Cairo",         "Cairo International Airport"},
    {"JNB", "Johannesburg",  "OR Tambo International Airport"},
    {"LOS", "Lagos",         "Murtala Muhammed International Airport"},
    // Middle East
    {"DXB", "Dubai",         "Dubai International Airport"},
    {"AUH", "Abu Dhabi",     "Abu Dhabi International Airport"},
    {"DOH", "Doha",          "Hamad International Airport"},
    // Europe
    {"LHR", "London",        "London Heathrow Airport"},
    {"CDG", "Paris",         "Paris Charles de Gaulle Airport"},
    {"FRA", "Frankfurt",     "Frankfurt am Main Airport"},
    {"AMS", "Amsterdam",     "Amsterdam Airport Schiphol"},
    {"FCO", "Rome",          "Leonardo da Vinci-Fiumicino Airport"},
    // North America
    {"JFK", "New York",      "John F. Kennedy International Airport"},
    {"LAX", "Los Angeles",   "Los Angeles International Airport"},
    {"ORD", "Chicago",       "Chicago O'Hare International Airport"},
    {"ATL", "Atlanta",       "Hartsfield-Jackson Atlanta International Airport"},
    {"DFW", "Dallas",        "Dallas/Fort Worth International Airport"},
    // Asia-Pacific
    {"HND", "Tokyo",         "Haneda Airport"},
    {"SIN", "Singapore",     "Singapore Changi Airport"},
    {"SYD", "Sydney",        "Sydney Kingsford Smith Airport"},
};

const std::vector<AirportInfo>& list_all_airports() {
    return AIRPORTS;
}

std::string get_airport_display_string(const AirportInfo& airport) {
    return airport.city_name + " (" + airport.iata_code + ") - " + airport.airport_name;
}
