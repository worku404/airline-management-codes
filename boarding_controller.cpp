/*
Yared Tsehaye
ETS1488/17
*/
#include "boarding_controller.h"
#include "reservation_engine.h"

static std::string gate_for(const std::string& flight_id) {
    if (flight_id.empty()) {
        return "G0";
    }
    char last = flight_id.back();
    if (last >= '0' && last <= '9') {
        return std::string("G") + last;
    }
    return "G1";
}

static int boarding_group_for(SeatClass sc) {
    if (sc == SeatClass::First) {
        return 1;
    }
    if (sc == SeatClass::Business) {
        return 2;
    }
    return 3;  // Economy is default
}

CheckInResult check_in_passenger(const std::string& pnr_id, int baggage_count) {
    const ReservationRecord* rec = find_reservation(pnr_id);
    if (rec == nullptr) {
        return {BoardingPass{}, make_failure("PNR not found"), baggage_count};
    }
    if (rec->status != ReservationStatus::Reserved) {
        return {BoardingPass{}, make_failure("Passenger is already checked in"), baggage_count};
    }
    BoardingPass pass;
    pass.pnr_id         = pnr_id;
    pass.gate           = gate_for(rec->request.flight_id);
    pass.boarding_group = boarding_group_for(rec->request.preferred_class);
    Status s = mark_checked_in(pnr_id);
    if (!s.success) {
        return {BoardingPass{}, s, baggage_count};
    }
    return {pass, make_success(), baggage_count};
}
