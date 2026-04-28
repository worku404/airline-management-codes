#pragma once

#include <string>
#include <vector>

#include "common_types.h"

struct RevenueAuditResult {
    long long computed_total;
    long long recorded_total;
    Status status;
};

struct OperationalReport {
    std::string report;
    Status status;
};

Money calculate_dynamic_price(const Money& base_price,
                              int remaining_seats,
                              int total_capacity,
                              Status& status);
RevenueAuditResult audit_revenue(const std::vector<Money>& booking_totals, long long recorded_total);
OperationalReport generate_operational_report(int total_reservations,
                                              int total_checked_in,
                                              int total_boarded,
                                              int delayed_flights);