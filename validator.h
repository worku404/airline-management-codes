/*
Zeadonay Wejebu
ETS1609/17
*/
#pragma once
#include <string>

bool is_valid_passport(const std::string& passport_number);  // 6-9 uppercase alphanumeric
bool is_valid_pnr(const std::string& pnr_id);                // exactly 6 uppercase alphanumeric
bool is_valid_seat_number(const std::string& seat_number);   // 2-5 chars: letter+digits or digits+letter
bool is_valid_baggage_count(int baggage_count);               // 0 to 5 inclusive
bool try_parse_int(const std::string& value, int& out);       // digit-only string, max 1,000,000
