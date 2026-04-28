#pragma once

#include <string>

struct Status {
    bool success;
    std::string error_code;
    std::string message;
};

Status make_success();
Status make_failure(const std::string& code, const std::string& message);

struct Money {
    long long amount_cents;
    std::string currency;
};

enum class SeatClass {
    Economy,
    Business,
    First
};

Status add_money(const Money& lhs, const Money& rhs, Money& out);
Status subtract_money(const Money& lhs, const Money& rhs, Money& out);

Status parse_seat_class(const std::string& input, SeatClass& out);
std::string seat_class_to_string(SeatClass seat_class);