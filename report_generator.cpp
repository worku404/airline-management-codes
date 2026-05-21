/*
Samuel Firegedil
ETS1292/17
*/

#include "report_generator.h"
#include <sstream>

// Generates an operational report summary showing reservations, check-ins, boardings, and delays.
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