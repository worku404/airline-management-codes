/*
Yared Tsehaye
ETS1488/17
*/
#pragma once
#include <string>
#include <cctype>

// Status: returned by functions to report success or failure without exceptions
struct Status {
    bool        success;
    std::string message;
};

Status      make_success();
Status      make_failure(const std::string& message);

// Money: stores prices in integer cents to avoid floating-point errors
struct Money {
    long long   amount_cents;   // e.g. 45000 means $450.00
    std::string currency;       // e.g. "USD"
};

// SeatClass: the three cabin classes available on a flight
enum class SeatClass {
    Economy,
    Business,
    First
};

Status      parse_seat_class(const std::string& input, SeatClass& out);
std::string seat_class_to_string(SeatClass seat_class);

// Converts every character in the string to uppercase
inline std::string to_upper(std::string value) {
    for (int i = 0; i < (int)value.size(); i++) {
        value[i] = (char)toupper((unsigned char)value[i]);
    }
    return value;
}
