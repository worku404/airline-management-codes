#include "common_types.h"

#include <algorithm>
#include <cctype>
#include <limits>

Status make_success() {
    return {true, "", ""};
}

Status make_failure(const std::string& code, const std::string& message) {
    Status status{false, code, message};
    if (status.error_code.empty()) {
        status.error_code = "UNKNOWN";
    }
    if (status.message.empty()) {
        status.message = "Unknown error";
    }
    return status;
}

namespace {
bool will_add_overflow(long long lhs, long long rhs) {
    if (rhs > 0 && lhs > std::numeric_limits<long long>::max() - rhs) {
        return true;
    }
    if (rhs < 0 && lhs < std::numeric_limits<long long>::min() - rhs) {
        return true;
    }
    return false;
}

std::string to_upper(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}
}

Status add_money(const Money& lhs, const Money& rhs, Money& out) {
    if (lhs.currency != rhs.currency) {
        return make_failure("MONEY_CURRENCY_MISMATCH", "Currency mismatch in Money addition");
    }
    if (will_add_overflow(lhs.amount_cents, rhs.amount_cents)) {
        return make_failure("MONEY_OVERFLOW", "Money addition overflow");
    }
    out.amount_cents = lhs.amount_cents + rhs.amount_cents;
    out.currency = lhs.currency;
    return make_success();
}

Status subtract_money(const Money& lhs, const Money& rhs, Money& out) {
    if (lhs.currency != rhs.currency) {
        return make_failure("MONEY_CURRENCY_MISMATCH", "Currency mismatch in Money subtraction");
    }
    if (will_add_overflow(lhs.amount_cents, -rhs.amount_cents)) {
        return make_failure("MONEY_OVERFLOW", "Money subtraction overflow");
    }
    out.amount_cents = lhs.amount_cents - rhs.amount_cents;
    out.currency = lhs.currency;
    return make_success();
}

Status parse_seat_class(const std::string& input, SeatClass& out) {
    const std::string value = to_upper(input);
    if (value == "ECONOMY") {
        out = SeatClass::Economy;
        return make_success();
    }
    if (value == "BUSINESS") {
        out = SeatClass::Business;
        return make_success();
    }
    if (value == "FIRST") {
        out = SeatClass::First;
        return make_success();
    }
    return make_failure("SEAT_CLASS_INVALID", "Unknown seat class");
}

std::string seat_class_to_string(SeatClass seat_class) {
    switch (seat_class) {
        case SeatClass::Economy:
            return "Economy";
        case SeatClass::Business:
            return "Business";
        case SeatClass::First:
            return "First";
        default:
            return "Unknown";
    }
}