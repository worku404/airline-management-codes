/*
Worku Wondoson
ETS1459/17
*/

#include "controller.h"

#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "boarding_controller.h"
#include "file_manager.h"
#include "flight_manager.h"
#include "interactive_search_helper.h"
#include "reservation_engine.h"
#include "validator.h"

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

static const int SECONDS_PER_HOUR = 3600;
static const int SECONDS_PER_DAY  = 24 * SECONDS_PER_HOUR;

static const std::string FLIGHT_STATUSES[] = {"On Time", "Delayed", "Boarding", "Cancelled"};
static const int         NUM_STATUSES      = 4;

// Caches the results of the most recent flight search for use in booking
static std::vector<Flight> g_search_results;

// ---------------------------------------------------------------------------
// Display helpers
// ---------------------------------------------------------------------------

static void print_divider() {
    std::cout << std::string(62, '=') << "\n";
}

static void print_section_title(const std::string& title) {
    std::cout << "\n";
    print_divider();
    std::cout << "  " << title << "\n";
    print_divider();
}

static void print_hint(const std::string& text) {
    std::cout << "Hint: " << text << "\n";
}

// Formats a Money value as a decimal string: "450.00 USD"
static std::string format_money(const Money& amount) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2)
       << (double)amount.amount_cents / 100.0
       << " " << amount.currency;
    return ss.str();
}

// Formats a Unix timestamp as "YYYY-MM-DD HH:MM"
static std::string format_date_time(std::time_t timestamp) {
    const std::tm* t = std::localtime(&timestamp);
    char buf[17] = {};
    if (t != nullptr) {
        std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", t);
    }
    return std::string(buf);
}

// Prints a numbered flight row with seat availability
static void print_flight_row(int index, const Flight& f) {
    std::cout << "  " << index << ". " << f.flight_id
              << " | " << f.origin_iata << " -> " << f.destination_iata
              << " | Depart " << format_date_time(f.departure_time)
              << " | " << format_money(f.base_price)
              << " | " << f.status << "\n"
              << "     Seats: Economy " << f.economy_seats
              << ", Business "          << f.business_seats
              << ", First "             << f.first_seats << "\n";
}

// Prints a success or error message with a context label
static void print_status(const Status& s, const std::string& context) {
    if (s.success) {
        std::cout << "OK: " << context << "\n";
    } else {
        std::cout << "Error [" << context << "]: " << s.message << "\n";
    }
}

// ---------------------------------------------------------------------------
// Input helpers
// ---------------------------------------------------------------------------

// Trims leading and trailing whitespace from a string
static std::string trim_copy(const std::string& value) {
    int start = 0;
    int end   = (int)value.size() - 1;
    while (start <= end && (value[start] == ' ' || value[start] == '\t' ||
                             value[start] == '\r' || value[start] == '\n')) {
        start++;
    }
    while (end >= start && (value[end] == ' ' || value[end] == '\t' ||
                              value[end] == '\r' || value[end] == '\n')) {
        end--;
    }
    if (start > end) return "";
    return value.substr(start, end - start + 1);
}

// Prints a label then reads and trims one line of input
static std::string prompt_line(const std::string& label) {
    std::cout << label << "\n> ";
    std::string input;
    std::getline(std::cin, input);
    return trim_copy(input);
}

// Loops until the user types a non-empty string
static std::string prompt_non_empty(const std::string& label) {
    while (true) {
        std::string input = prompt_line(label);
        if (!input.empty()) return input;
        std::cout << "This field cannot be empty. Please try again.\n";
    }
}

// Returns a future timestamp offset by the given number of days from now
static std::time_t now_plus_days(int days) {
    return std::time(nullptr) + (std::time_t)days * SECONDS_PER_DAY;
}

// ---------------------------------------------------------------------------
// Interactive prompts
// ---------------------------------------------------------------------------

// Shows either the last search results or all flights; returns chosen flight ID
static std::string prompt_flight_from_search() {
    const std::vector<Flight>& flights =
        !g_search_results.empty() ? g_search_results : get_flight_registry();

    if (flights.empty()) {
        std::cout << "No flights available.\n";
        return "";
    }

    print_section_title(!g_search_results.empty()
        ? "Choose a Flight From Your Search" : "All Available Flights");
    for (int i = 0; i < (int)flights.size(); i++) {
        print_flight_row(i + 1, flights[i]);
    }

    while (true) {
        std::string input = prompt_line("Choose a flight by number:");
        int selection = 0;
        if (try_parse_int(input, selection) &&
            selection >= 1 && selection <= (int)flights.size()) {
            return flights[selection - 1].flight_id;
        }
        std::cout << "Please enter a number between 1 and " << flights.size() << ".\n";
    }
}

// Shows all registered flights; returns chosen flight ID
static std::string prompt_flight_from_registry() {
    const std::vector<Flight>& flights = get_flight_registry();
    if (flights.empty()) {
        std::cout << "No flights available.\n";
        return "";
    }

    print_section_title("All Available Flights");
    for (int i = 0; i < (int)flights.size(); i++) {
        print_flight_row(i + 1, flights[i]);
    }

    while (true) {
        std::string input = prompt_line("Choose a flight by number:");
        int selection = 0;
        if (try_parse_int(input, selection) &&
            selection >= 1 && selection <= (int)flights.size()) {
            return flights[selection - 1].flight_id;
        }
        std::cout << "Please enter a number between 1 and " << flights.size() << ".\n";
    }
}

// Returns the SeatClass chosen by the user (defaults to Economy on empty input)
static SeatClass prompt_seat_class() {
    while (true) {
        std::cout << "  Cabin classes:\n"
                  << "    1. Economy\n"
                  << "    2. Business\n"
                  << "    3. First\n";
        std::string input = prompt_line("Choose a class (press Enter for Economy):");
        if (input.empty()) return SeatClass::Economy;

        int selection = 0;
        if (try_parse_int(input, selection)) {
            if (selection == 1) return SeatClass::Economy;
            if (selection == 2) return SeatClass::Business;
            if (selection == 3) return SeatClass::First;
        }
        SeatClass out;
        if (parse_seat_class(input, out).success) return out;
        std::cout << "Please choose 1, 2, 3 or type Economy / Business / First.\n";
    }
}

// Returns a validated passport number (6-9 uppercase alphanumeric)
static std::string prompt_passport() {
    while (true) {
        std::string passport = to_upper(prompt_line("Passenger passport number (6-9 uppercase letters/digits):"));
        if (is_valid_passport(passport)) return passport;
        std::cout << "Invalid passport format. Use 6-9 uppercase letters or digits (e.g. AB123456).\n";
    }
}

// Returns a valid seat number, or "" to auto-assign
static std::string prompt_seat_number() {
    while (true) {
        std::string seat = to_upper(prompt_line("Preferred seat (e.g. A1, 12E) — press Enter to auto-assign:"));
        if (seat.empty()) return "";
        if (is_valid_seat_number(seat)) return seat;
        std::cout << "Invalid format. Use a letter then digits (A1) or digits then a letter (12A).\n";
    }
}

// Asks the user to type their PNR
static std::string prompt_pnr() {
    while (true) {
        std::string raw = to_upper(prompt_line("Enter your 6-character PNR code (e.g. AB1234):"));
        // Strip optional "PNR-" prefix
        if (raw.size() > 4 && raw.substr(0, 4) == "PNR-") {
            raw = raw.substr(4);
        }
        if (is_valid_pnr(raw)) return raw;
        std::cout << "A PNR must be exactly 6 uppercase letters or digits. Please try again.\n";
    }
}

// Returns a baggage count from 0 to 5 (defaults to 0 on empty input)
static int prompt_baggage_count() {
    while (true) {
        std::string input = prompt_line("Number of checked bags (0-5, press Enter for 0):");
        if (input.empty()) return 0;
        int count = 0;
        if (try_parse_int(input, count) && is_valid_baggage_count(count)) return count;
        std::cout << "Please enter a whole number between 0 and 5.\n";
    }
}

// Returns one of the four canonical flight status strings
static std::string prompt_flight_status() {
    while (true) {
        std::cout << "  Flight statuses:\n";
        for (int i = 0; i < NUM_STATUSES; i++) {
            std::cout << "    " << (i + 1) << ". " << FLIGHT_STATUSES[i] << "\n";
        }
        std::string input = prompt_line("Choose a status by number or name:");
        int selection = 0;
        if (try_parse_int(input, selection) &&
            selection >= 1 && selection <= NUM_STATUSES) {
            return FLIGHT_STATUSES[selection - 1];
        }
        // Try matching by name (case-insensitive)
        std::string upper = to_upper(input);
        for (int i = 0; i < NUM_STATUSES; i++) {
            if (to_upper(FLIGHT_STATUSES[i]) == upper) return FLIGHT_STATUSES[i];
        }
        std::cout << "Please choose a number 1-4 or type the status name.\n";
    }
}

// ---------------------------------------------------------------------------
// Confirmation printers
// ---------------------------------------------------------------------------

static void print_search_results(const std::vector<Flight>& flights) {
    print_section_title("Search Results");
    std::cout << flights.size()
              << (flights.size() == 1 ? " flight found.\n" : " flights found.\n");
    for (int i = 0; i < (int)flights.size(); i++) {
        print_flight_row(i + 1, flights[i]);
    }
    print_hint("Select option 2 from the menu to book one of these flights.");
}

static void print_booking_confirmation(const BookingResult& result) {
    print_section_title("Booking Confirmed");
    const ReservationRecord* rec = find_reservation(result.pnr_id);
    if (rec != nullptr) {
        std::cout << "  PNR:       PNR-" << rec->pnr_id << "\n"
                  << "  Passenger: " << rec->request.passenger.first_name
                  << " "             << rec->request.passenger.last_name << "\n"
                  << "  Flight:    " << rec->request.flight_id << "\n"
                  << "  Class:     " << seat_class_to_string(rec->request.preferred_class) << "\n"
                  << "  Seat:      " << rec->request.seat_number << "\n";
    }
    std::cout << "  Total:     " << format_money(result.total_cost) << "\n";
    print_hint("Select option 3 to check in for this booking.");
}

static void print_checkin_confirmation(const CheckInResult& result) {
    print_section_title("Check-in Complete");
    const ReservationRecord* rec = find_reservation(result.pass.pnr_id);
    std::cout << "  PNR:            PNR-" << result.pass.pnr_id << "\n";
    if (rec != nullptr) {
        std::cout << "  Passenger:      " << rec->request.passenger.first_name
                  << " "                  << rec->request.passenger.last_name << "\n"
                  << "  Flight:         " << rec->request.flight_id << "\n"
                  << "  Seat:           " << rec->request.seat_number << "\n";
    }
    std::cout << "  Gate:           " << result.pass.gate << "\n"
              << "  Boarding group: " << result.pass.boarding_group << "\n"
              << "  Checked bags:   " << result.baggage_count << "\n";
    print_hint("Select option 5 to view the operations summary.");
}

static void print_status_confirmation(const Flight& flight) {
    print_section_title("Flight Status Updated");
    std::cout << "  Flight:     " << flight.flight_id << "\n"
              << "  Route:      " << flight.origin_iata << " -> " << flight.destination_iata << "\n"
              << "  New status: " << flight.status << "\n";
}

// ---------------------------------------------------------------------------
// Command handlers
// ---------------------------------------------------------------------------

static void handle_search_command() {
    SearchCriteria criteria = get_interactive_search();
    std::vector<Flight> results = search_flights(criteria);
    g_search_results = results;

    if (results.empty()) {
        print_section_title("Search Results");
        std::cout << "No flights matched your search.\n";
        print_hint("Try a wider date range or a different route.");
        return;
    }
    print_search_results(results);
}

static void handle_book_command() {
    std::string flight_id = prompt_flight_from_search();
    if (flight_id.empty()) {
        std::cout << "No flights to book. Please search first (option 1).\n";
        return;
    }

    SeatClass   seat_class  = prompt_seat_class();
    std::string first_name  = prompt_non_empty("Passenger first name:");
    std::string last_name   = prompt_non_empty("Passenger last name:");
    std::string passport    = prompt_passport();
    std::string seat_number = prompt_seat_number();

    BookingRequest request;
    request.flight_id               = flight_id;
    request.passenger.first_name    = first_name;
    request.passenger.last_name     = last_name;
    request.passenger.passport_number = passport;
    request.preferred_class         = seat_class;
    request.seat_number             = seat_number;

    BookingResult result = book_flight(request);
    if (!result.status.success) {
        print_status(result.status, "Booking");
        return;
    }
    print_booking_confirmation(result);
    save_data("airline_data.bin");
}

static void handle_checkin_command() {
    std::string pnr    = prompt_pnr();
    int         baggage = prompt_baggage_count();

    CheckInResult result = check_in_passenger(pnr, baggage);
    if (!result.status.success) {
        print_status(result.status, "Check-in");
        return;
    }
    print_checkin_confirmation(result);
    save_data("airline_data.bin");
}

static void handle_status_command() {
    std::string flight_id = prompt_flight_from_registry();
    if (flight_id.empty()) return;

    std::string new_status = prompt_flight_status();
    Status s = set_flight_status(flight_id, new_status);
    if (!s.success) {
        print_status(s, "Status Update");
        return;
    }
    Flight* f = find_flight(flight_id);
    if (f != nullptr) print_status_confirmation(*f);
    save_data("airline_data.bin");
}

static void print_report_summary() {
    print_section_title("Revenue and Operations Report");
    std::cout << "  Revenue total:    " << format_money({get_recorded_revenue(), "USD"}) << "\n"
              << "\n  Operations summary:\n"
              << "    Reservations:    " << get_total_reservations() << "\n"
              << "    Checked-in:      " << get_total_checkins()     << "\n"
              << "    Delayed flights: " << count_flights_with_status("Delayed") << "\n";
}

static void print_help() {
    print_section_title("Airline Management System");
    std::cout << "    1. Search flights\n"
              << "    2. Book a flight\n"
              << "    3. Check in a passenger\n"
              << "    4. Update a flight status\n"
              << "    5. Revenue and operations summary\n"
              << "    6. Show this menu\n"
              << "    7. Exit\n";
}

// ---------------------------------------------------------------------------
// Public entry points
// ---------------------------------------------------------------------------

void initialize_system() {
    // Seed the random number generator (used for PNR generation)
    srand((unsigned int)std::time(nullptr));

    // Add three sample flights (seat counts also serve as initial capacity)
    Flight fl;

    fl.flight_id        = "FL-101";
    fl.origin_iata      = "ADD";
    fl.destination_iata = "DXB";
    fl.departure_time   = now_plus_days(1);
    fl.arrival_time     = now_plus_days(1) + 3 * SECONDS_PER_HOUR;
    fl.base_price       = {45000LL, "USD"};
    fl.status           = "On Time";
    fl.economy_seats    = 120;
    fl.business_seats   = 24;
    fl.first_seats      = 12;
    add_flight(fl);

    fl.flight_id        = "FL-102";
    fl.origin_iata      = "ADD";
    fl.destination_iata = "LHR";
    fl.departure_time   = now_plus_days(2);
    fl.arrival_time     = now_plus_days(2) + 8 * SECONDS_PER_HOUR;
    fl.base_price       = {75000LL, "USD"};
    fl.status           = "On Time";
    fl.economy_seats    = 120;
    fl.business_seats   = 24;
    fl.first_seats      = 12;
    add_flight(fl);

    fl.flight_id        = "FL-201";
    fl.origin_iata      = "DXB";
    fl.destination_iata = "ADD";
    fl.departure_time   = now_plus_days(3);
    fl.arrival_time     = now_plus_days(3) + 3 * SECONDS_PER_HOUR;
    fl.base_price       = {47000LL, "USD"};
    fl.status           = "On Time";
    fl.economy_seats    = 120;
    fl.business_seats   = 24;
    fl.first_seats      = 12;
    add_flight(fl);

    // Restore any previously saved bookings and flight states
    load_data("airline_data.bin");
}

void run_repl() {
    print_help();
    std::string line;

    while (true) {
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
        else if (choice == 6) print_help();
        else if (choice == 7) break;
        else std::cout << "Please enter a number between 1 and 7.\n";
    }
}
