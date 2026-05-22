/*
Worku Wondoson ETS1459/17
*/

#include "controller.h"

#include <array>
#include <ctime>
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
#include "validator.h"

namespace {

constexpr int kSecondsPerHour  = 3600;
constexpr int kSecondsPerDay   = 24 * kSecondsPerHour;
constexpr int kSessionPnrLimit = 8;
constexpr std::array<const char*, 4> kFlightStatuses = {"On Time", "Delayed", "Boarding", "Cancelled"};

std::vector<Flight>      g_search_results;
std::vector<std::string> g_session_pnrs;

// ---------------------------------------------------------------------------
// Display helpers
// ---------------------------------------------------------------------------

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
    std::cout << "  1. Search flights\n"
              << "  2. Book a flight\n"
              << "  3. Check in a passenger\n"
              << "  4. Update a flight status\n"
              << "  5. Revenue and operations summary\n"
              << "  6. Show this menu\n"
              << "  7. Exit\n";
}

// Prints a success or error message with context label.
void print_status(const Status& status, const std::string& context) {
    if (status.success) {
        std::cout << "Success: " << context << "\n";
        return;
    }
    std::cout << "Error: " << context << ": " << status.message << "\n";
}

// Formats a Money value as a decimal string (e.g. "450.00 USD").
std::string format_money(const Money& amount) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2)
           << static_cast<double>(amount.amount_cents) / 100.0
           << ' ' << amount.currency;
    return stream.str();
}

// Formats a timestamp as "YYYY-MM-DD HH:MM".
std::string format_date_time(std::time_t timestamp) {
    const std::tm* time_info = std::localtime(&timestamp);
    char buffer[17] = {};
    if (time_info != nullptr) {
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", time_info);
    }
    return buffer;
}

// ---------------------------------------------------------------------------
// Input helpers
// ---------------------------------------------------------------------------

// Trims leading and trailing whitespace.
std::string trim_copy(const std::string& value) {
    const size_t start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    const size_t end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

// Strips the optional "PNR-" prefix and uppercases the input.
std::string normalize_pnr_input(std::string pnr) {
    pnr = to_upper(trim_copy(pnr));
    const std::string prefix = "PNR-";
    if (pnr.rfind(prefix, 0) == 0) return pnr.substr(prefix.size());
    return pnr;
}

// Prints the label, then reads and trims one line of user input.
std::string prompt_line(const std::string& label) {
    std::cout << label << "\n> ";
    std::string input;
    std::getline(std::cin, input);
    return trim_copy(input);
}

// Loops until the user enters a non-empty string.
std::string prompt_non_empty_text(const std::string& label) {
    while (true) {
        const std::string input = prompt_line(label);
        if (!input.empty()) return input;
        std::cout << "Please enter a value.\n";
    }
}

// ---------------------------------------------------------------------------
// Session PNR list
// ---------------------------------------------------------------------------

// Adds a PNR to the session list, keeping only the most recent kSessionPnrLimit entries.
void remember_pnr(const std::string& pnr) {
    g_session_pnrs.push_back(pnr);
    if (static_cast<int>(g_session_pnrs.size()) > kSessionPnrLimit) {
        g_session_pnrs.erase(g_session_pnrs.begin());
    }
}

// Returns session PNRs whose reservations are still in Reserved status.
std::vector<std::string> recent_reserved_pnrs() {
    std::vector<std::string> available;
    for (auto it = g_session_pnrs.rbegin(); it != g_session_pnrs.rend(); ++it) {
        const ReservationRecord* record = find_reservation(*it);
        if (record != nullptr && record->status == ReservationStatus::Reserved) {
            available.push_back(*it);
        }
    }
    return available;
}

// ---------------------------------------------------------------------------
// Flight display
// ---------------------------------------------------------------------------

// Prints one numbered flight row with its current seat availability.
void print_flight_option(int index, const Flight& flight) {
    const InventorySnapshot snap = get_inventory_snapshot(flight.flight_id);
    std::cout << "  " << index << ". " << flight.flight_id
              << " | " << flight.origin_iata << " -> " << flight.destination_iata
              << " | Depart " << format_date_time(flight.departure_time)
              << " | " << format_money(flight.base_price)
              << " | " << flight.status << "\n"
              << "     Seats: Economy " << snap.economy_available
              << ", Business " << snap.business_available
              << ", First "    << snap.first_available << "\n";
}

// ---------------------------------------------------------------------------
// Interactive prompts — each loops until the user gives valid input
// ---------------------------------------------------------------------------

// Shows last search results (or all flights if no search done yet).
// Returns the flight ID the user selects, or "" if no flights exist.
std::string prompt_flight_from_search() {
    const std::vector<Flight>& flights = !g_search_results.empty()
        ? g_search_results : get_flight_registry();
    if (flights.empty()) return "";

    print_section_title(!g_search_results.empty()
        ? "Choose a Flight From the Latest Search" : "Available Flights");
    for (size_t i = 0; i < flights.size(); ++i) {
        print_flight_option(static_cast<int>(i + 1), flights[i]);
    }
    while (true) {
        const std::string input = prompt_line("Choose a flight by number:");
        int selection = 0;
        if (try_parse_int(input, selection) &&
            selection >= 1 && selection <= static_cast<int>(flights.size())) {
            return flights[selection - 1].flight_id;
        }
        std::cout << "Please choose a valid flight number.\n";
    }
}

// Shows all registered flights.
// Returns the flight ID the user selects, or "" if no flights exist.
std::string prompt_flight_from_registry() {
    const std::vector<Flight>& flights = get_flight_registry();
    if (flights.empty()) return "";

    print_section_title("Available Flights");
    for (size_t i = 0; i < flights.size(); ++i) {
        print_flight_option(static_cast<int>(i + 1), flights[i]);
    }
    while (true) {
        const std::string input = prompt_line("Choose a flight by number:");
        int selection = 0;
        if (try_parse_int(input, selection) &&
            selection >= 1 && selection <= static_cast<int>(flights.size())) {
            return flights[selection - 1].flight_id;
        }
        std::cout << "Please choose a valid flight number.\n";
    }
}

// Returns the SeatClass the user chooses (defaults to Economy on empty input).
SeatClass prompt_seat_class() {
    while (true) {
        std::cout << "Available cabin classes:\n"
                  << "  1. Economy\n"
                  << "  2. Business\n"
                  << "  3. First\n";
        const std::string input = prompt_line("Choose a class by number or name (Enter = Economy):");
        if (input.empty()) return SeatClass::Economy;

        int selection = 0;
        if (try_parse_int(input, selection)) {
            if (selection == 1) return SeatClass::Economy;
            if (selection == 2) return SeatClass::Business;
            if (selection == 3) return SeatClass::First;
        }
        SeatClass out;
        if (parse_seat_class(input, out).success) return out;
        std::cout << "Please choose Economy, Business, or First.\n";
    }
}

// Returns a validated passport number (6-9 uppercase alphanumeric characters).
std::string prompt_passport() {
    while (true) {
        const std::string passport = to_upper(prompt_line("Passenger passport (6-9 uppercase letters/digits):"));
        if (is_valid_passport(passport)) return passport;
        std::cout << "Passport format is invalid.\n";
    }
}

// Returns a valid seat number (Letter+Digits or Digits+Letter), or "" for auto-assign.
std::string prompt_seat_number() {
    while (true) {
        const std::string seat = to_upper(prompt_line("Preferred seat (e.g. A1, 12E — Enter to auto-assign):"));
        if (seat.empty()) return "";
        if (is_valid_seat_number(seat)) return seat;
        std::cout << "Invalid format. Use letter + digits (A1) or digits + letter (12A).\n";
    }
}

// Shows recent Reserved bookings for quick selection, or falls back to manual PNR entry.
std::string prompt_pnr() {
    const std::vector<std::string> available = recent_reserved_pnrs();
    if (!available.empty()) {
        print_section_title("Recent Bookings Ready for Check-in");
        for (size_t i = 0; i < available.size(); ++i) {
            const ReservationRecord* record = find_reservation(available[i]);
            if (record == nullptr) continue;
            std::cout << "  " << (i + 1) << ". PNR-" << record->pnr_id
                      << " | " << record->request.passenger.first_name
                      << " "   << record->request.passenger.last_name
                      << " | " << record->request.flight_id
                      << " | Reserved\n";
        }
    }

    while (true) {
        if (available.empty()) {
            const std::string pnr = normalize_pnr_input(prompt_line("Enter PNR (e.g. ABC123 or PNR-ABC123):"));
            if (is_valid_pnr(pnr)) return pnr;
            std::cout << "Please enter a valid 6-character PNR.\n";
        } else {
            const std::string input = prompt_line("Choose a booking by number:");
            int selection = 0;
            if (try_parse_int(input, selection) &&
                selection >= 1 && selection <= static_cast<int>(available.size())) {
                return available[selection - 1];
            }
            std::cout << "Please choose a valid booking number.\n";
        }
    }
}

// Returns a baggage count between 0 and 5 (defaults to 0 on empty input).
int prompt_baggage_count() {
    while (true) {
        const std::string input = prompt_line("Checked bags (Enter = 0):");
        if (input.empty()) return 0;
        int count = 0;
        if (try_parse_int(input, count) && is_valid_baggage_count(count)) return count;
        std::cout << "Please enter a whole number between 0 and 5.\n";
    }
}

// Returns one of the four canonical flight status strings.
std::string prompt_flight_status() {
    while (true) {
        std::cout << "Available flight statuses:\n";
        for (size_t i = 0; i < kFlightStatuses.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << kFlightStatuses[i] << "\n";
        }
        const std::string input = prompt_line("Choose a status by number or name:");
        int selection = 0;
        if (try_parse_int(input, selection) &&
            selection >= 1 && selection <= static_cast<int>(kFlightStatuses.size())) {
            return kFlightStatuses[selection - 1];
        }
        const std::string selected = canonicalize_flight_status(input);
        if (!selected.empty()) return selected;
        std::cout << "Please choose a valid flight status.\n";
    }
}

// ---------------------------------------------------------------------------
// Confirmation printers
// ---------------------------------------------------------------------------

void print_search_results(const std::vector<Flight>& flights) {
    print_section_title("Search Results");
    std::cout << flights.size() << (flights.size() == 1 ? " flight found.\n" : " flights found.\n");
    for (size_t i = 0; i < flights.size(); ++i) {
        print_flight_option(static_cast<int>(i + 1), flights[i]);
    }
    print_hint("Select option 2 to book one of these flights.");
}

void print_booking_confirmation(const BookingResult& result) {
    print_section_title("Booking Confirmed");
    const ReservationRecord* record = find_reservation(result.pnr_id);
    if (record != nullptr) {
        std::cout << "PNR:       PNR-" << record->pnr_id << "\n"
                  << "Passenger: " << record->request.passenger.first_name
                  << " "           << record->request.passenger.last_name << "\n"
                  << "Flight:    " << record->request.flight_id << "\n"
                  << "Class:     " << seat_class_to_string(record->request.preferred_class) << "\n"
                  << "Seat:      " << record->request.seat_number << "\n";
    }
    std::cout << "Total:     " << format_money(result.total_cost) << "\n";
    print_hint("Select option 3 to check in for this booking.");
}

void print_checkin_confirmation(const CheckInResult& result) {
    print_section_title("Check-in Complete");
    const ReservationRecord* record = find_reservation(result.pass.pnr_id);
    std::cout << "PNR:            PNR-" << result.pass.pnr_id << "\n";
    if (record != nullptr) {
        std::cout << "Passenger:      " << record->request.passenger.first_name
                  << " "               << record->request.passenger.last_name << "\n"
                  << "Flight:         " << record->request.flight_id << "\n"
                  << "Seat:           " << record->request.seat_number << "\n";
    }
    std::cout << "Gate:           " << result.pass.gate << "\n"
              << "Boarding group: " << result.pass.boarding_group << "\n"
              << "Checked bags:   " << result.baggage_count << "\n";
    print_hint("Select option 5 to review the operations summary.");
}

void print_status_confirmation(const Flight& flight) {
    print_section_title("Flight Status Updated");
    std::cout << "Flight:     " << flight.flight_id << "\n"
              << "Route:      " << flight.origin_iata << " -> " << flight.destination_iata << "\n"
              << "New status: " << flight.status << "\n";
}

// ---------------------------------------------------------------------------
// Command handlers
// ---------------------------------------------------------------------------

void handle_search_command() {
    const SearchCriteria      criteria = get_interactive_search();
    const std::vector<Flight> results  = search_flights(criteria);

    g_search_results = results;
    if (results.empty()) {
        print_section_title("Search Results");
        std::cout << "No flights matched your search.\n";
        print_hint("Try another route or a wider date range.");
        return;
    }
    print_search_results(results);
}

void handle_book_command() {
    const std::string flight_id = prompt_flight_from_search();
    if (flight_id.empty()) {
        print_status(make_failure("No flights available"), "Booking");
        return;
    }

    const SeatClass   seat_class  = prompt_seat_class();
    const std::string first_name  = prompt_non_empty_text("Passenger first name:");
    const std::string last_name   = prompt_non_empty_text("Passenger last name:");
    const std::string passport    = prompt_passport();
    const std::string seat_number = prompt_seat_number();

    const BookingResult result = book_flight({flight_id, {first_name, last_name, passport}, seat_class, seat_number});
    if (!result.status.success) {
        print_status(result.status, "Booking");
        return;
    }
    remember_pnr(result.pnr_id);
    print_booking_confirmation(result);
}

void handle_checkin_command() {
    const std::string pnr     = prompt_pnr();
    const int         baggage = prompt_baggage_count();

    const CheckInResult result = check_in_passenger(pnr, baggage);
    if (!result.status.success) {
        print_status(result.status, "Check-in");
        return;
    }
    print_checkin_confirmation(result);
}

void handle_status_command() {
    const std::string flight_id = prompt_flight_from_registry();
    if (flight_id.empty()) {
        print_status(make_failure("No flights available"), "Status");
        return;
    }
    const std::string new_status = prompt_flight_status();
    set_flight_status(flight_id, new_status);
    print_status_confirmation(*find_flight(flight_id));
}

void print_report_summary() {
    print_section_title("Revenue and Operations Report");
    std::cout << "Revenue total:    " << format_money({get_recorded_revenue(), "USD"}) << "\n"
              << "\nOperations summary:\n"
              << "  Reservations:    " << get_total_reservations()            << "\n"
              << "  Checked-in:      " << get_total_checkins()                << "\n"
              << "  Delayed flights: " << count_flights_with_status("Delayed") << "\n";
}

} // namespace

// ---------------------------------------------------------------------------
// Public entry points
// ---------------------------------------------------------------------------

// Returns a future timestamp offset by the given number of days.
std::time_t now_plus_days(int days) {
    return std::time(nullptr) + static_cast<std::time_t>(days) * kSecondsPerDay;
}

// Seeds the system with three sample flights and initializes seat inventory.
void initialize_system() {
    add_flight({"FL-101", "ADD", "DXB", now_plus_days(1), now_plus_days(1) + 3 * kSecondsPerHour, {45000, "USD"}, "On Time"});
    add_flight({"FL-102", "ADD", "LHR", now_plus_days(2), now_plus_days(2) + 8 * kSecondsPerHour, {75000, "USD"}, "On Time"});
    add_flight({"FL-201", "DXB", "ADD", now_plus_days(3), now_plus_days(3) + 3 * kSecondsPerHour, {47000, "USD"}, "On Time"});
    initialize_inventory(get_flight_registry(), 120, 24, 12);
}

// Runs the interactive menu loop until the user exits.
void run_repl() {
    std::string line;
    while (true) {
        print_help();
        std::cout << "\nairline> ";
        if (!std::getline(std::cin, line)) break;
        line = trim_copy(line);
        if (line.empty()) continue;

        int choice = 0;
        if (!try_parse_int(line, choice)) {
            std::cout << "Please enter a number between 1 and 7.\n";
            continue;
        }

        if      (choice == 1) handle_search_command();
        else if (choice == 2) handle_book_command();
        else if (choice == 3) handle_checkin_command();
        else if (choice == 4) handle_status_command();
        else if (choice == 5) print_report_summary();
        else if (choice == 6) continue;
        else if (choice == 7) break;
        else std::cout << "Please enter a number between 1 and 7.\n";
    }
}
