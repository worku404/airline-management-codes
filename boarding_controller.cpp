#include "boarding_controller.h"

#include <map>

#include "flight_manager.h"
#include "reservation_engine.h"
#include "validator.h"

namespace {
int g_total_checkins = 0;

int boarding_group_for(SeatClass seat_class) {
    switch (seat_class) {
        case SeatClass::First:
            return 1;
        case SeatClass::Business:
            return 2;
        case SeatClass::Economy:
        default:
            return 3;
    }
}

std::string gate_for(const std::string& flight_id) {
    if (flight_id.empty()) {
        return "G0";
    }
    char suffix = flight_id.back();
    if (suffix < '0' || suffix > '9') {
        suffix = '1';
    }
    return std::string("G") + suffix;
}
}

CheckInResult process_check_in(const std::string& pnr_id, int baggage_count) {
    if (!is_valid_pnr(pnr_id)) {
        return {{}, make_failure("CHECKIN_PNR_INVALID", "Invalid PNR format"), baggage_count};
    }
    if (!is_valid_baggage_count(baggage_count)) {
        return {{}, make_failure("CHECKIN_BAGGAGE_INVALID", "Invalid baggage count"), baggage_count};
    }
    const ReservationRecord* record = find_reservation(pnr_id);
    if (record == nullptr) {
        return {{}, make_failure("CHECKIN_PNR_MISSING", "PNR not found"), baggage_count};
    }
    if (record->status != ReservationStatus::Reserved) {
        return {{}, make_failure("CHECKIN_STATE_INVALID", "Reservation not eligible for check-in"), baggage_count};
    }

    BoardingPass pass{pnr_id, gate_for(record->request.flight_id), boarding_group_for(record->request.preferred_class)};

    Status status = update_reservation_status(pnr_id, ReservationStatus::CheckedIn);
    if (!status.success) {
        return {{}, status, baggage_count};
    }

    ++g_total_checkins;
    return {pass, make_success(), baggage_count};
}

Status update_flight_status(const std::string& flight_id, const std::string& new_status) {
    return set_flight_status(flight_id, new_status);
}

int get_total_checkins() {
    return g_total_checkins;
}

int get_total_boarded() {
    return g_total_checkins;
}