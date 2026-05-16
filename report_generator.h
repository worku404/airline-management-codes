#pragma once

#include <string>
#include "common_types.h"

struct OperationalReport {
    std::string report;
    Status status;
};

OperationalReport generate_operational_report(int total_reservations,
                                              int total_checked_in,
                                              int total_boarded,
                                              int delayed_flights);