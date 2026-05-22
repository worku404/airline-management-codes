/*
Yared Tsehaye
ETS1488/17
*/
#pragma once
#include <string>
#include "common_types.h"

// BoardingPass: issued to a passenger at check-in
struct BoardingPass {
    std::string pnr_id;
    std::string gate;           // e.g. "G1"
    int         boarding_group; // 1=First, 2=Business, 3=Economy
};

// CheckInResult: returned by check_in_passenger
struct CheckInResult {
    BoardingPass pass;
    Status       status;
    int          baggage_count;
};

CheckInResult check_in_passenger(const std::string& pnr_id, int baggage_count);
