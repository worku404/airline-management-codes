/*
Yared Tsehaye
ETS1488/17
*/
#pragma once
#include "common_types.h"

// Calculates the ticket price based on how full the flight is.
// High demand (<=10% seats left): 1.5x price
// Moderate demand (<=25% seats left): 1.2x price
// Normal: base price
Money calculate_dynamic_price(const Money& base_price,
                              int remaining_seats,
                              int total_capacity,
                              Status& status);
