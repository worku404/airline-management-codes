/*
Zeadonay Wejebu
ETS1609/17
*/

#include "validator.h"

#include <cctype>

namespace {
    // Helper: Checks if character is digit OR uppercase letter.
    bool is_upper_alnum(char ch) {
        return std::isdigit(static_cast<unsigned char>(ch)) ||
            (std::isalpha(static_cast<unsigned char>(ch)) && 
                std::isupper(static_cast<unsigned char>(ch)));
    }

    // Helper: Checks if the seat number is a single letter followed entirely by digits (e.g., "A12").
    bool is_letter_followed_by_digits(const std::string& seat_number) {
        if (seat_number.empty() || !std::isalpha(static_cast<unsigned char>(seat_number.front()))) {
            return false;
        }
        for (size_t i = 1; i < seat_number.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(seat_number[i]))) {
                return false;
            }
        }
        return true;
    }

    // Helper: Checks if the seat number starts with digits followed by a single letter (e.g., "12A").
    bool is_digits_followed_by_letter(const std::string& seat_number) {
        if (seat_number.empty() || !std::isalpha(static_cast<unsigned char>(seat_number.back()))) {
            return false;
        }
        for (size_t i = 0; i + 1 < seat_number.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(seat_number[i]))) {
                return false;
            }
        }
        return true;
    }
}

// Function: is_valid_passport
// Purpose: Validates that a passport number consists of 6 to 9 alphanumeric characters.
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

// Function: is_valid_pnr
// Purpose: Validates that a PNR consists of exactly 6 uppercase alphanumeric characters.
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

// Function: is_valid_seat_number
// Purpose: Validates that a seat number format is either Letter+Digits or Digits+Letter.
bool is_valid_seat_number(const std::string& seat_number) {
     // RULE 1: Length must be 2-5 characters
    if (seat_number.size() < 2 || seat_number.size() > 5) {
        return false;
    }

    // RULE 2: Must be EITHER "Letter+Digits" OR "Digits+Letter"
    return is_letter_followed_by_digits(seat_number) || is_digits_followed_by_letter(seat_number);
}

// Function: is_valid_baggage_count
// Purpose: Checks if passenger baggage count is within valid range (0-5 bags).
bool is_valid_baggage_count(int baggage_count) {
    return baggage_count >= 0 && baggage_count <= 5;
}

// Function: try_parse_int
// Purpose: Parses a non-negative integer from a digit-only string without exceptions.
bool try_parse_int(const std::string& value, int& out) {
    if (value.empty()) {
        return false;
    }
    for (char ch : value) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    long long val = 0;
    for (char ch : value) {
        val = val * 10 + (ch - '0');
        if (val > 1000000) {
            return false;
        }
    }
    out = static_cast<int>(val);
    return true;
}