/*
Samuel Firegedil
ETS1292/17
*/
#include "interactive_search_helper.h"
#include "airport_registry.h"
#include "validator.h"
#include <algorithm>
#include <cctype>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

static const int SECONDS_PER_DAY = 24 * 60 * 60;

static std::string get_user_input() {
    std::cout << "> ";
    std::string input;
    if (!std::getline(std::cin, input)) return "";
    // trim leading and trailing whitespace using int-indexed loop
    int start = 0, end = (int)input.size() - 1;
    while (start <= end && (input[start]==' '||input[start]=='\t'||input[start]=='\r'||input[start]=='\n')) start++;
    while (end >= start && (input[end]==' '||input[end]=='\t'||input[end]=='\r'||input[end]=='\n')) end--;
    if (start > end) return "";
    return input.substr(start, end - start + 1);
}

static std::time_t parse_date(const std::string& date_str) {
    std::istringstream stream(date_str);
    int year=0, month=0, day=0; char d1=0, d2=0;
    stream >> year >> d1 >> month >> d2 >> day;
    if (d1!='-' || d2!='-') return -1;
    if (month<1||month>12||day<1||day>31||year<2020) return -1;
    std::tm t = {}; t.tm_year=year-1900; t.tm_mon=month-1; t.tm_mday=day; t.tm_isdst=-1;
    std::time_t result = std::mktime(&t);
    return (result == -1) ? -1 : result;
}

static bool is_date_today_or_future(std::time_t timestamp) {
    std::time_t now = std::time(nullptr);
    std::tm* now_info = std::localtime(&now);
    std::tm today = *now_info;
    today.tm_hour=0; today.tm_min=0; today.tm_sec=0;
    std::time_t today_midnight = std::mktime(&today);
    return timestamp >= today_midnight;
}

static std::string format_date(std::time_t timestamp) {
    const std::tm* t = std::localtime(&timestamp);
    char buf[11] = {};
    if (t != nullptr) std::strftime(buf, sizeof(buf), "%Y-%m-%d", t);
    return std::string(buf);
}

std::string get_airport_from_user(const std::string& prompt_text) {
    std::cout << "? " << prompt_text << "\n";
    const std::vector<AirportInfo>& airports = list_all_airports();
    // Print ALL airports as a numbered list (NO pagination)
    std::cout << "\nAvailable Airports:\n";
    for (int i = 0; i < (int)airports.size(); i++) {
        std::cout << "  " << (i+1) << ". " << get_airport_display_string(airports[i]) << "\n";
    }
    // Loop until valid selection
    while (true) {
        std::cout << "\n? Select airport number:\n";
        const std::string input = get_user_input();
        int selection = 0;
        if (try_parse_int(input, selection) && selection >= 1 && selection <= (int)airports.size()) {
            const AirportInfo& chosen = airports[selection - 1];
            std::cout << "OK: " << get_airport_display_string(chosen) << "\n\n";
            return chosen.iata_code;
        }
        std::cout << "Invalid selection. Please enter a number between 1 and " << airports.size() << ".\n";
    }
}

std::time_t get_date_from_user(const std::string& prompt_text) {
    while (true) {
        std::cout << "? " << prompt_text << "\n";
        std::string input = get_user_input();
        if (input.empty()) { std::cout << "Date cannot be empty.\n"; continue; }
        std::time_t parsed = parse_date(input);
        if (parsed == -1) { std::cout << "Invalid date format. Use YYYY-MM-DD (e.g. 2026-06-15).\n"; continue; }
        if (!is_date_today_or_future(parsed)) { std::cout << "Date must be today or in the future.\n"; continue; }
        std::cout << "OK: " << input << "\n\n";
        return parsed;
    }
}

int get_search_range_days() {
    while (true) {
        std::cout << "? Search range in days? (1-365):\n";
        std::string input = get_user_input();
        if (input.empty()) { std::cout << "Please enter a number.\n"; continue; }
        int days = 0;
        if (try_parse_int(input, days) && days>=1 && days<=365) {
            std::cout << "OK: " << days << " day" << (days==1?"":"s") << "\n\n";
            return days;
        }
        std::cout << "Enter a value between 1 and 365.\n";
    }
}

SearchCriteria get_interactive_search() {
    std::cout << "==========================================\n";
    std::cout << "         Flight Search Setup\n";
    std::cout << "==========================================\n\n";
    std::string origin = get_airport_from_user("Select departure airport:");
    std::string destination;
    while (true) {
        destination = get_airport_from_user("Select arrival airport:");
        if (destination != origin) break;
        std::cout << "Arrival airport must be different from departure airport.\n";
    }
    std::time_t departure_date = get_date_from_user("Departure date (YYYY-MM-DD):");
    int search_days = get_search_range_days();
    std::time_t end_date = departure_date + (std::time_t)(search_days - 1) * SECONDS_PER_DAY + (SECONDS_PER_DAY - 1);
    std::cout << "Searching: " << origin << " -> " << destination << "\n";
    std::cout << "Dates: " << format_date(departure_date) << " to " << format_date(end_date) << "\n\n";
    SearchCriteria criteria;
    criteria.origin = origin;
    criteria.destination = destination;
    criteria.date_window_start = departure_date;
    criteria.date_window_end = end_date;
    return criteria;
}
