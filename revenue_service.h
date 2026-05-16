//Yared Tsehaye
/*ETS1488/17*/



#pragma once

#include <string>
#include <vector>

#include "common_types.h"

struct RevenueAuditResult {
    long long computed_total;
    long long recorded_total;
    Status status;
};

Money calculate_dynamic_price(const Money& base_price,
                              int remaining_seats,
                              int total_capacity,
                              Status& status);
RevenueAuditResult audit_revenue(const std::vector<Money>& booking_totals, long long recorded_total);