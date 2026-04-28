#include "revenue_service.h"

#include <limits>
#include <sstream>

namespace {
constexpr double kHighDemandAvailabilityThreshold = 0.10;
constexpr double kModerateAvailabilityThreshold = 0.25;

bool will_mul_overflow(long long value, long long multiplier) {
    if (multiplier == 0) {
        return false;
    }
    if (value > 0 && multiplier > 0) {
        return value > std::numeric_limits<long long>::max() / multiplier;
    }
    if (value < 0 && multiplier < 0) {
        return value < std::numeric_limits<long long>::max() / multiplier;
    }
    if (value > 0 && multiplier < 0) {
        return multiplier < std::numeric_limits<long long>::min() / value;
    }
    if (value < 0 && multiplier > 0) {
        return value < std::numeric_limits<long long>::min() / multiplier;
    }
    return false;
}
}

Money calculate_dynamic_price(const Money& base_price,
                              int remaining_seats,
                              int total_capacity,
                              Status& status) {
    if (total_capacity <= 0) {
        status = make_failure("PRICE_CAPACITY_INVALID", "Total capacity must be positive");
        return base_price;
    }
    if (remaining_seats <= 0) {
        status = make_failure("PRICE_NO_SEATS", "No remaining seats for pricing");
        return base_price;
    }

    long long numerator = 1;
    long long denominator = 1;
    const double ratio = static_cast<double>(remaining_seats) / static_cast<double>(total_capacity);
    if (ratio <= kHighDemandAvailabilityThreshold) {
        numerator = 3;
        denominator = 2;
    } else if (ratio <= kModerateAvailabilityThreshold) {
        numerator = 6;
        denominator = 5;
    }

    if (will_mul_overflow(base_price.amount_cents, numerator)) {
        status = make_failure("PRICE_OVERFLOW", "Dynamic price overflow");
        return base_price;
    }

    Money priced = base_price;
    priced.amount_cents = (base_price.amount_cents * numerator) / denominator;
    status = make_success();
    return priced;
}

RevenueAuditResult audit_revenue(const std::vector<Money>& booking_totals, long long recorded_total) {
    long long computed = 0;
    for (const auto& amount : booking_totals) {
        if (amount.currency != "USD") {
            return {computed, recorded_total, make_failure("AUDIT_CURRENCY", "Unexpected currency in audit")};
        }
        if (amount.amount_cents > 0 && computed > std::numeric_limits<long long>::max() - amount.amount_cents) {
            return {computed, recorded_total, make_failure("AUDIT_OVERFLOW", "Audit total overflow")};
        }
        computed += amount.amount_cents;
    }
    if (computed != recorded_total) {
        return {computed, recorded_total, make_failure("AUDIT_MISMATCH", "Revenue totals do not match")};
    }
    return {computed, recorded_total, make_success()};
}

OperationalReport generate_operational_report(int total_reservations,
                                              int total_checked_in,
                                              int total_boarded,
                                              int delayed_flights) {
    std::ostringstream report;
    report << "Reservations: " << total_reservations
           << ", Checked-in: " << total_checked_in
           << ", Boarded: " << total_boarded
           << ", Delayed flights: " << delayed_flights;
    return {report.str(), make_success()};
}