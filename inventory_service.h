/*
Yonas Dereje
ETS1558/17
*/
#pragma once

#include <string>

#include "common_types.h"
#include "flight_manager.h"

struct InventoryUpdate {
    std::string flight_id;
    int economy_delta;
    int business_delta;
    int first_delta;
};

struct InventorySnapshot {
    int economy_available;
    int business_available;
    int first_available;
    int economy_capacity;
    int business_capacity;
    int first_capacity;
};

Status initialize_inventory(const std::vector<Flight>& flights,
                            int economy_capacity,
                            int business_capacity,
                            int first_capacity);

Status check_availability(const std::string& flight_id, SeatClass seat_class);
Status update_inventory(const InventoryUpdate& update);
Status reserve_seat(const std::string& flight_id, const std::string& seat_number);
bool is_seat_taken(const std::string& flight_id, const std::string& seat_number);
InventorySnapshot get_inventory_snapshot(const std::string& flight_id);
