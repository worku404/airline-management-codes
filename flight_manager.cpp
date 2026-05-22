/*
Yeabsera Mengesha
ETS1495/17
*/

#include "flight_manager.h"

#include <cctype>

namespace {
    std::vector<Flight> g_flights;

    // Internal helper: finds a flight by ID and returns a mutable pointer for modification.
    Flight* find_flight_mutable(const std::string& flight_id) {
        for (auto& flight : g_flights) {
            if (flight.flight_id == flight_id) {
                return &flight;
            }
        }
        return nullptr;
    }
}

// Normalizes a raw status string into one of the four canonical values.
// Handles extra whitespace, dashes, underscores, and mixed case.
// Returns empty string if the input doesn't match any known status.
std::string canonicalize_flight_status(const std::string& input) {
    const size_t first = input.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const size_t last = input.find_last_not_of(" \t\r\n");
    std::string value = input.substr(first, last - first + 1);

    std::string normalized;
    normalized.reserve(value.size());
    bool previous_was_space = false;

    for (char ch : value) {
        if (std::isspace(static_cast<unsigned char>(ch)) || ch == '-' || ch == '_') {
            if (!normalized.empty() && !previous_was_space) {
                normalized.push_back(' ');
            }
            previous_was_space = true;
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
        previous_was_space = false;
    }
    if (!normalized.empty() && normalized.back() == ' ') {
        normalized.pop_back();
    }

    if (normalized == "ON TIME")   return "On Time";
    if (normalized == "DELAYED")   return "Delayed";
    if (normalized == "BOARDING")  return "Boarding";
    if (normalized == "CANCELLED") return "Cancelled";
    return "";
}

Status add_flight(const Flight& flight) {
    if (flight.flight_id.empty()) {
        return make_failure("Flight ID is required");
    }
    if (flight.arrival_time <= flight.departure_time) {
        return make_failure("Arrival time must be after departure time");
    }
    if (find_flight(flight.flight_id) != nullptr) {
        return make_failure("Flight ID already exists");
    }

    Flight flight_to_store = flight;
    if (flight_to_store.status.empty()) {
        flight_to_store.status = "On Time";
    }
    g_flights.push_back(flight_to_store);
    return make_success();
}

std::vector<Flight> search_flights(const SearchCriteria& criteria) {
    if (criteria.date_window_start > criteria.date_window_end) return {};

    std::vector<Flight> matching_flights;
    for (const auto& flight : g_flights) {
        if (flight.origin_iata != criteria.origin) continue;
        if (flight.destination_iata != criteria.destination) continue;
        if (flight.departure_time < criteria.date_window_start) continue;
        if (flight.departure_time > criteria.date_window_end) continue;
        matching_flights.push_back(flight);
    }
    return matching_flights;
}

const Flight* find_flight(const std::string& flight_id) {
    for (const auto& flight : g_flights) {
        if (flight.flight_id == flight_id) {
            return &flight;
        }
    }
    return nullptr;
}

Status set_flight_status(const std::string& flight_id, const std::string& new_status) {
    find_flight_mutable(flight_id)->status = new_status;
    return make_success();
}

const std::vector<Flight>& get_flight_registry() {
    return g_flights;
}

int count_flights_with_status(const std::string& status) {
    int count = 0;
    for (const auto& flight : g_flights) {
        if (flight.status == status) {
            count++;
        }
    }
    return count;
}
