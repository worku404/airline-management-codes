/*
Samuel Firegedil
ETS1292/17
*/
#pragma once
#include <ctime>
#include <string>
#include "flight_manager.h"   // for SearchCriteria

// Guides the user through selecting airports and a date range
std::string    get_airport_from_user(const std::string& prompt_text);
std::time_t    get_date_from_user(const std::string& prompt_text);
int            get_search_range_days();
SearchCriteria get_interactive_search();
