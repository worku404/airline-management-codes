#pragma once

#include <string>

bool is_valid_iata(const std::string& code);
bool is_valid_passport(const std::string& passport_number);
bool is_valid_pnr(const std::string& pnr_id);
bool is_valid_seat_number(const std::string& seat_number);
bool is_valid_baggage_count(int baggage_count);