
/*
Yeabsera Mengesha
ETS1495/17
*/

#include "flight_manager.h"

#include <cctype>

// ============================================================================
// PRIVATE DATA & HELPERS (Global Flight Registry)
// ============================================================================

namespace {
    // Global storage for all flights in the system
    std::vector<Flight> g_flights;

std::string canonicalize_status(std::string status) {
    std::string normalized;
    normalized.reserve(status.size());
    bool previous_was_space = false;

    for (char ch : status) {
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

    if (normalized == "ON TIME") {
        return "On Time";
    }
    if (normalized == "DELAYED") {
        return "Delayed";
    }
    if (normalized == "BOARDING") {
        return "Boarding";
    }
    if (normalized == "CANCELLED") {
        return "Cancelled";
    }
    return "";
}

bool is_valid_status(const std::string& status) {
    return !canonicalize_status(status).empty();
}
}

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

// ---------------------------------------------------------------------------
// Function: add_flight
// Purpose: Add a new flight to the registry with validation
// 
// What happens:
// 1. Check if flight has an ID (required)
// 2. Validate origin and destination are real airport codes
// 3. Verify arrival time is after departure time
// 4. Ensure flight ID doesn't already exist (no duplicates)
// 5. Store flight in registry with default status if none provided
//
// Returns: Status object indicating success or failure
// ---------------------------------------------------------------------------
Status add_flight(const Flight& flight) {
    
    // STEP 1: Validate flight ID exists
    if (flight.flight_id.empty()) {
        return make_failure(
            "FLIGHT_ID_MISSING",
            "Flight ID is required"
        );
    }

    // // STEP 2: Validate IATA codes (airport codes like JFK, LAX)
    // bool origin_valid = flight.origin_iata;
    // bool destination_valid = flight.destination_iata;
    
    // if (!origin_valid || !destination_valid) {
    //     return make_failure(
    //         "IATA_INVALID",
    //         "Invalid IATA code in flight definition"
    //     );
    // }

    // STEP 3: Validate flight times (arrival must be after departure)
    if (flight.arrival_time <= flight.departure_time) {
        return make_failure(
            "FLIGHT_TIME_INVALID",
            "Arrival time must be after departure time"
        );
    }

    // STEP 4: Check for duplicate flight IDs
    const Flight* existing_flight = find_flight(flight.flight_id);
    if (existing_flight != nullptr) {
        return make_failure(
            "FLIGHT_DUPLICATE",
            "Flight ID already exists"
        );
    }

    // STEP 5: Create a copy of the flight to store
    Flight flight_to_store = flight;
    
    // If no status was provided, set default status
    if (flight_to_store.status.empty()) {
        flight_to_store.status = "On Time";
    }

    // Add the flight to our registry
    g_flights.push_back(flight_to_store);
    
    // Return success
    return make_success();
}


// ---------------------------------------------------------------------------
// Function: search_flights
// Purpose: Find all flights matching search criteria
//
// What happens:
// 1. Validate origin and destination IATA codes
// 2. Validate date window (start date before end date)
// 3. Loop through all flights and collect matches
// 4. Return matching flights with a success status
//
// Returns: FlightQueryResult containing matches and status
// ---------------------------------------------------------------------------
FlightQueryResult search_flights(const SearchCriteria& criteria) {
    
    // // STEP 1: Validate IATA codes
    // bool origin_valid = is_valid_iata(criteria.origin);
    // bool destination_valid = is_valid_iata(criteria.destination);
    
    // if (!origin_valid || !destination_valid) {
    //     // Return empty results with failure status
    //     std::vector<Flight> empty_results;
    //     Status error = make_failure(
    //         "IATA_INVALID",
    //         "Invalid origin or destination IATA code"
    //     );
    //     return {empty_results, error};
    // }

    // STEP 2: Validate date window
    if (criteria.date_window_start > criteria.date_window_end) {
        std::vector<Flight> empty_results;
        Status error = make_failure(
            "DATE_WINDOW_INVALID",
            "Invalid date window"
        );
        return {empty_results, error};
    }

    // STEP 3: Search through all flights
    std::vector<Flight> matching_flights;
    
    for (const auto& flight : g_flights) {
        
        // Check if origin matches
        if (flight.origin_iata != criteria.origin) {
            continue;  // Skip this flight, try next one
        }
        
        // Check if destination matches
        if (flight.destination_iata != criteria.destination) {
            continue;  // Skip this flight, try next one
        }
        
        // Check if departure time is within the date window
        bool departs_after_start = flight.departure_time >= criteria.date_window_start;
        bool departs_before_end = flight.departure_time <= criteria.date_window_end;
        
        if (!departs_after_start || !departs_before_end) {
            continue;  // Skip this flight, try next one
        }
        
        // All criteria matched! Add this flight to results
        matching_flights.push_back(flight);
    }

    // STEP 4: Return results with success status
    return {matching_flights, make_success()};
}


// ---------------------------------------------------------------------------
// Function: find_flight
// Purpose: Look up a single flight by its flight ID
//
// Returns: Pointer to the Flight if found, nullptr if not found
// ---------------------------------------------------------------------------
const Flight* find_flight(const std::string& flight_id) {
    
    // Loop through all flights looking for matching ID
    for (const auto& flight : g_flights) {
        if (flight.flight_id == flight_id) {
            return &flight;
        }
    }
    
    // Not found
    return nullptr;
}

// VERSION 2: Writable (for modifications like status updates)
// ============================================================================
Flight* find_flight_mutable(const std::string& flight_id) {
    
    for (auto& flight : g_flights) {  // NOT const
        if (flight.flight_id == flight_id) {
            return &flight;  // Returns non-const pointer
        }
    }
    
    return nullptr;
}

// ---------------------------------------------------------------------------
// Function: set_flight_status
// Purpose: Update the status of a flight (On Time, Delayed, Cancelled, etc.)
//
// What happens:
// 1. Validate the new status is one of the allowed values
// 2. Find the flight by ID
// 3. Update its status
// 4. Return success or appropriate error
//
// Returns: Status indicating success or failure
// ---------------------------------------------------------------------------

Status set_flight_status(const std::string& flight_id, 
                         const std::string& new_status) {
    const std::string canonical_status = canonicalize_status(new_status);
    
    // STEP 1: Validate new status is allowed
    if (canonical_status.empty()) {
        return make_failure(
            "STATUS_INVALID",
            "Invalid flight status transition"
        );
    }

    // STEP 2: Find the flight using mutable version
    Flight* flight_ptr = find_flight_mutable(flight_id);
    
    // STEP 3: Check if flight was found
    if (flight_ptr == nullptr) {
        return make_failure(
            "FLIGHT_NOT_FOUND",
            "Flight ID not found for status update"
        );
    }

    // STEP 4: Now we CAN modify it because it's non-const
    flight_ptr->status = canonical_status;
    
    return make_success();
}


// ---------------------------------------------------------------------------
// Function: get_flight_registry
// Purpose: Get read-only access to all flights (for other modules)
//
// Returns: Const reference to the entire flight registry
// ---------------------------------------------------------------------------
const std::vector<Flight>& get_flight_registry() {
    return g_flights;
}


// ---------------------------------------------------------------------------
// Function: count_flights_with_status
// Purpose: Count how many flights have a specific status
//
// Example: count_flights_with_status("Delayed") returns number of delayed flights
//
// Returns: Integer count of flights with matching status
// ---------------------------------------------------------------------------
int count_flights_with_status(const std::string& status) {
    
    int count = 0;
    
    for (const auto& flight : g_flights) {
        if (flight.status == status) {
            count++;
        }
    }
    
    return count;
}
