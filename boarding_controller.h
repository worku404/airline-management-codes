#pragma once

#include <string>

#include "common_types.h"

struct BoardingPass {
    std::string pnr_id;
    std::string gate;
    int boarding_group;
};

struct CheckInResult {
    BoardingPass pass;
    Status status;
    int baggage_count;
};

CheckInResult process_check_in(const std::string& pnr_id, int baggage_count);
Status update_flight_status(const std::string& flight_id, const std::string& new_status);
int get_total_checkins();
int get_total_boarded();