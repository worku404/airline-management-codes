/*
Samuel Firegedil
ETS1292/17
*/

#include "interactive_search_helper.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "airport_registry.h"
#include "flight_manager.h"

namespace {
constexpr const char* GREEN = "\033[32m";
constexpr const char* BLUE = "\033[34m";
constexpr const char* RED = "\033[31m";
constexpr const char* RESET = "\033[0m";
constexpr int kSecondsPerDay = 24 * 60 * 60;

void print_divider() {
    std::cout << "==========================================\n";
}

void print_title(const std::string& title) {
    print_divider();
    std::cout << title << "\n";
    print_divider();
}

void prompt_display(const std::string& text) {
    std::cout << BLUE << "? " << RESET << text << "\n";
}

std::string get_user_input() {
    std::cout << "> ";
    std::string input;
    if (std::getline(std::cin, input)) {
        const size_t start = input.find_first_not_of(" \t\r\n");
        const size_t end = input.find_last_not_of(" \t\r\n");
        if (start != std::string::npos) {
            return input.substr(start, end - start + 1);
        }
    }
    return "";
}

std::time_t parse_date(const std::string& date_str) {
    std::istringstream stream(date_str);
    int year = 0;
    int month = 0;
    int day = 0;
    char dash1 = '\0';
    char dash2 = '\0';

    if (!(stream >> year >> dash1 >> month >> dash2 >> day)) {
        return -1;
    }

    if (dash1 != '-' || dash2 != '-') {
        return -1;
    }

    if (month < 1 || month > 12 || day < 1 || day > 31 || year < 2000) {
        return -1;
    }

    std::tm time_info = {};
    time_info.tm_year = year - 1900;
    time_info.tm_mon = month - 1;
    time_info.tm_mday = day;
    time_info.tm_isdst = -1;

    const std::time_t result = std::mktime(&time_info);
    return result == -1 ? -1 : result;
}

bool is_valid_date_format(const std::string& date_str) {
    if (date_str.length() != 10) {
        return false;
    }
    if (date_str[4] != '-' || date_str[7] != '-') {
        return false;
    }

    for (size_t i = 0; i < date_str.length(); ++i) {
        if (i == 4 || i == 7) {
            continue;
        }
        if (!std::isdigit(static_cast<unsigned char>(date_str[i]))) {
            return false;
        }
    }
    return true;
}

bool is_date_future(std::time_t timestamp) {
    const std::time_t now = std::time(nullptr);
    std::tm* now_info = std::localtime(&now);
    if (now_info == nullptr) {
        return false;
    }

    std::tm today = *now_info;
    today.tm_hour = 0;
    today.tm_min = 0;
    today.tm_sec = 0;
    const std::time_t today_midnight = std::mktime(&today);
    return timestamp >= today_midnight;
}

std::string format_date(std::time_t timestamp) {
    const std::tm* time_info = std::localtime(&timestamp);
    char buffer[11] = {};
    if (time_info != nullptr) {
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", time_info);
    }
    return buffer;
}
}

std::string get_airport_from_user(const std::string& prompt_text) {
    prompt_display(prompt_text);

    const auto all_airports = list_all_airports();
    int page = 0;
    const int airports_per_page = 10;

    while (true) {
        std::cout << "\nAvailable Airports:\n";

        const int start = page * airports_per_page;
        const int end = std::min(start + airports_per_page,
                                 static_cast<int>(all_airports.size()));

        for (int i = start; i < end; ++i) {
            std::cout << "  " << (i - start + 1) << ". "
                      << get_airport_display_string(all_airports[i]) << "\n";
        }

        if (all_airports.size() > static_cast<size_t>(airports_per_page)) {
            std::cout << "\n";
            if (page > 0) {
                std::cout << "  [P] Previous page\n";
            }
            if (end < static_cast<int>(all_airports.size())) {
                std::cout << "  [N] Next page\n";
            }
        }

        prompt_display("Select airport (enter number)");
        const std::string input = get_user_input();

        if ((input == "N" || input == "n") && end < static_cast<int>(all_airports.size())) {
            ++page;
            continue;
        }
        if ((input == "P" || input == "p") && page > 0) {
            --page;
            continue;
        }

        try {
            const int selection = std::stoi(input);
            const int actual_index = start + selection - 1;

            if (selection >= 1 && actual_index < static_cast<int>(all_airports.size())) {
                const AirportInfo& chosen = all_airports[actual_index];
                std::cout << GREEN << "OK: " << RESET
                          << get_airport_display_string(chosen) << "\n\n";
                return chosen.iata_code;
            }
            std::cout << RED << "Invalid selection.\n" << RESET;
        } catch (const std::exception&) {
            std::cout << RED << "Invalid input.\n" << RESET;
        }
    }
}

std::time_t get_date_from_user(const std::string& prompt_text) {
    while (true) {
        prompt_display(prompt_text);
        const std::string date_input = get_user_input();

        if (date_input.empty()) {
            std::cout << RED << "Date cannot be empty.\n" << RESET;
            continue;
        }

        if (!is_valid_date_format(date_input)) {
            std::cout << RED << "Invalid format. Use YYYY-MM-DD.\n" << RESET;
            continue;
        }

        const std::time_t parsed_date = parse_date(date_input);
        if (parsed_date == -1) {
            std::cout << RED << "Invalid date.\n" << RESET;
            continue;
        }

        if (!is_date_future(parsed_date)) {
            std::cout << RED << "Date must be today or later.\n" << RESET;
            continue;
        }

        std::cout << GREEN << "OK: " << RESET << date_input << "\n\n";
        return parsed_date;
    }
}

int get_search_range_days() {
    while (true) {
        prompt_display("Search range in days? (1-365)");
        const std::string input = get_user_input();

        if (input.empty()) {
            std::cout << RED << "Input cannot be empty.\n" << RESET;
            continue;
        }

        try {
            const int days = std::stoi(input);
            if (days >= 1 && days <= 365) {
                std::cout << GREEN << "OK: " << RESET << days << " day"
                          << (days == 1 ? "" : "s") << "\n\n";
                return days;
            }
            std::cout << RED << "Enter a value between 1 and 365.\n" << RESET;
        } catch (const std::exception&) {
            std::cout << RED << "Invalid input.\n" << RESET;
        }
    }
}

SearchCriteria get_interactive_search() {
    print_title("Flight Search Setup");
    std::cout << "\n";

    const std::string origin = get_airport_from_user("Select departure airport:");

    std::string destination;
    while (true) {
        destination = get_airport_from_user("Select arrival airport:");
        if (destination != origin) {
            break;
        }
        std::cout << RED << "Arrival airport must be different from departure airport.\n"
                  << RESET;
    }

    const std::time_t departure_date = get_date_from_user("Departure date (YYYY-MM-DD):");
    const int search_days = get_search_range_days();

    const std::time_t end_date =
        departure_date +
        static_cast<std::time_t>(search_days - 1) * kSecondsPerDay +
        (kSecondsPerDay - 1);

    std::cout << GREEN << "Searching: " << RESET << origin << " -> "
              << destination << "\n";
    std::cout << GREEN << "Dates: " << RESET
              << format_date(departure_date) << " to "
              << format_date(end_date) << "\n\n";

    print_divider();
    std::cout << "\n";

    return SearchCriteria{origin, destination, departure_date, end_date};
}
