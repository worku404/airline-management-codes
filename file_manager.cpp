/*
Worku Wondoson
ETS1459/17
*/
#include "file_manager.h"
#include "reservation_engine.h"
#include "flight_manager.h"
#include <fstream>
#include <iostream>
#include <string>

static void write_str(std::ofstream& file, const std::string& s) {
    int len = (int)s.size();
    file.write((char*)&len, sizeof(int));       // (char*) cast — NOT reinterpret_cast
    if (len > 0) file.write(s.c_str(), len);    // c_str() returns const char*, accepted by write
}

static std::string read_str(std::ifstream& file) {
    int len = 0;
    file.read((char*)&len, sizeof(int));        // (char*) cast
    if (!file || len <= 0) return "";
    std::string s(len, '\0');
    file.read(&s[0], len);                      // &s[0] is char*, no cast needed
    return s;
}

void save_data(const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file) { std::cout << "Warning: could not open file for saving.\n"; return; }

    ReservationLog& log = get_reservation_log();

    // Write number of reservations
    int num_reservations = (int)log.records.size();
    file.write((char*)&num_reservations, sizeof(int));

    // Write each reservation using int-indexed loop
    for (int i = 0; i < num_reservations; i++) {
        const ReservationRecord& rec = log.records[i];
        write_str(file, rec.pnr_id);
        write_str(file, rec.request.flight_id);
        write_str(file, rec.request.passenger.first_name);
        write_str(file, rec.request.passenger.last_name);
        write_str(file, rec.request.passenger.passport_number);
        int pclass = (int)rec.request.preferred_class;
        file.write((char*)&pclass, sizeof(int));
        write_str(file, rec.request.seat_number);
        file.write((char*)&rec.total_cost.amount_cents, sizeof(long long));
        write_str(file, rec.total_cost.currency);
        int rstatus = (int)rec.status;
        file.write((char*)&rstatus, sizeof(int));
    }

    // Write flight statuses and seat counts
    const std::vector<Flight>& flights = get_flight_registry();
    int num_flights = (int)flights.size();
    file.write((char*)&num_flights, sizeof(int));
    for (int i = 0; i < num_flights; i++) {
        write_str(file, flights[i].flight_id);
        write_str(file, flights[i].status);
        file.write((char*)&flights[i].economy_seats, sizeof(int));
        file.write((char*)&flights[i].business_seats, sizeof(int));
        file.write((char*)&flights[i].first_seats, sizeof(int));
    }

    file.close();
}

void load_data(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) { return; }  // first run — no saved data, silently skip

    ReservationLog& log = get_reservation_log();

    // Read reservations
    int num_reservations = 0;
    file.read((char*)&num_reservations, sizeof(int));

    for (int i = 0; i < num_reservations; i++) {
        ReservationRecord rec;
        rec.pnr_id                              = read_str(file);
        rec.request.flight_id                   = read_str(file);
        rec.request.passenger.first_name        = read_str(file);
        rec.request.passenger.last_name         = read_str(file);
        rec.request.passenger.passport_number   = read_str(file);
        int pclass = 0;
        file.read((char*)&pclass, sizeof(int));
        rec.request.preferred_class = (SeatClass)pclass;
        rec.request.seat_number = read_str(file);
        file.read((char*)&rec.total_cost.amount_cents, sizeof(long long));
        rec.total_cost.currency = read_str(file);
        int rstatus = 0;
        file.read((char*)&rstatus, sizeof(int));
        rec.status = (ReservationStatus)rstatus;

        // Push directly into the log (do not use book_flight — inventory already updated below)
        log.records.push_back(rec);
        log.total_revenue += rec.total_cost.amount_cents;
        if (rec.status == ReservationStatus::CheckedIn) {
            log.total_checkins++;
        }
    }

    // Read and restore flight statuses + seat counts
    int num_flights = 0;
    file.read((char*)&num_flights, sizeof(int));
    for (int i = 0; i < num_flights; i++) {
        std::string fid     = read_str(file);
        std::string fstatus = read_str(file);
        int eco = 0, bus = 0, fst = 0;
        file.read((char*)&eco, sizeof(int));
        file.read((char*)&bus, sizeof(int));
        file.read((char*)&fst, sizeof(int));
        Flight* f = find_flight(fid);
        if (f != nullptr) {
            f->status          = fstatus;
            f->economy_seats   = eco;
            f->business_seats  = bus;
            f->first_seats     = fst;
        }
    }

    file.close();
}
