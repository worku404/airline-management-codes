/*
Yonas Dereje
ETS1558/17
*/

#include "inventory_service.h"

#include <unordered_map>
#include <unordered_set>
#include <string>



namespace {
    struct InventoryState {
        int economy_capacity;
        int business_capacity;
        int first_capacity;
        int economy_available;
        int business_available;
        int first_available;
        std::unordered_set<std::string> occupied_seats;
    };

    std::unordered_map<std::string, InventoryState> g_inventory;

    // Returns the available seat count for the given class. Returns -1 for an invalid class.
    int get_available(const InventoryState& state, SeatClass seat_class) {
        switch (seat_class) {
            case SeatClass::Economy:  return state.economy_available;
            case SeatClass::Business: return state.business_available;
            case SeatClass::First:    return state.first_available;
            default:                  return -1;
        }
    }

    // Checks that applying delta to available stays within [0, capacity].
    Status check_delta(int available, int capacity, int delta) {
        const int next = available + delta;
        if (next < 0)        return make_failure("Inventory cannot drop below zero");
        if (next > capacity) return make_failure("Inventory cannot exceed capacity");
        return make_success();
    }
}

Status initialize_inventory(const std::vector<Flight>& flights,
                            int economy_capacity,
                            int business_capacity,
                            int first_capacity) {
    if (economy_capacity <= 0 || business_capacity <= 0 || first_capacity <= 0) {
        return make_failure("Inventory capacity must be positive");
    }
    g_inventory.clear();
    for (const auto& flight : flights) {
        g_inventory.emplace(flight.flight_id, InventoryState{
            economy_capacity, business_capacity, first_capacity,
            economy_capacity, business_capacity, first_capacity,
            {}
        });
    }
    return make_success();
}

Status check_availability(const std::string& flight_id, SeatClass seat_class) {
    auto it = g_inventory.find(flight_id);
    if (it == g_inventory.end()) {
        return make_failure("Inventory not initialized for flight");
    }
    const int available = get_available(it->second, seat_class);
    if (available < 0)  return make_failure("Invalid seat class");
    if (available <= 0) return make_failure("Seat class capacity exceeded");
    return make_success();
}

Status update_inventory(const InventoryUpdate& update) {
    auto it = g_inventory.find(update.flight_id);
    if (it == g_inventory.end()) {
        return make_failure("Inventory not initialized for flight");
    }
    InventoryState& state = it->second;

    Status status = check_delta(state.economy_available,  state.economy_capacity,  update.economy_delta);
    if (!status.success) return status;
    status = check_delta(state.business_available, state.business_capacity, update.business_delta);
    if (!status.success) return status;
    status = check_delta(state.first_available,    state.first_capacity,    update.first_delta);
    if (!status.success) return status;

    state.economy_available  += update.economy_delta;
    state.business_available += update.business_delta;
    state.first_available    += update.first_delta;
    return make_success();
}

Status reserve_seat(const std::string& flight_id, const std::string& seat_number) {
    auto it = g_inventory.find(flight_id);
    if (it == g_inventory.end()) {
        return make_failure("Inventory not initialized for flight");
    }
    auto& occupied = it->second.occupied_seats;
    if (occupied.count(seat_number) > 0) {
        return make_failure("Requested seat is already occupied");
    }
    occupied.insert(seat_number);
    return make_success();
}

bool is_seat_taken(const std::string& flight_id, const std::string& seat_number) {
    auto it = g_inventory.find(flight_id);
    if (it == g_inventory.end()) return false;
    return it->second.occupied_seats.count(seat_number) > 0;
}

InventorySnapshot get_inventory_snapshot(const std::string& flight_id) {
    auto it = g_inventory.find(flight_id);
    if (it == g_inventory.end()) return {0, 0, 0, 0, 0, 0};
    const InventoryState& state = it->second;
    return {
        state.economy_available,  state.business_available,  state.first_available,
        state.economy_capacity,   state.business_capacity,   state.first_capacity
    };
}
