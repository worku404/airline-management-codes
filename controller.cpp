/*
Worku Wondoson ETS1459/17
controller.cpp
controller.h       
*/

#include "controller.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "boarding_controller.h"
#include "flight_manager.h"
#include "interactive_search_helper.h"
#include "inventory_service.h"
#include "reservation_engine.h"
#include "revenue_service.h"
#include "validator.h"
#include "report_generator.h"

namespace {
constexpr const char* GREEN = "\033[32m";
constexpr const char* BLUE = "\033[34m";
constexpr const char* RED = "\033[31m";
constexpr const char* RESET = "\033[0m";
constexpr int kSecondsPerHour = 3600;
constexpr int kSecondsPerDay = 24 * kSecondsPerHour;
constexpr int kRecentPnrLimit = 8;
constexpr int kFlightStatusCount = 4;
const char* kFlightStatuses[] = {"On Time", "Delayed", "Boarding", "Cancelled"};

std::vector<Flight> g_last_search_results;
std::vector<std::string> g_recent_pnrs;

void print_divider(char fill = '=') {
    std::cout << std::string(78, fill) << "\n";
}

void print_section_title(const std::string& title) {
    std::cout << "\n";
    print_divider();
    std::cout << title << "\n";
    print_divider();
}

void print_hint(const std::string& text) {
    std::cout << "Hint: " << text << "\n";
}

void print_help() {
    print_section_title("Airline Management CLI");
    std::cout <<GREEN<< "Type a command name to open guided mode, or enter the full command directly.\n\n"<<RESET
              << "Guided actions:\n"
              <<BLUE<< "  search                         Search flights interactively\n"
              << "  book                           Create a booking with prompts\n"
              << "  checkin                        Check in a passenger with prompts\n"
              << "  status                         Update a flight status with prompts\n"
              << "  report                         Show revenue and operations summary\n\n"<<RESET
              << "Direct commands:\n"
              << "  book <flight_id> <class> <first> <last> <passport> [seat]\n"
              << "  checkin <pnr> [baggage_count]\n"
              << "  status <flight_id> <status>\n\n"
              << "System commands:\n"
              << "  :help, help                    Show this help\n"
              << "  :quit, quit, exit              Exit the system\n\n"
              << "Examples:\n"
              << "  search\n"
              << "  book FL-101 Economy John Smith AB123456\n"
              << "  checkin PNR-ABC123 2\n"
              << "  status FL-101 On Time\n";
}

void print_status(const Status& status, const std::string& context) {
    if (status.success) {
        std::cout << "Success: " << context << "\n";
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

std::string join_tokens(const std::vector<std::string>& tokens, size_t start_index) {
    std::ostringstream stream;
    for (size_t i = start_index; i < tokens.size(); ++i) {
        if (i > start_index) {
            stream << ' ';
        }
        stream << tokens[i];
    }
    return stream.str();
}

std::string trim_copy(const std::string& value) {
    const size_t start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const size_t end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

std::string normalize_command(const std::string& command) {
    if (!command.empty() && command[0] == ':') {
        return ":" + normalize_command(command.substr(1));
    }

    std::string normalized;
    normalized.reserve(command.size());
    for (char ch : command) {
        if (ch == '-' || ch == '_') {
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }
    return normalized;
}

std::string normalize_pnr_input(std::string pnr) {
    pnr = to_upper(trim_copy(pnr));
    const std::string prefix = "PNR-";
    if (pnr.rfind(prefix, 0) == 0) {
        return pnr.substr(prefix.size());
    }
    return pnr;
}

std::string canonicalize_status_input(std::string value) {
    value = trim_copy(value);
    std::string normalized;
    normalized.reserve(value.size());
    bool previous_was_space = false;

    for (char ch : value) {
        if (std::isspace(static_cast<unsigned char>(ch)) || ch == '-' || ch == '_') {
            if (!normalized.empty() && !previous_was_space) {
                normalized.push_back(' ');
            }
            previous_was_space = true;
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
        previous_was_space = false;
    }

    if (!normalized.empty() && normalized.back() == ' ') {
        normalized.pop_back();
    }

    if (normalized == "ON TIME") {
        return "On Time";
    }
    if (normalized == "DELAYED") {
        return "Delayed";
    }
    if (normalized == "BOARDING") {
        return "Boarding";
    }
    if (normalized == "CANCELLED") {
        return "Cancelled";
    }
    return "";
}

std::time_t now_plus_days(int days) {
    return std::time(nullptr) + static_cast<std::time_t>(days) * kSecondsPerDay;
}

bool try_parse_int(const std::string& value, int& out) {
    try {
        size_t index = 0;
        const int parsed = std::stoi(value, &index);
        if (index != value.size()) {
            return false;
        }
        out = parsed;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool parse_baggage_count(const std::string& value, int& out) {
    return try_parse_int(value, out);
}

std::string format_money(const Money& amount) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2)
           << static_cast<double>(amount.amount_cents) / 100.0
           << ' ' << amount.currency;
    return stream.str();
}

std::string format_date_time(std::time_t timestamp) {
    const std::tm* time_info = std::localtime(&timestamp);
    char buffer[17] = {};
    if (time_info != nullptr) {
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", time_info);
    }
    return buffer;
}

std::string reservation_status_to_string(ReservationStatus status) {
    switch (status) {
        case ReservationStatus::Reserved:
            return "Reserved";
        case ReservationStatus::CheckedIn:
            return "Checked In";
        case ReservationStatus::Boarded:
            return "Boarded";
        default:
            return "Unknown";
    }
}

void remember_pnr(const std::string& pnr) {
    const auto existing = std::find(g_recent_pnrs.begin(), g_recent_pnrs.end(), pnr);
    if (existing != g_recent_pnrs.end()) {
        g_recent_pnrs.erase(existing);
    }
    g_recent_pnrs.push_back(pnr);
    if (static_cast<int>(g_recent_pnrs.size()) > kRecentPnrLimit) {
        g_recent_pnrs.erase(g_recent_pnrs.begin());
    }
}

std::string prompt_line(const std::string& label) {
    std::cout << label << "\n> ";
    std::string input;
    std::getline(std::cin, input);
    return trim_copy(input);
}

std::string prompt_non_empty_text(const std::string& label) {
    while (true) {
        const std::string input = prompt_line(label);
        if (!input.empty()) {
            return input;
        }
        std::cout << "Please enter a value.\n";
    }
}

void print_flight_option(int index, const Flight& flight) {
    const InventorySnapshot snapshot = get_inventory_snapshot(flight.flight_id);
    std::cout << "  " << index << ". " << flight.flight_id
              << " | " << flight.origin_iata << " -> " << flight.destination_iata
              << " | Depart " << format_date_time(flight.departure_time)
              << " | " << format_money(flight.base_price)
              << " | " << flight.status << "\n"
              << "     Seats: Economy " << snapshot.economy_available
              << ", Business " << snapshot.business_available
              << ", First " << snapshot.first_available << "\n";
}

std::string prompt_flight_id(const std::string& initial_value,
                             bool prefer_last_search_results,
                             bool allow_prompt) {
    const std::string normalized_value = to_upper(trim_copy(initial_value));
    if (!normalized_value.empty() && find_flight(normalized_value) != nullptr) {
        return normalized_value;
    }

    if (!allow_prompt) {
        return "";
    }

    if (!normalized_value.empty()) {
        std::cout << "Flight '" << initial_value << "' was not found. Please choose from the list below.\n";
    }

    const std::vector<Flight>& flights =
        (prefer_last_search_results && !g_last_search_results.empty())
            ? g_last_search_results
            : get_flight_registry();

    if (flights.empty()) {
        return "";
    }

    print_section_title(prefer_last_search_results && !g_last_search_results.empty()
                            ? "Choose a Flight From the Latest Search"
                            : "Available Flights");
    for (size_t i = 0; i < flights.size(); ++i) {
        print_flight_option(static_cast<int>(i + 1), flights[i]);
    }

    while (true) {
        const std::string input = prompt_line("Choose a flight by number or enter a flight ID:");
        int selection = 0;
        if (try_parse_int(input, selection)) {
            if (selection >= 1 && selection <= static_cast<int>(flights.size())) {
                return flights[selection - 1].flight_id;
            }
        } else {
            const std::string flight_id = to_upper(input);
            if (!flight_id.empty() && find_flight(flight_id) != nullptr) {
                return flight_id;
            }
        }
        std::cout << "Please choose a valid flight number or flight ID.\n";
    }
}

bool resolve_seat_class(const std::string& initial_value, bool allow_prompt, SeatClass& out) {
    if (!trim_copy(initial_value).empty()) {
        const Status status = parse_seat_class(initial_value, out);
        if (status.success) {
            return true;
        }
        if (!allow_prompt) {
            return false;
        }
        std::cout << "Seat class '" << initial_value << "' was not recognized.\n";
    }

    if (!allow_prompt) {
        return false;
    }

    while (true) {
        std::cout << "Available cabin classes:\n"
                  << "  1. Economy\n"
                  << "  2. Business\n"
                  << "  3. First\n";
        const std::string input =
            prompt_line("Choose a class by number or name (press Enter for Economy):");
        if (input.empty()) {
            out = SeatClass::Economy;
            return true;
        }

        int selection = 0;
        if (try_parse_int(input, selection)) {
            if (selection == 1) {
                out = SeatClass::Economy;
                return true;
            }
            if (selection == 2) {
                out = SeatClass::Business;
                return true;
            }
            if (selection == 3) {
                out = SeatClass::First;
                return true;
            }
        }

        const Status status = parse_seat_class(input, out);
        if (status.success) {
            return true;
        }
        std::cout << "Please choose Economy, Business, or First.\n";
    }
}

std::string resolve_name_field(const std::string& initial_value,
                               const std::string& label,
                               bool allow_prompt) {
    if (!trim_copy(initial_value).empty()) {
        return trim_copy(initial_value);
    }
    if (!allow_prompt) {
        return "";
    }
    return prompt_non_empty_text(label);
}

std::string resolve_passport(const std::string& initial_value, bool allow_prompt) {
    const std::string normalized_value = to_upper(trim_copy(initial_value));
    if (!normalized_value.empty() && is_valid_passport(normalized_value)) {
        return normalized_value;
    }

    if (!allow_prompt) {
        return "";
    }

    if (!normalized_value.empty()) {
        std::cout << "Passport '" << initial_value
                  << "' is invalid. Use 6 to 9 uppercase letters and digits.\n";
    }

    while (true) {
        const std::string passport =
            to_upper(prompt_line("Passenger passport number (6-9 uppercase letters/digits):"));
        if (is_valid_passport(passport)) {
            return passport;
        }
        std::cout << "Passport format is invalid.\n";
    }
}

std::string resolve_seat_number(const std::string& initial_value, bool allow_prompt) {
    const std::string normalized_value = to_upper(trim_copy(initial_value));
    if (normalized_value.empty()) {
        if (!allow_prompt) {
            return "";
        }
    } else if (is_valid_seat_number(normalized_value)) {
        return normalized_value;
    } else if (!allow_prompt) {
        return "";
    } else {
        std::cout << "Seat '" << initial_value << "' is invalid. Examples: E12 or 12A.\n";
    }

    while (true) {
        const std::string seat =
            to_upper(prompt_line("Preferred seat (optional, press Enter to auto-assign):"));
        if (seat.empty()) {
            return "";
        }
        if (is_valid_seat_number(seat)) {
            return seat;
        }
        std::cout << "Seat format is invalid.\n";
    }
}

std::vector<std::string> recent_reserved_pnrs() {
    std::vector<std::string> available;
    for (auto it = g_recent_pnrs.rbegin(); it != g_recent_pnrs.rend(); ++it) {
        const ReservationRecord* record = find_reservation(*it);
        if (record != nullptr && record->status == ReservationStatus::Reserved) {
            available.push_back(*it);
        }
    }
    return available;
}

std::string prompt_pnr(const std::string& initial_value, bool allow_prompt) {
    const std::string normalized_value = normalize_pnr_input(initial_value);
    if (!normalized_value.empty() && is_valid_pnr(normalized_value)) {
        return normalized_value;
    }

    if (!allow_prompt) {
        return "";
    }

    if (!trim_copy(initial_value).empty()) {
        std::cout << "PNR '" << initial_value
                  << "' is invalid. Enter a 6-character booking code.\n";
    }

    const std::vector<std::string> available = recent_reserved_pnrs();
    if (!available.empty()) {
        print_section_title("Recent Bookings Ready for Check-in");
        for (size_t i = 0; i < available.size(); ++i) {
            const ReservationRecord* record = find_reservation(available[i]);
            if (record == nullptr) {
                continue;
            }
            std::cout << "  " << (i + 1) << ". PNR-" << record->pnr_id
                      << " | " << record->request.passenger.first_name
                      << " " << record->request.passenger.last_name
                      << " | " << record->request.flight_id
                      << " | " << reservation_status_to_string(record->status) << "\n";
        }
    }

    while (true) {
        const std::string input = prompt_line(
            available.empty()
                ? "Enter the booking PNR (example: ABC123 or PNR-ABC123):"
                : "Choose a booking by number or enter a PNR:");
        int selection = 0;
        if (!available.empty() && try_parse_int(input, selection)) {
            if (selection >= 1 && selection <= static_cast<int>(available.size())) {
                return available[selection - 1];
            }
        } else {
            const std::string pnr = normalize_pnr_input(input);
            if (is_valid_pnr(pnr)) {
                return pnr;
            }
        }
        std::cout << "Please enter a valid PNR.\n";
    }
}

bool resolve_baggage_count(const std::string& initial_value, bool allow_prompt, int& baggage_count) {
    if (!trim_copy(initial_value).empty()) {
        if (parse_baggage_count(initial_value, baggage_count) &&
            is_valid_baggage_count(baggage_count)) {
            return true;
        }
        if (!allow_prompt) {
            return false;
        }
        std::cout << "Baggage count must be a whole number between 0 and 5.\n";
    }

    if (!allow_prompt) {
        baggage_count = 0;
        return true;
    }

    while (true) {
        const std::string input = prompt_line("Checked bags (press Enter for 0):");
        if (input.empty()) {
            baggage_count = 0;
            return true;
        }
        if (parse_baggage_count(input, baggage_count) && is_valid_baggage_count(baggage_count)) {
            return true;
        }
        std::cout << "Please enter a whole number between 0 and 5.\n";
    }
}

std::string prompt_flight_status(const std::string& initial_value, bool allow_prompt) {
    const std::string canonical = canonicalize_status_input(initial_value);
    if (!canonical.empty()) {
        return canonical;
    }

    if (!allow_prompt) {
        return "";
    }

    if (!trim_copy(initial_value).empty()) {
        std::cout << "Status '" << initial_value << "' was not recognized.\n";
    }

    while (true) {
        std::cout << "Available flight statuses:\n";
        for (int i = 0; i < kFlightStatusCount; ++i) {
            std::cout << "  " << (i + 1) << ". " << kFlightStatuses[i] << "\n";
        }
        const std::string input = prompt_line("Choose a status by number or name:");
        int selection = 0;
        if (try_parse_int(input, selection)) {
            if (selection >= 1 && selection <= kFlightStatusCount) {
                return kFlightStatuses[selection - 1];
            }
        } else {
            const std::string selected = canonicalize_status_input(input);
            if (!selected.empty()) {
                return selected;
            }
        }
        std::cout << "Please choose a valid flight status.\n";
    }
}

void print_search_results(const std::vector<Flight>& flights) {
    print_section_title("Search Results");
    std::cout << flights.size() << (flights.size() == 1 ? " flight found.\n" : " flights found.\n");
    for (size_t i = 0; i < flights.size(); ++i) {
        print_flight_option(static_cast<int>(i + 1), flights[i]);
    }
    print_hint("Type 'book' to open guided booking for one of these flights.");
}

void print_booking_confirmation(const BookingResult& result) {
    print_section_title("Booking Confirmed");
    const ReservationRecord* record = find_reservation(result.pnr_id);
    if (record != nullptr) {
        std::cout << "PNR:       PNR-" << record->pnr_id << "\n"
                  << "Passenger: " << record->request.passenger.first_name
                  << " " << record->request.passenger.last_name << "\n"
                  << "Flight:    " << record->request.flight_id << "\n"
                  << "Class:     " << seat_class_to_string(record->request.preferred_class) << "\n"
                  << "Seat:      " << record->request.seat_number << "\n";
    } else {
        std::cout << "PNR:       PNR-" << result.pnr_id << "\n";
    }
    std::cout << "Total:     " << format_money(result.total_cost) << "\n";
    print_hint("Type 'checkin' to start guided check-in for this booking.");
}

void print_checkin_confirmation(const CheckInResult& result) {
    print_section_title("Check-in Complete");
    const ReservationRecord* record = find_reservation(result.pass.pnr_id);
    std::cout << "PNR:            PNR-" << result.pass.pnr_id << "\n";
    if (record != nullptr) {
        std::cout << "Passenger:      " << record->request.passenger.first_name
                  << " " << record->request.passenger.last_name << "\n"
                  << "Flight:         " << record->request.flight_id << "\n"
                  << "Seat:           " << record->request.seat_number << "\n";
    }
    std::cout << "Gate:           " << result.pass.gate << "\n"
              << "Boarding group: " << result.pass.boarding_group << "\n"
              << "Checked bags:   " << result.baggage_count << "\n";
    print_hint("Type 'report' to review the updated operations summary.");
}

void print_status_confirmation(const Flight& flight) {
    print_section_title("Flight Status Updated");
    std::cout << "Flight:     " << flight.flight_id << "\n"
              << "Route:      " << flight.origin_iata << " -> " << flight.destination_iata << "\n"
              << "New status: " << flight.status << "\n";
}

void print_report_summary() {
    const RevenueAuditResult audit = audit_revenue(get_booking_totals(), get_recorded_revenue());
    const int reservations = get_total_reservations();
    const int checked_in = get_total_checkins();
    const int boarded = get_total_boarded();
    const int delayed = count_flights_with_status("Delayed");

    print_section_title("Revenue and Operations Report");
    if (audit.status.success) {
        std::cout << "Revenue audit: Passed\n"
                  << "Revenue total: " << format_money({audit.recorded_total, "USD"}) << "\n";
    } else {
        std::cout << "Revenue audit: Failed\n"
                  << "Recorded total: " << format_money({audit.recorded_total, "USD"}) << "\n"
                  << "Computed total: " << format_money({audit.computed_total, "USD"}) << "\n";
        print_status(audit.status, "Revenue audit");
    }

    OperationalReport report = generate_operational_report(reservations, checked_in, boarded, delayed);
    if (report.status.success) {
        std::cout << "\nOperations summary:\n  "
                  << report.report << "\n";
    }
}

void handle_search_command() {
    const SearchCriteria criteria = get_interactive_search();
    const FlightQueryResult result = search_flights(criteria);

    if (!result.status.success) {
        print_status(result.status, "Search");
        return;
    }

    g_last_search_results = result.available_flights;
    if (result.available_flights.empty()) {
        print_section_title("Search Results");
        std::cout << "No flights matched your search.\n";
        print_hint("Try another route or search a wider date range.");
        return;
    }

    print_search_results(result.available_flights);
}

void handle_book_command(const std::vector<std::string>& tokens) {
    const bool allow_prompt = tokens.size() < 6;

    const std::string flight_id = prompt_flight_id(
        tokens.size() >= 2 ? tokens[1] : "",
        true,
        true);
    if (flight_id.empty()) {
        print_status(make_failure("BOOKING_FLIGHT_UNKNOWN", "Flight not found"), "Booking");
        return;
    }

    SeatClass seat_class = SeatClass::Economy;
    if (!resolve_seat_class(tokens.size() >= 3 ? tokens[2] : "", allow_prompt || tokens.size() < 3, seat_class)) {
        print_status(make_failure("SEAT_CLASS_INVALID", "Unknown seat class"), "Booking");
        return;
    }

    const std::string first_name = resolve_name_field(tokens.size() >= 4 ? tokens[3] : "",
                                                      "Passenger first name:",
                                                      allow_prompt || tokens.size() < 4);
    const std::string last_name = resolve_name_field(tokens.size() >= 5 ? tokens[4] : "",
                                                     "Passenger last name:",
                                                     allow_prompt || tokens.size() < 5);
    const std::string passport = resolve_passport(tokens.size() >= 6 ? tokens[5] : "",
                                                  allow_prompt || tokens.size() < 6);

    if (first_name.empty() || last_name.empty() || passport.empty()) {
        print_status(make_failure("BOOKING_PASSENGER_MISSING", "Passenger details are required"), "Booking");
        return;
    }

    std::string seat_number;
    if (tokens.size() >= 7) {
        seat_number = resolve_seat_number(tokens[6], false);
        if (seat_number.empty()) {
            print_status(make_failure("BOOKING_SEAT_INVALID", "Invalid seat number format"), "Booking");
            return;
        }
    } else if (allow_prompt) {
        seat_number = resolve_seat_number("", true);
    }

    const BookingRequest request{
        flight_id,
        {first_name, last_name, passport},
        seat_class,
        seat_number,
    };

    const BookingResult result = create_booking(request);
    if (!result.status.success) {
        print_status(result.status, "Booking");
        return;
    }

    remember_pnr(result.pnr_id);
    print_booking_confirmation(result);
}

void handle_checkin_command(const std::vector<std::string>& tokens) {
    const bool guided_mode = tokens.size() < 2;

    const std::string pnr = prompt_pnr(tokens.size() >= 2 ? tokens[1] : "", guided_mode);
    if (pnr.empty()) {
        print_status(make_failure("CHECKIN_PNR_INVALID", "Invalid PNR format"), "Check-in");
        return;
    }

    int baggage_count = 0;
    if (tokens.size() >= 3) {
        if (!resolve_baggage_count(tokens[2], false, baggage_count)) {
            print_status(make_failure("CHECKIN_BAGGAGE_INVALID", "Invalid baggage count"), "Check-in");
            return;
        }
    } else if (guided_mode) {
        if (!resolve_baggage_count("", true, baggage_count)) {
            print_status(make_failure("CHECKIN_BAGGAGE_INVALID", "Invalid baggage count"), "Check-in");
            return;
        }
    }

    if (tokens.size() < 3 && !guided_mode) {
        baggage_count = 0;
    }

    const CheckInResult result = process_check_in(pnr, baggage_count);
    if (!result.status.success) {
        print_status(result.status, "Check-in");
        return;
    }

    print_checkin_confirmation(result);
}

void handle_status_command(const std::vector<std::string>& tokens) {
    const bool allow_prompt = tokens.size() < 3;

    const std::string flight_id = prompt_flight_id(
        tokens.size() >= 2 ? tokens[1] : "",
        false,
        true);
    if (flight_id.empty()) {
        print_status(make_failure("FLIGHT_NOT_FOUND", "Flight not found"), "Status");
        return;
    }

    const std::string new_status = prompt_flight_status(
        tokens.size() >= 3 ? join_tokens(tokens, 2) : "",
        allow_prompt || tokens.size() < 3);
    if (new_status.empty()) {
        print_status(make_failure("STATUS_INVALID", "Invalid flight status transition"), "Status");
        return;
    }

    const Status operation_status = update_flight_status(flight_id, new_status);
    if (!operation_status.success) {
        print_status(operation_status, "Status");
        return;
    }

    const Flight* flight = find_flight(flight_id);
    if (flight != nullptr) {
        print_status_confirmation(*flight);
    } else {
        print_status(operation_status, "Flight status updated");
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
    while (std::cout << "\nairline> " && std::getline(std::cin, line)) {
        line = trim_copy(line);
        if (line.empty()) {
            continue;
        }

        const std::vector<std::string> tokens = split_tokens(line);
        if (tokens.empty()) {
            continue;
        }

        const std::string command = normalize_command(tokens[0]);

        if (command == ":QUIT" || command == "QUIT" || command == "EXIT") {
            break;
        }
        if (command == ":HELP" || command == "HELP") {
            print_help();
            continue;
        }

        if (command == "SEARCH") {
            handle_search_command();
            continue;
        }

        if (command == "BOOK") {
            handle_book_command(tokens);
            continue;
        }

        if (command == "CHECKIN") {
            handle_checkin_command(tokens);
            continue;
        }

        if (command == "STATUS") {
            handle_status_command(tokens);
            continue;
        }

        if (command == "REPORT") {
            print_report_summary();
            continue;
        }

        std::cout << "Unknown command: " << tokens[0] << "\n";
        print_hint("Try search, book, checkin, status, report, :help, or :quit.");
    }
}
