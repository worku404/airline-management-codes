#include "security_utils.h"

#include <iostream>

std::string mask_identifier(const std::string& value, std::size_t visible_suffix) {
    if (value.size() <= visible_suffix) {
        return value;
    }
    return std::string(value.size() - visible_suffix, '*') + value.substr(value.size() - visible_suffix);
}

void log_status(const Status& status, const std::string& context) {
    if (status.success) {
        return;
    }
    std::cerr << "Error: " << context << ": " << status.message;
    if (!status.error_code.empty()) {
        std::cerr << " [" << status.error_code << "]";
    }
    std::cerr << '\n';
}