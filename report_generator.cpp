#include "report_generator.h"
#include <sstream>

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