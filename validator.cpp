/*
Zeadonay Wejebu
ETS1609/17
*/
#include "validator.h"
#include <cctype>

static bool is_upper_alnum(char ch) {
    return (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9');
}

static bool is_letter_then_digits(const std::string& s) {
    if (s.empty()) {
        return false;
    }
    if (!isalpha((unsigned char)s[0])) {
        return false;
    }
    for (int i = 1; i < (int)s.size(); i++) {
        if (!isdigit((unsigned char)s[i])) {
            return false;
        }
    }
    return true;
}

static bool is_digits_then_letter(const std::string& s) {
    if (s.empty()) {
        return false;
    }
    for (int i = 0; i < (int)s.size() - 1; i++) {
        if (!isdigit((unsigned char)s[i])) {
            return false;
        }
    }
    if (!isalpha((unsigned char)s[s.size() - 1])) {
        return false;
    }
    return true;
}

bool is_valid_passport(const std::string& passport_number) {
    int len = (int)passport_number.size();
    if (len < 6 || len > 9) {
        return false;
    }
    for (int i = 0; i < len; i++) {
        if (!is_upper_alnum(passport_number[i])) {
            return false;
        }
    }
    return true;
}

bool is_valid_pnr(const std::string& pnr_id) {
    if ((int)pnr_id.size() != 6) {
        return false;
    }
    for (int i = 0; i < 6; i++) {
        if (!is_upper_alnum(pnr_id[i])) {
            return false;
        }
    }
    return true;
}

bool is_valid_seat_number(const std::string& seat_number) {
    int len = (int)seat_number.size();
    if (len < 2 || len > 5) {
        return false;
    }
    return is_letter_then_digits(seat_number) || is_digits_then_letter(seat_number);
}

bool is_valid_baggage_count(int baggage_count) {
    return baggage_count >= 0 && baggage_count <= 5;
}

bool try_parse_int(const std::string& value, int& out) {
    if (value.empty()) {
        return false;
    }
    int val = 0;
    for (int i = 0; i < (int)value.size(); i++) {
        if (!isdigit((unsigned char)value[i])) {
            return false;
        }
        val = val * 10 + (value[i] - '0');
        if (val > 1000000) {
            return false;
        }
    }
    out = val;
    return true;
}
