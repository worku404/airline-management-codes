/*
Zeadonay Wejebu
ETS1609/17
*/

#include "validator.h"

#include <cctype>

namespace {
    bool is_upper_alnum(char ch) {
        // Check if character is digit OR uppercase letter
        return std::isdigit(static_cast<unsigned char>(ch)) ||
            (std::isalpha(static_cast<unsigned char>(ch)) && 
                std::isupper(static_cast<unsigned char>(ch)));
    }
}


bool is_valid_passport(const std::string& passport_number) {
     // RULE 1: Length must be between 6-9 characters
    if (passport_number.size() < 6 || passport_number.size() > 9) {
        return false;
    }
    // RULE 2: Every character must be UPPERCASE letter or digit
    for (char ch : passport_number) {
        if (!is_upper_alnum(ch)) {
            return false;
        }
    }
    return true;
}

//Validate PNR (Passenger Name Record) - the booking reference code
bool is_valid_pnr(const std::string& pnr_id) {
    // RULE 1: Must be EXACTLY 6 characters
    if (pnr_id.size() != 6) {
        return false;
    }
     // RULE 2: Each character must be uppercase letter OR digit
    for (char ch : pnr_id) {
        if (!is_upper_alnum(ch)) {
            return false;
        }
    }
    return true;
}

bool is_valid_seat_number(const std::string& seat_number) {
     // RULE 1: Length must be 2-5 characters
    if (seat_number.size() < 2 || seat_number.size() > 5) {
        return false;
    }

    // RULE 2: Must be EITHER "Letter+Digits" OR "Digits+Letter"
    // Check format 1: Letter followed by digits (e.g., "A1", "B123")
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