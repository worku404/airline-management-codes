#include "flight_manager.h"

#include <algorithm>

#include "validator.h"

namespace {
std::vector<Flight> g_flights;

bool is_valid_status(const std::string& status) {
    return status == "On Time" || status == "Delayed" || status == "Boarding" || status == "Cancelled";
}
}

Status add_flight(const Flight& flight) {
    if (flight.flight_id.empty()) {
        return make_failure("FLIGHT_ID_MISSING", "Flight ID is required");
    }
    if (!is_valid_iata(flight.origin_iata) || !is_valid_iata(flight.destination_iata)) {
        return make_failure("IATA_INVALID", "Invalid IATA code in flight definition");
    }
    if (flight.arrival_time <= flight.departure_time) {
        return make_failure("FLIGHT_TIME_INVALID", "Arrival time must be after departure time");
    }
    if (find_flight(flight.flight_id) != nullptr) {
        return make_failure("FLIGHT_DUPLICATE", "Flight ID already exists");
    }
    Flight stored = flight;
    if (stored.status.empty()) {
        stored.status = "On Time";
    }
    g_flights.push_back(stored);
    return make_success();
}

FlightQueryResult search_flights(const SearchCriteria& criteria) {
    if (!is_valid_iata(criteria.origin) || !is_valid_iata(criteria.destination)) {
        return {{}, make_failure("IATA_INVALID", "Invalid origin or destination IATA code")};
    }
    if (criteria.date_window_start > criteria.date_window_end) {
        return {{}, make_failure("DATE_WINDOW_INVALID", "Invalid date window")};
    }

    std::vector<Flight> matches;
    for (const auto& flight : g_flights) {
        if (flight.origin_iata != criteria.origin) {
            continue;
        }
        if (flight.destination_iata != criteria.destination) {
            continue;
        }
        if (flight.departure_time < criteria.date_window_start || flight.departure_time > criteria.date_window_end) {
            continue;
        }
        matches.push_back(flight);
    }

    return {matches, make_success()};
}

const Flight* find_flight(const std::string& flight_id) {
    auto it = std::find_if(g_flights.begin(), g_flights.end(), [&](const Flight& flight) {
        return flight.flight_id == flight_id;
    });
    if (it == g_flights.end()) {
        return nullptr;
    }
    return &(*it);
}

Status set_flight_status(const std::string& flight_id, const std::string& new_status) {
    if (!is_valid_status(new_status)) {
        return make_failure("STATUS_INVALID", "Invalid flight status transition");
    }
    for (auto& flight : g_flights) {
        if (flight.flight_id == flight_id) {
            flight.status = new_status;
            return make_success();
        }
    }
    return make_failure("FLIGHT_NOT_FOUND", "Flight ID not found for status update");
}

const std::vector<Flight>& get_flight_registry() {
    return g_flights;
}

int count_flights_with_status(const std::string& status) {
    return static_cast<int>(std::count_if(g_flights.begin(), g_flights.end(), [&](const Flight& flight) {
        return flight.status == status;
    }));
}