/*
Yeabsera Mengesha
ETS1495/17
*/
#include "flight_manager.h"

static std::vector<Flight> g_flights;

Status add_flight(const Flight& flight) {
    if (flight.flight_id.empty()) {
        return make_failure("Flight ID cannot be empty");
    }
    if (flight.arrival_time <= flight.departure_time) {
        return make_failure("Arrival time must be after departure time");
    }
    if (find_flight(flight.flight_id) != nullptr) {
        return make_failure("A flight with this ID already exists");
    }
    Flight f = flight;
    if (f.status.empty()) {
        f.status = "On Time";
    }
    f.economy_capacity  = f.economy_seats;
    f.business_capacity = f.business_seats;
    f.first_capacity    = f.first_seats;
    g_flights.push_back(f);
    return make_success();
}

std::vector<Flight> search_flights(const SearchCriteria& criteria) {
    std::vector<Flight> results;
    if (criteria.date_window_start > criteria.date_window_end) {
        return results;
    }
    for (int i = 0; i < (int)g_flights.size(); i++) {
        const Flight& f = g_flights[i];
        if (!criteria.origin.empty() && f.origin_iata != criteria.origin) {
            continue;
        }
        if (!criteria.destination.empty() && f.destination_iata != criteria.destination) {
            continue;
        }
        if (f.departure_time < criteria.date_window_start ||
            f.departure_time > criteria.date_window_end) {
            continue;
        }
        results.push_back(f);
    }
    return results;
}

Flight* find_flight(const std::string& flight_id) {
    for (int i = 0; i < (int)g_flights.size(); i++) {
        if (g_flights[i].flight_id == flight_id) {
            return &g_flights[i];
        }
    }
    return nullptr;
}

Status set_flight_status(const std::string& flight_id, const std::string& new_status) {
    Flight* f = find_flight(flight_id);
    if (f == nullptr) {
        return make_failure("Flight not found");
    }
    f->status = new_status;
    return make_success();
}

Status decrement_seat(const std::string& flight_id, SeatClass seat_class) {
    Flight* f = find_flight(flight_id);
    if (f == nullptr) {
        return make_failure("Flight not found");
    }
    if (seat_class == SeatClass::Economy) {
        if (f->economy_seats <= 0) {
            return make_failure("No economy seats available");
        }
        f->economy_seats--;
    } else if (seat_class == SeatClass::Business) {
        if (f->business_seats <= 0) {
            return make_failure("No business seats available");
        }
        f->business_seats--;
    } else {
        if (f->first_seats <= 0) {
            return make_failure("No first class seats available");
        }
        f->first_seats--;
    }
    return make_success();
}

const std::vector<Flight>& get_flight_registry() {
    return g_flights;
}

int count_flights_with_status(const std::string& status) {
    int count = 0;
    for (int i = 0; i < (int)g_flights.size(); i++) {
        if (g_flights[i].status == status) {
            count++;
        }
    }
    return count;
}
