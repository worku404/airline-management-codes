#pragma once

#include <string>

#include "common_types.h"

std::string mask_identifier(const std::string& value, std::size_t visible_suffix = 2);
void log_status(const Status& status, const std::string& context);