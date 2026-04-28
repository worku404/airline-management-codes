#include "controller.h"

#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "boarding_controller.h"
#include "flight_manager.h"
#include "inventory_service.h"
#include "reservation_engine.h"
#include "revenue_service.h"
#include "security_utils.h"
#include "validator.h"

namespace {
constexpr int kSecondsPerHour = 3600;
constexpr int kSecondsPerDay = 24 * kSecondsPerHour;

void print_help() {
    std::cout << "Commands:\n"
              << "  :help                          Show this help\n"
              << "  :quit                          Exit the system\n"
              << "  search <origin> <dest>         Search flights\n"
              << "  book <flight_id> <class> [first last passport seat]\n"
              << "  checkin <pnr> [baggage_count]  Check in by PNR\n"
              << "  status <flight_id> <status>    Update flight status\n"
              << "  report                         Show revenue/ops report\n";
}

void print_status(const Status& status, const std::string& context) {
    if (status.success) {
        std::cout << "Result: " << context << "\n";
        return;
    }
    std::cout << "Error: " << context << ": " << status.message;
    if (!status.error_code.empty()) {
        std::cout << " [" << status.error_code << "]";
    }
    std::cout << "\n";
}

std::vector<std::string> split_tokens(const std::string& line) {
    std::istringstream stream(line);
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::time_t now_plus_days(int days) {
    return std::time(nullptr) + static_cast<std::time_t>(days) * kSecondsPerDay;
}

bool parse_baggage_count(const std::string& value, int& out) {
    try {
        size_t index = 0;
        int parsed = std::stoi(value, &index);
        if (index != value.size()) {
            return false;
        }
        out = parsed;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}
}

void initialize_system() {
    add_flight({"FL-101", "ADD", "DXB", now_plus_days(1), now_plus_days(1) + 3 * kSecondsPerHour, {45000, "USD"}, "On Time"});
    add_flight({"FL-102", "ADD", "LHR", now_plus_days(2), now_plus_days(2) + 8 * kSecondsPerHour, {75000, "USD"}, "On Time"});
    add_flight({"FL-201", "DXB", "ADD", now_plus_days(3), now_plus_days(3) + 3 * kSecondsPerHour, {47000, "USD"}, "On Time"});

    initialize_inventory(get_flight_registry(), 120, 24, 12);
}

void run_repl() {
    print_help();
    std::string line;
    while (std::cout << "> " && std::getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }
        if (line == ":quit") {
            break;
        }
        if (line == ":help") {
            print_help();
            continue;
        }

        const auto tokens = split_tokens(line);
        if (tokens.empty()) {
            continue;
        }

        if (tokens[0] == "search") {
            if (tokens.size() < 3) {
                print_status(make_failure("SEARCH_ARGS", "Usage: search <origin> <dest>"), "Search");
                continue;
            }
            SearchCriteria criteria{tokens[1], tokens[2], std::time(nullptr) - kSecondsPerHour, now_plus_days(30)};
            FlightQueryResult result = search_flights(criteria);
            if (!result.status.success) {
                print_status(result.status, "Search");
                continue;
            }
            if (result.available_flights.empty()) {
                std::cout << "Result: No flights found\n";
                continue;
            }
            std::cout << "Result: " << result.available_flights.size() << " flights\n";
            for (const auto& flight : result.available_flights) {
                std::cout << "  " << flight.flight_id << " " << flight.origin_iata << "->" << flight.destination_iata
                          << " price " << flight.base_price.amount_cents / 100.0
                          << " " << flight.base_price.currency << " status " << flight.status << "\n";
            }
            continue;
        }

        if (tokens[0] == "book") {
            if (tokens.size() < 3) {
                print_status(make_failure("BOOK_ARGS", "Usage: book <flight_id> <class>"), "Booking");
                continue;
            }
            SeatClass seat_class;
            Status seat_status = parse_seat_class(tokens[2], seat_class);
            if (!seat_status.success) {
                print_status(seat_status, "Booking");
                continue;
            }

            Passenger passenger{"Guest", "Passenger", "P000000"};
            std::string seat_number;
            if (tokens.size() >= 6) {
                passenger.first_name = tokens[3];
                passenger.last_name = tokens[4];
                passenger.passport_number = tokens[5];
            }
            if (tokens.size() >= 7) {
                seat_number = tokens[6];
            }

            BookingRequest request{tokens[1], passenger, seat_class, seat_number};
            BookingResult result = create_booking(request);
            if (!result.status.success) {
                print_status(result.status, "Booking");
                continue;
            }
            std::cout << "Result: Booking Confirmed: PNR-" << result.pnr_id
                      << " Total " << result.total_cost.amount_cents / 100.0
                      << " " << result.total_cost.currency << "\n";
            continue;
        }

        if (tokens[0] == "checkin") {
            if (tokens.size() < 2) {
                print_status(make_failure("CHECKIN_ARGS", "Usage: checkin <pnr>"), "Check-in");
                continue;
            }
            int baggage = 0;
            if (tokens.size() >= 3) {
                if (!parse_baggage_count(tokens[2], baggage)) {
                    print_status(make_failure("CHECKIN_BAGGAGE_FORMAT", "Baggage count must be an integer"), "Check-in");
                    continue;
                }
            }
            CheckInResult result = process_check_in(tokens[1], baggage);
            if (!result.status.success) {
                print_status(result.status, "Check-in");
                continue;
            }
            std::cout << "Result: Boarding Pass " << result.pass.pnr_id
                      << " Gate " << result.pass.gate
                      << " Group " << result.pass.boarding_group
                      << " Bags " << result.baggage_count << "\n";
            continue;
        }

        if (tokens[0] == "status") {
            if (tokens.size() < 3) {
                print_status(make_failure("STATUS_ARGS", "Usage: status <flight_id> <status>"), "Status");
                continue;
            }
            Status operation_status = update_flight_status(tokens[1], tokens[2]);
            print_status(operation_status, "Status");
            continue;
        }

        if (tokens[0] == "report") {
            RevenueAuditResult audit = audit_revenue(get_booking_totals(), get_recorded_revenue());
            if (!audit.status.success) {
                print_status(audit.status, "Revenue Audit");
            } else {
                std::cout << "Result: Revenue audited. Total " << audit.recorded_total / 100.0 << "\n";
            }
            OperationalReport report = generate_operational_report(get_total_reservations(),
                                                                   get_total_checkins(),
                                                                   get_total_boarded(),
                                                                   count_flights_with_status("Delayed"));
            if (report.status.success) {
                std::cout << "Result: " << report.report << "\n";
            }
            continue;
        }

        print_status(make_failure("CMD_UNKNOWN", "Unknown command"), "Command");
    }
}