/*
Yared Tsehaye
ETS1488/17
*/
#include "revenue_service.h"

Money calculate_dynamic_price(const Money& base_price,
                              int remaining_seats,
                              int total_capacity,
                              Status& status) {
    if (total_capacity <= 0) {
        status = make_failure("Total capacity must be positive");
        return base_price;
    }
    if (remaining_seats <= 0) {
        status = make_failure("No seats remaining for pricing");
        return base_price;
    }
    if (base_price.amount_cents > 1000000000LL) {
        status = make_failure("Base price too large");
        return base_price;
    }

    double ratio = (double)remaining_seats / (double)total_capacity;

    int numerator   = 1;
    int denominator = 1;
    if (ratio <= 0.10) {
        numerator   = 3;
        denominator = 2;
    } else if (ratio <= 0.25) {
        numerator   = 6;
        denominator = 5;
    }

    Money result = base_price;
    result.amount_cents = (base_price.amount_cents * numerator) / denominator;

    status = make_success();
    return result;
}
