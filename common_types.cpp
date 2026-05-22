/*
Yared Tsehaye
ETS1488/17
*/
#include "common_types.h"

Status make_success() {
    Status s;
    s.success = true;
    s.message = "";
    return s;
}

Status make_failure(const std::string& message) {
    Status s;
    s.success = false;
    s.message = message;
    return s;
}

Status parse_seat_class(const std::string& input, SeatClass& out) {
    std::string upper = to_upper(input);
    if (upper == "ECONOMY") {
        out = SeatClass::Economy;
        return make_success();
    }
    if (upper == "BUSINESS") {
        out = SeatClass::Business;
        return make_success();
    }
    if (upper == "FIRST") {
        out = SeatClass::First;
        return make_success();
    }
    return make_failure("Unknown seat class. Use: Economy, Business, or First");
}

std::string seat_class_to_string(SeatClass seat_class) {
    if (seat_class == SeatClass::Economy) {
        return "Economy";
    }
    if (seat_class == SeatClass::Business) {
        return "Business";
    }
    if (seat_class == SeatClass::First) {
        return "First";
    }
    return "Unknown";
}
