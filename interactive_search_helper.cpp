#include "interactive_search_helper.h"

#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "airport_registry.h"
#include "flight_manager.h"

namespace {
    constexpr const char* GREEN = "\033[32m";
    constexpr const char* BLUE = "\033[34m";
    constexpr const char* RED = "\033[31m";
    constexpr const char* RESET = "\033[0m";
    constexpr int SECONDS_PER_DAY = 24 * 60 * 60;

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
            size_t start = input.find_first_not_of(" \t\r\n");
            size_t end = input.find_last_not_of(" \t\r\n");
            if (start != std::string::npos) {
                return input.substr(start, end - start + 1);
            }
        }
        return "";
    }

    std::time_t parse_date(const std::string& date_str) {
        std::istringstream iss(date_str);
        int year, month, day;
        char dash1, dash2;

        if (!(iss >> year >> dash1 >> month >> dash2 >> day)) {
            return -1;
        }

        if (dash1 != '-' || dash2 != '-') {
            return -1;
        }

        if (month < 1 || month > 12 || day < 1 || day > 31 || year < 2000) {
            return -1;
        }

        struct tm time_info = {};
        time_info.tm_year = year - 1900;
        time_info.tm_mon = month - 1;
        time_info.tm_mday = day;
        time_info.tm_isdst = -1;

        std::time_t result = std::mktime(&time_info);
        return result == -1 ? -1 : result;
    }

    bool is_valid_date_format(const std::string& date_str) {
        if (date_str.length() != 10) return false;
        if (date_str[4] != '-' || date_str[7] != '-') return false;

        for (size_t i = 0; i < date_str.length(); ++i) {
            if (i == 4 || i == 7) continue;
            if (!std::isdigit(static_cast<unsigned char>(date_str[i]))) {
                return false;
            }
        }
        return true;
    }

    bool is_date_future(std::time_t timestamp) {
        std::time_t now = std::time(nullptr);
        struct tm* now_info = std::localtime(&now);
        struct tm today = *now_info;
        today.tm_hour = 0;
        today.tm_min = 0;
        today.tm_sec = 0;
        std::time_t today_midnight = std::mktime(&today);
        return timestamp >= today_midnight;
    }

    std::string format_date(std::time_t timestamp) {
        struct tm* time_info = std::localtime(&timestamp);
        char buffer[11];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", time_info);
        return std::string(buffer);
    }
}

std::string get_airport_from_user(const std::string& prompt_text) {
    prompt_display(prompt_text);

    auto all_airports = list_all_airports();
    int page = 0;
    const int airports_per_page = 10;

    while (true) {
        std::cout << "\nAvailable Airports:\n";

        int start = page * airports_per_page;
        int end = std::min(start + airports_per_page, 
                          static_cast<int>(all_airports.size()));

        for (int i = start; i < end; ++i) {
            std::cout << "  " << (i + 1) << ". "
                      << get_airport_display_string(all_airports[i]) << "\n";
        }

        if (all_airports.size() > airports_per_page) {
            std::cout << "\n";
            if (page > 0) {
                std::cout << "  [P] Previous page\n";
            }
            if (end < static_cast<int>(all_airports.size())) {
                std::cout << "  [N] Next page\n";
            }
        }

        prompt_display("Select airport (enter number)");
        std::string input = get_user_input();

        if (input == "N" || input == "n") {
            if (end < static_cast<int>(all_airports.size())) {
                page++;
                continue;
            }
        } else if (input == "P" || input == "p") {
            if (page > 0) {
                page--;
                continue;
            }
        } else {
            try {
                int selection = std::stoi(input);
                int actual_index = start + selection - 1;

                if (actual_index >= 0 && 
                    actual_index < static_cast<int>(all_airports.size())) {
                    const AirportInfo& chosen = all_airports[actual_index];
                    std::cout << GREEN << "✓ Selected: " << RESET
                              << get_airport_display_string(chosen) << "\n\n";
                    return chosen.iata_code;
                } else {
                    std::cout << RED << "✗ Invalid selection\n" << RESET;
                }
            } catch (const std::exception&) {
                std::cout << RED << "✗ Invalid input\n" << RESET;
            }
        }
    }
}

std::time_t get_date_from_user(const std::string& prompt_text) {
    while (true) {
        prompt_display(prompt_text);
        std::string date_input = get_user_input();

        if (date_input.empty()) {
            std::cout << RED << "✗ Date cannot be empty\n" << RESET;
            continue;
        }

        if (!is_valid_date_format(date_input)) {
            std::cout << RED << "✗ Invalid format. Use YYYY-MM-DD\n" << RESET;
            continue;
        }

        std::time_t parsed_date = parse_date(date_input);

        if (parsed_date == -1) {
            std::cout << RED << "✗ Invalid date\n" << RESET;
            continue;
        }

        if (!is_date_future(parsed_date)) {
            std::cout << RED << "✗ Date must be today or later\n" << RESET;
            continue;
        }

        std::cout << GREEN << "✓ Date: " << RESET << date_input << "\n\n";
        return parsed_date;
    }
}

int get_search_range_days() {
    while (true) {
        prompt_display("Search range in days? (1-365)");
        std::string input = get_user_input();

        if (input.empty()) {
            std::cout << RED << "✗ Input cannot be empty\n" << RESET;
            continue;
        }

        try {
            int days = std::stoi(input);

            if (days >= 1 && days <= 365) {
                std::cout << GREEN << "✓ Range: " << RESET << days 
                          << " days\n\n";
                return days;
            } else {
                std::cout << RED << "✗ Enter value between 1 and 365\n" 
                          << RESET;
            }
        } catch (const std::exception&) {
            std::cout << RED << "✗ Invalid input\n" << RESET;
        }
    }
}

SearchCriteria get_interactive_search() {
    print_title("Flight Search Setup");
    std::cout << "\n";

    std::string origin = get_airport_from_user("Select departure airport:");
    std::string destination = get_airport_from_user("Select arrival airport:");

    std::time_t departure_date = get_date_from_user("Departure date (YYYY-MM-DD):");
    int search_days = get_search_range_days();

    std::time_t end_date = departure_date + (search_days * SECONDS_PER_DAY);

    std::cout << GREEN << "Searching. . . : " << RESET << origin << " → " 
              << destination << "\n";
    std::cout << GREEN << "✓ Dates: " << RESET 
              << format_date(departure_date) << " to " 
              << format_date(end_date) << "\n\n";

    print_divider();
    std::cout << "\n";

    return SearchCriteria{origin, destination, departure_date, end_date};
}