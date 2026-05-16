#pragma once

#include <ctime>
#include <string>
#include <vector>

struct SearchCriteria;
struct AirportInfo;

// Display list of airports and get user selection
std::string get_airport_from_user(const std::string& prompt_text);

// Get date from user (YYYY-MM-DD format)
std::time_t get_date_from_user(const std::string& prompt_text);

// Get search range in days
int get_search_range_days();

// Interactive search flow - returns complete SearchCriteria
SearchCriteria get_interactive_search();