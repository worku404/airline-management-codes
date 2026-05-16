#include "inventory_service.h"

#include <unordered_map>

#include "validator.h"

namespace {
    struct InventoryState {
        int economy_capacity;
        int business_capacity;
        int first_capacity;
        int economy_available;
        int business_available;
        int first_available;
        SeatMap seat_map;
    };

    std::unordered_map<std::string, InventoryState> g_inventory;

    int* select_available(InventoryState& state, SeatClass seat_class) {
        switch (seat_class) {
            case SeatClass::Economy:
                return &state.economy_available;    // Pointer to available count
            case SeatClass::Business:
                return &state.business_available;
            case SeatClass::First:
                return &state.first_available;
            default:
                return nullptr;  // Invalid class
    }
}

}

Status initialize_inventory(const std::vector<Flight>& flights,
                           int economy_capacity,
                           int business_capacity,
                           int first_capacity) {
    
    // VALIDATION: Capacities must be positive
    if (economy_capacity <= 0 || business_capacity <= 0 || first_capacity <= 0) {
        return make_failure("INV_CAPACITY_INVALID", 
                          "Inventory capacity must be positive");
    }
    
    // Clear any old data
    g_inventory.clear();
    
    // For each flight, create inventory entry
    for (const auto& flight : flights) {
        InventoryState state{
            economy_capacity,      // Total capacity
            business_capacity,
            first_capacity,
            economy_capacity,      // Initially all available
            business_capacity,
            first_capacity,
            SeatMap{flight.flight_id, {}}  // Empty seat map
        };
        
        g_inventory.emplace(flight.flight_id, state);
    }
    
    return make_success();
}
Status check_availability(const std::string& flight_id, SeatClass seat_class) {
    
    // STEP 1: Find flight in inventory
    auto it = g_inventory.find(flight_id);
    if (it == g_inventory.end()) {
        return make_failure("INV_FLIGHT_UNKNOWN", 
                          "Inventory not initialized for flight");
    }
    
    // STEP 2: Get available seats for this class
    int* available = select_available(it->second, seat_class);
    if (available == nullptr) {
        return make_failure("INV_CLASS_INVALID", "Invalid seat class");
    }
    
    // STEP 3: Check if seats available
    if (*available <= 0) {
        return make_failure("INV_FULL", "Seat class capacity exceeded");
    }
    
    return make_success();  // Seats available!
}


Status update_inventory(const InventoryUpdate& update) {
    
    // Find flight
    auto it = g_inventory.find(update.flight_id);
    if (it == g_inventory.end()) {
        return make_failure("INV_FLIGHT_UNKNOWN", 
                          "Inventory not initialized for flight");
    }
    
    InventoryState& state = it->second;

    // Lambda to validate a single class change
    auto check_delta = [&](int available, int capacity, int delta, 
                          const std::string& code) -> Status {
        const int next = available + delta;
        
        // Can't go negative
        if (next < 0) {
            return make_failure(code, "Inventory cannot drop below zero");
        }
        
        // Can't exceed capacity
        if (next > capacity) {
            return make_failure(code, "Inventory cannot exceed capacity");
        }
        
        return make_success();
    };

    // Validate all three classes BEFORE applying any changes
    Status status = check_delta(state.economy_available, 
                               state.economy_capacity, 
                               update.economy_delta, 
                               "INV_ECONOMY_LIMIT");
    if (!status.success) return status;
    
    status = check_delta(state.business_available, 
                        state.business_capacity, 
                        update.business_delta, 
                        "INV_BUSINESS_LIMIT");
    if (!status.success) return status;
    
    status = check_delta(state.first_available, 
                        state.first_capacity, 
                        update.first_delta, 
                        "INV_FIRST_LIMIT");
    if (!status.success) return status;

    // All checks passed - apply changes atomically
    state.economy_available += update.economy_delta;
    state.business_available += update.business_delta;
    state.first_available += update.first_delta;
    
    return make_success();
}

Status reserve_seat(const std::string& flight_id, const std::string& seat_number) {
    
    // STEP 1: Validate seat format (using validator from Member 5)
    if (!is_valid_seat_number(seat_number)) {
        return make_failure("SEAT_INVALID", "Seat number format is invalid");
    }
    
    // STEP 2: Find flight
    auto it = g_inventory.find(flight_id);
    if (it == g_inventory.end()) {
        return make_failure("INV_FLIGHT_UNKNOWN", 
                          "Inventory not initialized for flight");
    }
    
    // STEP 3: Get seat map
    auto& seat_map = it->second.seat_map.occupied_seats;
    
    // STEP 4: Check if seat already taken
    auto existing = seat_map.find(seat_number);
    if (existing != seat_map.end() && existing->second) {
        return make_failure("SEAT_OCCUPIED", 
                          "Requested seat is already occupied");
    }
    
    // STEP 5: Reserve the seat
    seat_map[seat_number] = true;
    return make_success();
}
bool is_seat_taken(const std::string& flight_id, const std::string& seat_number) {
    
    // Try to find flight
    auto it = g_inventory.find(flight_id);
    if (it == g_inventory.end()) {
        return false;  // Flight not found, assume empty
    }
    
    // Try to find seat
    auto seat_it = it->second.seat_map.occupied_seats.find(seat_number);
    
    // Return true only if seat exists AND is marked true
    return seat_it != it->second.seat_map.occupied_seats.end() 
           && seat_it->second;
}


InventorySnapshot get_inventory_snapshot(const std::string& flight_id) {
    
    auto it = g_inventory.find(flight_id);
    
    // If flight not found, return all zeros
    if (it == g_inventory.end()) {
        return {0, 0, 0, 0, 0, 0};
    }
    
    const InventoryState& state = it->second;
    
    // Return snapshot of current state
    return {
        state.economy_available,
        state.business_available,
        state.first_available,
        state.economy_capacity,
        state.business_capacity,
        state.first_capacity
    };
}