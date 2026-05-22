/*
Worku Wondoson
ETS1459/17
*/
#pragma once
#include <string>

// Saves all reservations and flight state to a binary file
void save_data(const std::string& filepath);

// Loads reservations and flight state back from a binary file
void load_data(const std::string& filepath);
