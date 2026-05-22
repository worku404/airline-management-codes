/*
Zeadonay Wejebu
ETS1609/17
*/

#pragma once

#include <string>

bool is_valid_passport(const std::string& passport_number);
bool is_valid_pnr(const std::string& pnr_id);
bool is_valid_seat_number(const std::string& seat_number);
bool is_valid_baggage_count(int baggage_count);

// Parses a non-negative integer from a digit-only string.
// Returns false if the string is empty, contains non-digits, or exceeds 1,000,000.
bool try_parse_int(const std::string& value, int& out);
