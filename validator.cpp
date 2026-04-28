#include "validator.h"

#include <cctype>

namespace {
bool is_upper_alnum(char ch) {
    return std::isdigit(static_cast<unsigned char>(ch)) ||
           (std::isalpha(static_cast<unsigned char>(ch)) && std::isupper(static_cast<unsigned char>(ch)));
}
}

bool is_valid_iata(const std::string& code) {
    if (code.size() != 3) {
        return false;
    }
    for (char ch : code) {
        if (!std::isupper(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

bool is_valid_passport(const std::string& passport_number) {
    if (passport_number.size() < 6 || passport_number.size() > 9) {
        return false;
    }
    for (char ch : passport_number) {
        if (!is_upper_alnum(ch)) {
            return false;
        }
    }
    return true;
}

bool is_valid_pnr(const std::string& pnr_id) {
    if (pnr_id.size() != 6) {
        return false;
    }
    for (char ch : pnr_id) {
        if (!is_upper_alnum(ch)) {
            return false;
        }
    }
    return true;
}

bool is_valid_seat_number(const std::string& seat_number) {
    if (seat_number.size() < 2 || seat_number.size() > 5) {
        return false;
    }
    auto is_letter_followed_by_digits = [&]() {
        if (!std::isalpha(static_cast<unsigned char>(seat_number.front()))) {
            return false;
        }
        for (size_t i = 1; i < seat_number.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(seat_number[i]))) {
                return false;
            }
        }
        return true;
    };
    auto is_digits_followed_by_letter = [&]() {
        if (!std::isalpha(static_cast<unsigned char>(seat_number.back()))) {
            return false;
        }
        for (size_t i = 0; i + 1 < seat_number.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(seat_number[i]))) {
                return false;
            }
        }
        return true;
    };
    return is_letter_followed_by_digits() || is_digits_followed_by_letter();
}

bool is_valid_baggage_count(int baggage_count) {
    return baggage_count >= 0 && baggage_count <= 5;
}