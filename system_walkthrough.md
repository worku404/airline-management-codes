# Airline Management System — Complete Technical Walkthrough

This document is the definitive reference for the Airline Management System built as a university group project. It covers every data type, every function, the class design, binary file I/O, and a full step-by-step lifecycle trace from the moment the program starts to the moment it exits. Every section is written to help team members explain the system clearly to their instructor.

---

## Team Members and Module Assignments

| Student | ID | Files Written | Primary Concepts |
|---|---|---|---|
| Yared Tsehaye | ETS1488/17 | `common_types.h/.cpp`, `boarding_controller.h/.cpp`, `revenue_service.h/.cpp` | `struct`, `enum class`, error propagation, dynamic pricing, check-in logic |
| Zeadonay Wejebu | ETS1609/17 | `airport_registry.h/.cpp`, `validator.h/.cpp` | `struct` arrays, const reference, input validation functions |
| Yeabsera Mengesha | ETS1495/17 | `flight_manager.h/.cpp` | `struct` with many fields, `vector<Flight>`, `for` loop, pointer return |
| Yonas Dereje | ETS1558/17 | `reservation_engine.h/.cpp` | **the `class`**, constructor, member functions, `vector`, `enum` |
| Worku Wondoson | ETS1459/17 | `controller.h/.cpp`, `main.cpp`, `file_manager.h/.cpp` | menu loop, command handlers, binary file I/O with `(char*)` |
| Samuel Firegedil | ETS1292/17 | `interactive_search_helper.h/.cpp` | user input functions, `while` loop, date parsing, airport selection |

---

## System Architecture: How the Modules Connect

```
main.cpp
  └── controller.h/.cpp          ← orchestrates everything
        ├── flight_manager        ← stores all flights in a vector
        ├── reservation_engine    ← the class; stores all bookings
        ├── boarding_controller   ← handles check-in, issues boarding passes
        ├── revenue_service       ← calculates dynamic ticket price
        ├── validator             ← validates all user input
        ├── interactive_search    ← guides user through airport/date selection
        └── file_manager          ← saves and loads binary data file
```

The project is split into **11 active modules** (header + source pairs). Each module has one clear responsibility. All modules share the types defined in `common_types.h`.

---

## C++ Concepts Demonstrated — Quick Reference for the Teacher

| Concept | Where It Appears |
|---|---|
| `struct` | `Status`, `Money`, `Flight`, `AirportInfo`, `Passenger`, `BookingRequest`, `BookingResult`, `ReservationRecord`, `BoardingPass`, `CheckInResult`, `SearchCriteria` |
| `enum class` | `SeatClass` (Economy/Business/First), `ReservationStatus` (Reserved/CheckedIn) |
| `class` | `ReservationLog` in `reservation_engine.h/.cpp` |
| Constructor | `ReservationLog()` initializes `total_revenue=0`, `total_checkins=0` |
| Member variables | `records`, `total_revenue`, `total_checkins` in `ReservationLog` |
| Member functions | `add()`, `find()`, `check_in()`, `count()` in `ReservationLog` |
| `vector` | `vector<Flight>` in flight_manager, `vector<ReservationRecord>` in ReservationLog |
| `for` loop | Every module — all use `int i`-indexed loops |
| `while` loop | Every input prompt loops until valid input is received |
| `if / else if / else` | Seat class selection, status checking, validation logic |
| Functions | Every module defines free functions with clear single responsibilities |
| Pointers | `find_flight()` returns `Flight*`, `find()` returns `ReservationRecord*` |
| File I/O (`ofstream`) | `save_data()` in `file_manager.cpp` |
| File I/O (`ifstream`) | `load_data()` in `file_manager.cpp` |
| Binary read/write | `file.write((char*)&value, sizeof(value))` — `(char*)` cast used throughout |
| String serialization | `write_str()` / `read_str()` in `file_manager.cpp` |
| `static` functions | File-scope helpers (e.g. `generate_pnr`, `gate_for`, `is_upper_alnum`) |
| `inline` function | `to_upper()` in `common_types.h` |
| `const` reference | `const std::vector<Flight>&` return from `get_flight_registry()` |
| Passing by reference | All functions that return results through output parameters |

---

## Part 1: Complete Data Type Dictionary

Every custom data type used in the project is described here — what it is, what it stores, and how to explain it.

---

### 1. `Status` — `common_types.h` — Written by Yared Tsehaye (ETS1488/17)

```cpp
struct Status {
    bool        success;
    std::string message;
};
```

**What it is:** A return type used by nearly every function in the system instead of throwing exceptions. If a function succeeds, `success` is `true` and `message` is empty. If it fails, `success` is `false` and `message` explains why.

**Why we use it:** C++ exceptions (`try/catch`) are expensive and complex. Using a `Status` struct is simpler: the caller just checks `if (!result.success)` and handles the error message.

**Fields:**
- `success` (`bool`): `true` = the operation worked, `false` = something went wrong
- `message` (`std::string`): empty on success; human-readable error description on failure (e.g. `"Flight not found"`, `"No economy seats available"`)

**Example values:**
- `{true, ""}` — success
- `{false, "PNR not found"}` — failure

---

### 2. `Money` — `common_types.h` — Written by Yared Tsehaye (ETS1488/17)

```cpp
struct Money {
    long long   amount_cents;
    std::string currency;
};
```

**What it is:** Stores a monetary amount as an integer number of cents rather than a floating-point number.

**Why we use it:** Floating-point types like `double` cannot represent all decimal values exactly (e.g. `0.1 + 0.2 ≠ 0.3` in IEEE-754). By storing `45000` instead of `450.00`, all arithmetic is exact.

**Fields:**
- `amount_cents` (`long long`): the price multiplied by 100 (e.g. `45000` = $450.00, `75000` = $750.00)
- `currency` (`std::string`): ISO currency code (always `"USD"` in this system)

**How to display it:** Divide by 100.0 and format with 2 decimal places — see `format_money()` in `controller.cpp`.

---

### 3. `SeatClass` — `common_types.h` — Written by Yared Tsehaye (ETS1488/17)

```cpp
enum class SeatClass {
    Economy,
    Business,
    First
};
```

**What it is:** A strongly-typed enumeration listing the three cabin classes on any flight.

**Why `enum class` instead of plain numbers:** Using `enum class` prevents accidental comparisons with unrelated integers and makes code self-documenting — `SeatClass::Economy` is more readable than `0`.

**Values:**
- `SeatClass::Economy` (underlying integer value 0)
- `SeatClass::Business` (underlying integer value 1)
- `SeatClass::First` (underlying integer value 2)

**Used in:** `BookingRequest.preferred_class`, `ReservationRecord.request.preferred_class`, `decrement_seat()`, `assign_seat()`, `boarding_group_for()`, `calculate_dynamic_price()`

---

### 4. `AirportInfo` — `airport_registry.h` — Written by Zeadonay Wejebu (ETS1609/17)

```cpp
struct AirportInfo {
    std::string iata_code;
    std::string city_name;
    std::string airport_name;
};
```

**What it is:** Stores identifying information for one physical airport.

**Fields:**
- `iata_code`: the internationally recognised 3-letter airport code (e.g. `"ADD"`, `"DXB"`, `"LHR"`)
- `city_name`: the city name shown to the user (e.g. `"Addis Ababa"`, `"Dubai"`, `"London"`)
- `airport_name`: the full official name (e.g. `"Bole International Airport"`, `"Dubai International Airport"`)

**Registry size:** 20 airports across Africa, Middle East, Europe, North America, and Asia-Pacific. Stored in a `static const vector<AirportInfo>` called `AIRPORTS` inside `airport_registry.cpp`.

---

### 5. `Flight` — `flight_manager.h` — Written by Yeabsera Mengesha (ETS1495/17)

```cpp
struct Flight {
    std::string flight_id;
    std::string origin_iata;
    std::string destination_iata;
    std::time_t departure_time;
    std::time_t arrival_time;
    Money       base_price;
    std::string status;
    int economy_seats;
    int business_seats;
    int first_seats;
    int economy_capacity;
    int business_capacity;
    int first_capacity;
};
```

**What it is:** The central data structure representing one scheduled commercial flight. This struct is more comprehensive than a basic struct because seat inventory is stored directly inside it — there is no separate inventory module.

**Fields explained:**
- `flight_id` (`std::string`): unique identifier, e.g. `"FL-101"`
- `origin_iata` / `destination_iata` (`std::string`): 3-letter airport codes for origin and destination
- `departure_time` / `arrival_time` (`std::time_t`): Unix timestamps — integer seconds since 1 January 1970
- `base_price` (`Money`): the starting ticket price before dynamic surcharges
- `status` (`std::string`): one of `"On Time"`, `"Delayed"`, `"Boarding"`, `"Cancelled"`
- `economy_seats` / `business_seats` / `first_seats` (`int`): remaining available seats — these decrease as bookings are made
- `economy_capacity` / `business_capacity` / `first_capacity` (`int`): total seats (fixed at creation, used to calculate demand ratio for dynamic pricing)

**Design note:** When `add_flight()` is called, it automatically sets `economy_capacity = economy_seats`, so the capacity fields are always set correctly.

**The three sample flights loaded at startup:**

| ID | Route | Depart | Base Price | Seats (E/B/F) |
|---|---|---|---|---|
| FL-101 | ADD → DXB | +1 day, 3-hour flight | $450.00 (45000 cents) | 120 / 24 / 12 |
| FL-102 | ADD → LHR | +2 days, 8-hour flight | $750.00 (75000 cents) | 120 / 24 / 12 |
| FL-201 | DXB → ADD | +3 days, 3-hour flight | $470.00 (47000 cents) | 120 / 24 / 12 |

---

### 6. `SearchCriteria` — `flight_manager.h` — Written by Yeabsera Mengesha (ETS1495/17)

```cpp
struct SearchCriteria {
    std::string origin;
    std::string destination;
    std::time_t date_window_start;
    std::time_t date_window_end;
};
```

**What it is:** A parameter bundle passed to `search_flights()`. Bundles all four filter values into one struct so the function signature stays clean.

**Fields:**
- `origin`: 3-letter IATA code of the departure airport
- `destination`: 3-letter IATA code of the arrival airport
- `date_window_start`: earliest acceptable departure time (Unix timestamp)
- `date_window_end`: latest acceptable departure time (Unix timestamp)

**Built by:** `get_interactive_search()` in `interactive_search_helper.cpp`

---

### 7. `Passenger` — `reservation_engine.h` — Written by Yonas Dereje (ETS1558/17)

```cpp
struct Passenger {
    std::string first_name;
    std::string last_name;
    std::string passport_number;
};
```

**What it is:** Holds the personal identity details of the traveler.

**Fields:**
- `first_name`, `last_name`: collected via prompts in `handle_book_command()`
- `passport_number`: validated via `is_valid_passport()` before being stored (must be 6–9 uppercase letters/digits)

---

### 8. `BookingRequest` — `reservation_engine.h` — Written by Yonas Dereje (ETS1558/17)

```cpp
struct BookingRequest {
    std::string flight_id;
    Passenger   passenger;
    SeatClass   preferred_class;
    std::string seat_number;
};
```

**What it is:** Everything the system needs to make a booking, packaged in one struct.

**Fields:**
- `flight_id`: which flight to book
- `passenger`: nested `Passenger` struct with traveler details
- `preferred_class`: which cabin class (`SeatClass::Economy`, `Business`, or `First`)
- `seat_number`: preferred seat (e.g. `"A3"`, `"12B"`) or empty string for auto-assignment

---

### 9. `BookingResult` — `reservation_engine.h` — Written by Yonas Dereje (ETS1558/17)

```cpp
struct BookingResult {
    std::string pnr_id;
    Money       total_cost;
    Status      status;
};
```

**What it is:** Everything the system returns after attempting a booking.

**Fields:**
- `pnr_id`: the generated 6-character booking reference code (e.g. `"X8A4D2"`) — empty if booking failed
- `total_cost`: the final dynamically-priced ticket price
- `status`: success or failure details

---

### 10. `ReservationStatus` — `reservation_engine.h` — Written by Yonas Dereje (ETS1558/17)

```cpp
enum class ReservationStatus {
    Reserved,
    CheckedIn
};
```

**What it is:** Tracks the lifecycle stage of a booking.

**Values:**
- `ReservationStatus::Reserved` (value 0): booking confirmed, passenger not yet checked in
- `ReservationStatus::CheckedIn` (value 1): passenger has checked in and received a boarding pass

**Transitions:** A reservation starts as `Reserved` when `book_flight()` succeeds. It moves to `CheckedIn` when `check_in_passenger()` succeeds. A reservation can only move forward — you cannot undo a check-in.

---

### 11. `ReservationRecord` — `reservation_engine.h` — Written by Yonas Dereje (ETS1558/17)

```cpp
struct ReservationRecord {
    std::string       pnr_id;
    BookingRequest    request;
    Money             total_cost;
    ReservationStatus status;
};
```

**What it is:** One complete booking stored inside the `ReservationLog` class. Contains all information needed to look up, display, or modify a reservation.

**Fields:**
- `pnr_id`: unique 6-character identifier
- `request`: the original `BookingRequest` including the assigned seat number
- `total_cost`: what was charged for this booking
- `status`: current lifecycle state (`Reserved` or `CheckedIn`)

---

### 12. `ReservationLog` (THE CLASS) — `reservation_engine.h/.cpp` — Written by Yonas Dereje (ETS1558/17)

```cpp
class ReservationLog {
public:
    std::vector<ReservationRecord> records;
    long long                      total_revenue;
    int                            total_checkins;

    ReservationLog();
    void               add(const ReservationRecord& record);
    ReservationRecord* find(const std::string& pnr);
    Status             check_in(const std::string& pnr);
    int                count();
};
```

**What it is:** The core class of the entire system. It groups all booking-related state — the list of bookings, the running revenue total, and the check-in counter — into one object.

**Why a class:** All three pieces of data (records, total_revenue, total_checkins) are always updated together. The class enforces this: when you call `add()`, the revenue is automatically updated. When you call `check_in()`, the counter is automatically incremented. This prevents forgetting to update one of the values.

**Member variables (all public):**
- `records` (`vector<ReservationRecord>`): stores every booking ever made
- `total_revenue` (`long long`): running sum of all booking costs in cents
- `total_checkins` (`int`): number of passengers who have checked in

**Constructor:**
```cpp
ReservationLog() {
    total_revenue  = 0;
    total_checkins = 0;
}
```
Called automatically when the global `static ReservationLog g_log` is created at program start. Sets numeric members to zero (the vector initialises itself).

**Member functions** (see Part 2 for full details):
- `add(record)`: adds a booking and updates `total_revenue`
- `find(pnr)`: searches `records` vector by PNR, returns pointer or `nullptr`
- `check_in(pnr)`: marks a reservation as `CheckedIn` and increments `total_checkins`
- `count()`: returns number of bookings

**Global instance:** `static ReservationLog g_log;` at file scope in `reservation_engine.cpp`. The `static` keyword means there is exactly one instance for the whole program, and it is only visible inside `reservation_engine.cpp`. External code accesses it through the global API functions.

---

### 13. `BoardingPass` — `boarding_controller.h` — Written by Yared Tsehaye (ETS1488/17)

```cpp
struct BoardingPass {
    std::string pnr_id;
    std::string gate;
    int         boarding_group;
};
```

**What it is:** The ticket issued to a passenger at check-in.

**Fields:**
- `pnr_id`: links the boarding pass back to the original booking
- `gate`: boarding gate identifier, derived from the flight ID (e.g. flight `"FL-101"` → gate `"G1"`)
- `boarding_group`: priority boarding order — First class = 1, Business = 2, Economy = 3

---

### 14. `CheckInResult` — `boarding_controller.h` — Written by Yared Tsehaye (ETS1488/17)

```cpp
struct CheckInResult {
    BoardingPass pass;
    Status       status;
    int          baggage_count;
};
```

**What it is:** Everything returned by `check_in_passenger()`.

**Fields:**
- `pass`: the generated boarding pass (only valid if `status.success == true`)
- `status`: success or failure details
- `baggage_count`: echoes the baggage count entered by the user (used in the confirmation printout)

---

## Part 2: Complete Function Registry

Every function in every module is documented here — what it takes, what it returns, and exactly what it does step by step.

---

### Module 1 — `common_types.cpp` — Yared Tsehaye (ETS1488/17)

#### `make_success()`
- **Declaration:** `Status make_success();`
- **Takes:** nothing
- **Returns:** `Status` with `success = true`, `message = ""`
- **Logic:** Creates a Status struct, sets both fields, returns it. Used at the end of every function that succeeds.

#### `make_failure(message)`
- **Declaration:** `Status make_failure(const std::string& message);`
- **Takes:** `message` — a plain-English explanation of what went wrong
- **Returns:** `Status` with `success = false`, `message = message`
- **Logic:** Creates a Status struct, sets `success = false` and copies the message, returns it. Every error path in the system ends with `return make_failure("...")`.

#### `parse_seat_class(input, out)`
- **Declaration:** `Status parse_seat_class(const std::string& input, SeatClass& out);`
- **Takes:** `input` — a string the user typed (e.g. `"economy"`, `"Business"`, `"FIRST"`); `out` — a reference that receives the result
- **Returns:** `Status`
- **Logic:**
  1. Converts `input` to uppercase using `to_upper()`
  2. Compares against `"ECONOMY"`, `"BUSINESS"`, `"FIRST"` in sequence
  3. If a match is found, sets `out` to the corresponding enum value and returns `make_success()`
  4. If no match, returns `make_failure("Unknown seat class. Use: Economy, Business, or First")`

#### `seat_class_to_string(seat_class)`
- **Declaration:** `std::string seat_class_to_string(SeatClass seat_class);`
- **Takes:** a `SeatClass` enum value
- **Returns:** `"Economy"`, `"Business"`, `"First"`, or `"Unknown"` as a `std::string`
- **Logic:** Uses an `if / if / if` chain to match the enum value and return the corresponding string. Used when displaying bookings and confirmations.

#### `to_upper(value)` — inline in header
- **Declaration:** `inline std::string to_upper(std::string value);`
- **Takes:** a string passed by value (a copy is made automatically)
- **Returns:** the same string with every character converted to uppercase
- **Logic:** Loops through each character with `int i`-indexed loop. Applies `(char)toupper((unsigned char)value[i])` to each position. The `(unsigned char)` cast avoids undefined behaviour on non-ASCII characters.

---

### Module 2 — `airport_registry.cpp` — Zeadonay Wejebu (ETS1609/17)

#### `list_all_airports()`
- **Declaration:** `const std::vector<AirportInfo>& list_all_airports();`
- **Takes:** nothing
- **Returns:** a `const` reference to the global static `AIRPORTS` vector — the caller can read it but not modify it
- **Logic:** Simply returns a reference to the static variable `AIRPORTS`. Because it returns a reference (not a copy), no data is duplicated in memory.

#### `get_airport_display_string(airport)`
- **Declaration:** `std::string get_airport_display_string(const AirportInfo& airport);`
- **Takes:** a `const AirportInfo&` reference
- **Returns:** a formatted single-line string: `"Addis Ababa (ADD) - Bole International Airport"`
- **Logic:** Concatenates `city_name + " (" + iata_code + ") - " + airport_name`. Used to display airport choices to the user.

---

### Module 3 — `flight_manager.cpp` — Yeabsera Mengesha (ETS1495/17)

**Global state:** `static std::vector<Flight> g_flights;` — stores all flights for the program's lifetime. The `static` keyword restricts visibility to this file.

#### `add_flight(flight)`
- **Declaration:** `Status add_flight(const Flight& flight);`
- **Takes:** a `const Flight&` reference (read-only, no copy made)
- **Returns:** `Status`
- **Logic:**
  1. Checks `flight.flight_id.empty()` — returns failure if empty
  2. Checks `flight.arrival_time <= flight.departure_time` — returns failure if arrival is not after departure
  3. Calls `find_flight(flight.flight_id)` — returns failure if a flight with that ID already exists
  4. Makes a copy of the flight: `Flight f = flight`
  5. If `f.status` is empty, sets it to `"On Time"`
  6. **Sets capacity fields:** `f.economy_capacity = f.economy_seats` (and same for business and first) — this records the initial seat counts permanently
  7. Pushes the copy onto `g_flights`
  8. Returns `make_success()`

#### `search_flights(criteria)`
- **Declaration:** `std::vector<Flight> search_flights(const SearchCriteria& criteria);`
- **Takes:** `const SearchCriteria&`
- **Returns:** `vector<Flight>` containing all flights that match all four criteria
- **Logic:**
  1. If `date_window_start > date_window_end`, returns empty vector immediately
  2. Int-indexed loop over every flight in `g_flights`:
     - Skips if `origin_iata` doesn't match (when criteria origin is non-empty)
     - Skips if `destination_iata` doesn't match
     - Skips if `departure_time` is outside the date window
     - If all checks pass, adds a copy to `results`
  3. Returns `results`

#### `find_flight(flight_id)`
- **Declaration:** `Flight* find_flight(const std::string& flight_id);`
- **Takes:** flight ID string
- **Returns:** a **mutable** pointer to the matching `Flight` in `g_flights`, or `nullptr` if not found
- **Logic:** Int-indexed loop; returns `&g_flights[i]` when `flight_id` matches. Returning a mutable pointer allows callers to modify the flight in-place (e.g. `f->status = "Delayed"` in `set_flight_status`).

#### `set_flight_status(flight_id, new_status)`
- **Declaration:** `Status set_flight_status(const std::string& flight_id, const std::string& new_status);`
- **Takes:** flight ID and the new status string (e.g. `"Delayed"`)
- **Returns:** `Status`
- **Logic:**
  1. Calls `find_flight(flight_id)` — returns failure if `nullptr`
  2. Sets `f->status = new_status` directly through the pointer
  3. Returns `make_success()`

#### `decrement_seat(flight_id, seat_class)`
- **Declaration:** `Status decrement_seat(const std::string& flight_id, SeatClass seat_class);`
- **Takes:** flight ID and the cabin class to decrement
- **Returns:** `Status`
- **Logic:**
  1. Calls `find_flight(flight_id)` — returns failure if `nullptr`
  2. `if (seat_class == SeatClass::Economy)`:
     - If `f->economy_seats <= 0`: returns failure `"No economy seats available"`
     - Otherwise: `f->economy_seats--`
  3. `else if (seat_class == SeatClass::Business)`:
     - Same check and decrement for `business_seats`
  4. `else` (First class):
     - Same check and decrement for `first_seats`
  5. Returns `make_success()`

#### `get_flight_registry()`
- **Declaration:** `const std::vector<Flight>& get_flight_registry();`
- **Returns:** a `const` reference to `g_flights` — used to list all flights

#### `count_flights_with_status(status)`
- **Declaration:** `int count_flights_with_status(const std::string& status);`
- **Takes:** a status string
- **Returns:** `int` — how many flights currently have that status
- **Logic:** Int-indexed loop counting matches. Used by `print_report_summary()` to show delayed flights.

---

### Module 4 — `reservation_engine.cpp` — Yonas Dereje (ETS1558/17)

#### `ReservationLog::add(record)`
- **Takes:** `const ReservationRecord&`
- **Logic:** Calls `records.push_back(record)` to append the booking, then adds `record.total_cost.amount_cents` to `total_revenue`. Both operations always happen together, ensuring the total is always accurate.

#### `ReservationLog::find(pnr)`
- **Takes:** PNR string
- **Returns:** `ReservationRecord*` or `nullptr`
- **Logic:** Int-indexed loop over `records`; returns `&records[i]` when `pnr_id` matches. Returns `nullptr` if not found. The returned pointer points directly into the `records` vector, so changes made through it are visible to all code that holds a reference to the same `ReservationLog`.

#### `ReservationLog::check_in(pnr)`
- **Takes:** PNR string
- **Returns:** `Status`
- **Logic:**
  1. Calls `find(pnr)` — returns `make_failure("Reservation not found")` if `nullptr`
  2. Checks `rec->status != ReservationStatus::Reserved` — returns `make_failure("Passenger is already checked in")` if already done
  3. Sets `rec->status = ReservationStatus::CheckedIn`
  4. Increments `total_checkins++`
  5. Returns `make_success()`

#### `ReservationLog::count()`
- **Returns:** `(int)records.size()` — total number of bookings

#### `generate_pnr()` — static helper
- **Logic:** Uses an alphabet string of 26 uppercase letters plus 10 digits. Loops 6 times, each time appending `alphabet[rand() % 36]` to build a random 6-character string. `rand()` is seeded in `initialize_system()` with `srand(time(nullptr))`.

#### `assign_seat(flight_id, seat_class)` — static helper
- **Logic:**
  1. Determines the prefix: `"E"` for Economy, `"B"` for Business, `"F"` for First
  2. Int-indexed loop over `g_log.records`: counts how many existing bookings are for the same `flight_id` AND same `seat_class`
  3. Returns `prefix + std::to_string(count + 1)` — so the first Economy seat on FL-101 is `"E1"`, second is `"E2"`, etc.

#### `book_flight(request)`
- **Declaration:** `BookingResult book_flight(const BookingRequest& request);`
- **Takes:** `const BookingRequest&`
- **Returns:** `BookingResult`
- **Logic (check-first design — no rollback needed):**
  1. **Find flight:** `find_flight(request.flight_id)` — returns `{"", {0,"USD"}, make_failure(...)}` if not found
  2. **Check seats:** Reads `flight->economy_seats` (or business/first) and the matching capacity. If `remaining <= 0`, returns failure
  3. **Price:** Calls `calculate_dynamic_price(flight->base_price, remaining, capacity, pricing_status)`. If pricing fails, returns failure
  4. **Generate PNR:** Calls `generate_pnr()`. Checks for collision with `g_log.find(pnr)`. Retries up to 10 times if collision found
  5. **Assign seat:** If `request.seat_number` is empty, calls `assign_seat()` to auto-generate (e.g. `"E1"`)
  6. **Decrement inventory:** Calls `decrement_seat()`. Since availability was checked in step 2, this rarely fails. If it does, returns failure
  7. **Store record:** Creates a `ReservationRecord`, calls `g_log.add(record)` which stores it AND updates revenue atomically
  8. **Return:** `{pnr, total_cost, make_success()}`

#### `find_reservation(pnr_id)`
- **Returns:** `const ReservationRecord*` via `g_log.find(pnr_id)`. Const because external callers should not directly modify records.

#### `mark_checked_in(pnr_id)`
- **Returns:** delegates to `g_log.check_in(pnr_id)` and returns its `Status`.

#### `get_total_reservations()`, `get_recorded_revenue()`, `get_total_checkins()`
- Simple accessors: return `g_log.count()`, `g_log.total_revenue`, `g_log.total_checkins`

#### `get_reservation_log()`
- **Returns:** `ReservationLog&` (mutable reference to `g_log`) — used only by `file_manager.cpp` to directly populate records during loading without going through `book_flight()`.

---

### Module 5 — `boarding_controller.cpp` — Yared Tsehaye (ETS1488/17)

#### `gate_for(flight_id)` — static helper
- **Logic:** Gets the last character of the flight ID string. If it is a digit (`'0'`–`'9'`), returns `"G"` + that digit (e.g. `"FL-101"` → last char is `'1'` → `"G1"`). Otherwise returns `"G1"` as default. If the flight ID is empty, returns `"G0"`.

#### `boarding_group_for(sc)` — static helper
- **Logic:** `if (sc == SeatClass::First) return 1;` then Business returns 2, Economy returns 3. First class boards first (smallest group number), economy last.

#### `check_in_passenger(pnr_id, baggage_count)`
- **Declaration:** `CheckInResult check_in_passenger(const std::string& pnr_id, int baggage_count);`
- **Takes:** PNR string (already validated by controller) and bag count (already validated)
- **Returns:** `CheckInResult`
- **Logic:**
  1. Calls `find_reservation(pnr_id)` — if `nullptr`, returns `{BoardingPass{}, make_failure("PNR not found"), baggage_count}`
  2. Checks `rec->status != ReservationStatus::Reserved` — if already checked in, returns failure
  3. Builds `BoardingPass pass`: sets `pass.pnr_id = pnr_id`, `pass.gate = gate_for(...)`, `pass.boarding_group = boarding_group_for(...)`
  4. Calls `mark_checked_in(pnr_id)` (which calls `g_log.check_in()`, updates status and increments `total_checkins`)
  5. Returns `{pass, make_success(), baggage_count}`

---

### Module 6 — `revenue_service.cpp` — Yared Tsehaye (ETS1488/17)

#### `calculate_dynamic_price(base_price, remaining_seats, total_capacity, status)`
- **Declaration:** `Money calculate_dynamic_price(const Money& base_price, int remaining_seats, int total_capacity, Status& status);`
- **Takes:** base price, remaining seats, total capacity, and a `Status&` output parameter
- **Returns:** `Money` — the final price after applying any demand surcharge
- **Logic:**
  1. Guard: if `total_capacity <= 0` → set `status = make_failure(...)`, return `base_price` unchanged
  2. Guard: if `remaining_seats <= 0` → same
  3. Guard: if `base_price.amount_cents > 1000000000` → overflow protection
  4. Calculate demand ratio: `double ratio = (double)remaining_seats / (double)total_capacity`
  5. Determine multiplier:
     - `ratio <= 0.10` (10% or fewer seats left) → `numerator=3, denominator=2` → **1.5× price**
     - `ratio <= 0.25` (25% or fewer seats left) → `numerator=6, denominator=5` → **1.2× price**
     - Otherwise → `numerator=1, denominator=1` → **base price**
  6. `result.amount_cents = (base_price.amount_cents * numerator) / denominator`
  7. Set `status = make_success()`, return `result`

**Example:** FL-101, Economy, 120 seats remaining out of 120 capacity → ratio = 1.0 → multiplier 1/1 → price stays at $450.00. If 10 seats were left (ratio = 0.083 ≤ 0.10) → price becomes $450.00 × 1.5 = $675.00.

---

### Module 7 — `validator.cpp` — Zeadonay Wejebu (ETS1609/17)

#### `is_upper_alnum(ch)` — static helper
- **Logic:** `return (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')` — checks if a character is an uppercase letter or digit.

#### `is_letter_then_digits(s)` — static helper
- **Logic:** First character must be a letter (`isalpha`); all remaining characters must be digits (`isdigit`). Used by `is_valid_seat_number`.

#### `is_digits_then_letter(s)` — static helper
- **Logic:** All characters except the last must be digits; the last character must be a letter. Used by `is_valid_seat_number`.

#### `is_valid_passport(passport_number)`
- **Logic:** Length must be 6–9 characters. Every character must pass `is_upper_alnum()`. Returns `false` on first failure, `true` at end.
- **Examples:** `"EP123456"` → valid (length 8). `"ep123456"` → invalid (lowercase). `"12345"` → invalid (too short).

#### `is_valid_pnr(pnr_id)`
- **Logic:** Length must be exactly 6. Every character must pass `is_upper_alnum()`.
- **Examples:** `"AB1234"` → valid. `"abc123"` → invalid. `"AB12345"` → invalid (too long).

#### `is_valid_seat_number(seat_number)`
- **Logic:** Length must be 2–5 characters. Must match either `is_letter_then_digits` (e.g. `"A1"`, `"B12"`) or `is_digits_then_letter` (e.g. `"12A"`, `"1E"`).
- **Examples:** `"E1"` → valid. `"12A"` → valid. `"EEE"` → invalid (no digits). `"123"` → invalid (no letter).

#### `is_valid_baggage_count(baggage_count)`
- **Logic:** `return baggage_count >= 0 && baggage_count <= 5;` — simple range check.

#### `try_parse_int(value, out)`
- **Logic:**
  1. Empty string → `return false`
  2. Int-indexed loop: if any character is not a digit (`isdigit`) → `return false`
  3. Accumulate: `val = val * 10 + (value[i] - '0')`
  4. If `val > 1000000` → `return false` (overflow guard)
  5. Set `out = val`, return `true`
- **Why not use `stoi()`?** `stoi()` throws an exception on invalid input. This function never throws — it just returns `false`. Used throughout the controller for parsing menu choices.

---

### Module 8 — `interactive_search_helper.cpp` — Samuel Firegedil (ETS1292/17)

**Internal constant:** `static const int SECONDS_PER_DAY = 24 * 60 * 60` (86400)

#### `get_user_input()` — static helper
- **Logic:** Prints `"> "`, calls `std::getline(std::cin, input)`, then manually trims whitespace using two `while` loops advancing `start` and `end` indices. Returns the trimmed substring.

#### `parse_date(date_str)` — static helper
- **Logic:** Uses `std::istringstream` to parse `YYYY-MM-DD` format. Reads year, dash, month, dash, day. Returns `-1` if the separator characters are not `'-'`, if month is outside 1–12, day outside 1–31, or year before 2020. Constructs a `std::tm` struct and calls `std::mktime()` to get a `std::time_t` timestamp.

#### `is_date_today_or_future(timestamp)` — static helper
- **Logic:** Gets the current time via `std::time(nullptr)`. Converts to a `std::tm` struct. Zeroes out the hours, minutes, seconds to get midnight today. Converts back to `time_t` with `mktime()`. Returns `timestamp >= today_midnight`.

#### `format_date(timestamp)` — static helper
- **Logic:** Calls `std::localtime()` to convert the timestamp, then `std::strftime()` with `"%Y-%m-%d"` format into a `char[11]` buffer. Returns as `std::string`.

#### `get_airport_from_user(prompt_text)`
- **Logic:**
  1. Prints the prompt
  2. Calls `list_all_airports()` to get the registry reference
  3. Int-indexed loop prints every airport as a numbered entry (no pagination)
  4. `while(true)` loop: calls `get_user_input()`, calls `try_parse_int()`, validates range `[1, airports.size()]`
  5. Returns the IATA code of the selected airport

#### `get_date_from_user(prompt_text)`
- **Logic:** `while(true)` loop. Calls `get_user_input()`. Calls `parse_date()`. Calls `is_date_today_or_future()`. Loops until all validations pass. Returns the `std::time_t` timestamp.

#### `get_search_range_days()`
- **Logic:** `while(true)` loop. Calls `get_user_input()`. Calls `try_parse_int()`. Validates `days >= 1 && days <= 365`. Returns valid integer.

#### `get_interactive_search()`
- **Logic:**
  1. Prints the "Flight Search Setup" header
  2. Calls `get_airport_from_user("Select departure airport:")` → stores in `origin`
  3. Calls `get_airport_from_user("Select arrival airport:")` in a `while(true)` loop; breaks only when `destination != origin` (prevents same-origin-and-destination)
  4. Calls `get_date_from_user()` → `departure_date` timestamp
  5. Calls `get_search_range_days()` → `search_days`
  6. Computes `end_date = departure_date + (search_days - 1) * SECONDS_PER_DAY + (SECONDS_PER_DAY - 1)` so the last day is fully included
  7. Builds and returns a `SearchCriteria` struct

---

### Module 9 — `file_manager.cpp` — Worku Wondoson (ETS1459/17)

**Key design:** Uses `(char*)` cast for all binary reads and writes. This is the C-style equivalent of `reinterpret_cast<char*>`, used here for simplicity. Binary format means the file is not human-readable text — data is stored as raw bytes.

#### `write_str(file, s)` — static helper
```cpp
int len = (int)s.size();
file.write((char*)&len, sizeof(int));    // write the length as 4 bytes
if (len > 0) file.write(s.c_str(), len); // write the characters
```
- Writes a string as: 4-byte integer (length) followed by the character bytes. The length is written first so `read_str` knows how many bytes to read back.

#### `read_str(file)` — static helper
```cpp
int len = 0;
file.read((char*)&len, sizeof(int));     // read the 4-byte length
std::string s(len, '\0');
file.read(&s[0], len);                   // read len bytes into the string buffer
return s;
```
- Reads a string that was written by `write_str`. `&s[0]` is already a `char*` pointer so no cast is needed.

#### `save_data(filepath)`
- **Logic:**
  1. Opens a binary `ofstream` at `filepath` — returns with a warning if the file cannot be opened
  2. Gets `ReservationLog& log = get_reservation_log()`
  3. Writes `int num_reservations = log.records.size()` using `(char*)` cast
  4. Int-indexed loop over all records: for each, writes all string fields via `write_str()` and all numeric fields via `(char*)` cast
  5. Gets `const vector<Flight>& flights = get_flight_registry()`
  6. Writes `int num_flights`
  7. Int-indexed loop over all flights: writes `flight_id`, `status` via `write_str()`, and `economy_seats`, `business_seats`, `first_seats` via `(char*)` cast
  8. Closes the file

#### `load_data(filepath)`
- **Logic:**
  1. Opens a binary `ifstream` — silently returns if file does not exist (first run)
  2. Reads `int num_reservations`
  3. Int-indexed loop: for each reservation, reads all fields in the same order as `save_data` wrote them. Casts integers back: `(SeatClass)pclass` and `(ReservationStatus)rstatus`
  4. Pushes each record directly into `log.records` (bypassing `book_flight()` to avoid re-decrementing seat counts), manually accumulates `total_revenue`, increments `total_checkins` for each `CheckedIn` record
  5. Reads `int num_flights`, then for each: calls `find_flight(fid)` and directly sets the status and seat counts

**Binary file format (`airline_data.bin`):**
```
[4 bytes: number of reservations]
For each reservation:
  [4 bytes: PNR length] [N bytes: PNR characters]
  [4 bytes: flight_id length] [N bytes: flight_id characters]
  [4 bytes: first_name length] [N bytes: first_name]
  [4 bytes: last_name length]  [N bytes: last_name]
  [4 bytes: passport length]   [N bytes: passport]
  [4 bytes: seat class integer (0/1/2)]
  [4 bytes: seat_number length] [N bytes: seat_number]
  [8 bytes: cost in cents (long long)]
  [4 bytes: currency length] [N bytes: currency]
  [4 bytes: reservation status integer (0/1)]
[4 bytes: number of flights]
For each flight:
  [4 bytes: flight_id length] [N bytes: flight_id]
  [4 bytes: status length]    [N bytes: status]
  [4 bytes: economy_seats]
  [4 bytes: business_seats]
  [4 bytes: first_seats]
```

---

### Module 10 — `controller.cpp` — Worku Wondoson (ETS1459/17)

**Global state:**
- `static const int SECONDS_PER_HOUR = 3600;`
- `static const int SECONDS_PER_DAY = 24 * SECONDS_PER_HOUR;`
- `static const std::string FLIGHT_STATUSES[] = {"On Time","Delayed","Boarding","Cancelled"};`
- `static const int NUM_STATUSES = 4;`
- `static std::vector<Flight> g_search_results;` — caches the last search result for use by the book command

**All helper functions** in `controller.cpp` are declared `static` (file-scope only, not visible outside):

| Function | Purpose |
|---|---|
| `print_divider()` | Prints a line of 62 `=` characters |
| `print_section_title(title)` | Prints `\n=====  title  =====\n` |
| `print_hint(text)` | Prints `Hint: text` |
| `format_money(amount)` | Returns `"450.00 USD"` from a `Money` struct |
| `format_date_time(ts)` | Returns `"2026-05-23 11:00"` from a `time_t` |
| `print_flight_row(i, f)` | Numbered row showing flight info and seat counts |
| `print_status(s, ctx)` | Prints `OK: ctx` or `Error [ctx]: message` |
| `trim_copy(value)` | Trims whitespace from both ends of a string |
| `prompt_line(label)` | Prints label + `"\n> "`, reads line, returns trimmed |
| `prompt_non_empty(label)` | Loops until user types something non-empty |
| `now_plus_days(n)` | Returns `time(nullptr) + n * SECONDS_PER_DAY` |
| `prompt_flight_from_search()` | Shows search results or all flights, returns flight ID |
| `prompt_flight_from_registry()` | Shows all flights, returns flight ID |
| `prompt_seat_class()` | Shows 1/2/3 options, returns SeatClass |
| `prompt_passport()` | Loops until `is_valid_passport()` passes |
| `prompt_seat_number()` | Loops until valid format or empty (auto-assign) |
| `prompt_pnr()` | Strips optional `"PNR-"` prefix, validates 6-char format |
| `prompt_baggage_count()` | Loops until 0–5 or empty (defaults to 0) |
| `prompt_flight_status()` | Shows 4 status options, returns canonical string |
| `print_search_results(flights)` | Displays all flights with seat counts |
| `print_booking_confirmation(result)` | Displays PNR, passenger, flight, seat, total |
| `print_checkin_confirmation(result)` | Displays boarding pass details |
| `print_status_confirmation(flight)` | Displays updated flight status |
| `handle_search_command()` | Orchestrates option 1 |
| `handle_book_command()` | Orchestrates option 2 |
| `handle_checkin_command()` | Orchestrates option 3 |
| `handle_status_command()` | Orchestrates option 4 |
| `print_report_summary()` | Orchestrates option 5 |
| `print_help()` | Prints the 7-item menu |

#### `initialize_system()` — public
- Calls `srand((unsigned int)time(nullptr))` to seed the random number generator for PNR generation
- Creates three `Flight` structs and calls `add_flight()` for each:
  - `FL-101`: ADD → DXB, +1 day, 3 hours, $450.00, 120/24/12 seats
  - `FL-102`: ADD → LHR, +2 days, 8 hours, $750.00, 120/24/12 seats
  - `FL-201`: DXB → ADD, +3 days, 3 hours, $470.00, 120/24/12 seats
- Calls `load_data("airline_data.bin")` to restore any saved state

#### `run_repl()` — public
- Calls `print_help()` once to show the menu at startup
- Enters `while(true)` loop:
  - Prints `"\nairline> "` prompt
  - Reads a line with `std::getline`
  - `trim_copy()` the input
  - Calls `try_parse_int()` to parse the choice
  - Routes via `if/else if` chain: 1→search, 2→book, 3→checkin, 4→status, 5→report, 6→help, 7→break
  - Invalid input → print error, loop again

---

## Part 3: The Class — ReservationLog — Detailed Explanation

This is the only class in the project and demonstrates the fundamental OOP concept of grouping related data and operations.

### How to explain it to the teacher:

> *"We defined a class called `ReservationLog` to manage all passenger bookings. Instead of having three separate global variables — a vector of records, a revenue counter, and a check-in counter — we grouped them into one class. This way, whenever we add a booking, both the vector and the revenue total are updated in the same place. The constructor initialises the counters to zero when the program starts. The `find()` method searches the vector using a for loop and returns a pointer. The `check_in()` method updates the status and increments the counter together so they cannot get out of sync."*

### The global instance:

```cpp
// In reservation_engine.cpp
static ReservationLog g_log;
```

- `static` at file scope means: created when the program starts, destroyed when it ends, and only directly accessible within `reservation_engine.cpp`
- External modules call `find_reservation()`, `mark_checked_in()`, etc. — these are wrapper functions that delegate to `g_log`'s methods
- `file_manager.cpp` gets direct access through `get_reservation_log()` which returns a reference

### Flow of a booking through the class:

```
book_flight() called
  → creates ReservationRecord
  → calls g_log.add(record)
       → records.push_back(record)        // stored
       → total_revenue += cost_cents      // revenue updated

check_in_passenger() called
  → calls mark_checked_in(pnr)
       → calls g_log.check_in(pnr)
            → calls g_log.find(pnr)       // searches vector
            → rec->status = CheckedIn    // status updated
            → total_checkins++           // counter updated
```

---

## Part 4: Binary File I/O — Detailed Explanation

### How to explain it to the teacher:

> *"When a booking is made, we call `save_data()` which opens a binary file called `airline_data.bin` and writes all the current data. Binary means the data is stored as raw bytes — not human-readable text. We use `file.write((char*)&variable, sizeof(variable))` to write integers and long longs. For strings, we write the length first as a 4-byte integer, then write the character bytes. When the program starts next time, `load_data()` reads the same file in the same order, reconstructing all the reservations and flight states exactly as they were."*

### The `(char*)` cast:

```cpp
int n = 42;
file.write((char*)&n, sizeof(int));  // writes 4 raw bytes
```

- `&n` gives the memory address of the integer
- `(char*)` tells the compiler: treat this address as a pointer to bytes, not as a pointer to an int
- `sizeof(int)` = 4 bytes on most systems
- This allows `write()` (which only accepts `const char*`) to work with any data type

### Why binary over text:

- Text would require parsing (splitting on commas, handling edge cases)
- Binary is direct: what is in memory is what goes to disk
- Loading is also simpler — read exactly `sizeof(type)` bytes into a variable

### When `save_data()` is called:

- After a successful booking: `handle_book_command()`
- After a successful check-in: `handle_checkin_command()`
- After a flight status update: `handle_status_command()`

### First run behaviour:

On the first run, `airline_data.bin` does not exist. `load_data()` tries `std::ifstream file(filepath, std::ios::binary)`. The `if (!file)` check catches this and returns immediately. No error is shown — the system just starts with empty data.

---

## Part 5: Complete End-to-End Lifecycle Walkthrough

This section traces a complete session from the moment the user runs the program to the moment it exits. Every function called is identified, every decision point is explained.

---

### Phase 1: Program Startup

#### User Perspective:
```
C:\...> airline.exe
```
The program opens, the menu is displayed:
```
==============================================================
  Airline Management System
==============================================================
    1. Search flights
    2. Book a flight
    3. Check in a passenger
    4. Update a flight status
    5. Revenue and operations summary
    6. Show this menu
    7. Exit

airline>
```

#### Developer Perspective — Call Stack:

**`main()` → `initialize_system()` → `run_repl()`**

1. **`main()` calls `initialize_system()`**

2. **Inside `initialize_system()`:**
   - `srand((unsigned int)time(nullptr))` — seeds the random number generator with the current Unix timestamp so each run produces different PNRs
   - Creates three `Flight` structs (local variables in the function):
     ```
     FL-101: ADD→DXB, now+1day departure, now+1day+3hours arrival
             base_price = {45000, "USD"}, economy_seats=120, business_seats=24, first_seats=12
     FL-102: ADD→LHR, now+2days, +8hours
             base_price = {75000, "USD"}, economy_seats=120, business_seats=24, first_seats=12
     FL-201: DXB→ADD, now+3days, +3hours
             base_price = {47000, "USD"}, economy_seats=120, business_seats=24, first_seats=12
     ```
   - Calls `add_flight(fl)` for each:
     - Validates ID is not empty, arrival after departure, no duplicate
     - Copies flight; sets `economy_capacity = economy_seats = 120`, etc.
     - Pushes onto `g_flights` vector
   - Calls `load_data("airline_data.bin")`:
     - On first run: file does not exist, `if (!file)` is true, function returns immediately
     - On subsequent runs: restores reservations, total_revenue, total_checkins, flight statuses, seat counts

3. **`main()` calls `run_repl()`**
   - `print_help()` prints the menu
   - Enters `while(true)` loop, prints `"airline> "` prompt and waits

---

### Phase 2: Option 1 — Flight Search

#### User Perspective:
```
airline> 1

==========================================
         Flight Search Setup
==========================================

? Select departure airport:

Available Airports:
  1. Addis Ababa (ADD) - Bole International Airport
  2. Cairo (CAI) - Cairo International Airport
  ... [20 airports listed]

? Select airport number:
> 1
OK: Addis Ababa (ADD) - Bole International Airport

? Select arrival airport:
  ... [20 airports listed]
? Select airport number:
> 5
OK: Dubai (DXB) - Dubai International Airport

? Departure date (YYYY-MM-DD):
> 2026-05-23
OK: 2026-05-23

? Search range in days? (1-365):
> 3
OK: 3 days

Searching: ADD -> DXB
Dates: 2026-05-23 to 2026-05-25

==============================================================
  Search Results
==============================================================
1 flight found.
  1. FL-101 | ADD -> DXB | Depart 2026-05-23 09:00 | 450.00 USD | On Time
     Seats: Economy 120, Business 24, First 12
Hint: Select option 2 from the menu to book one of these flights.
```

#### Developer Perspective — Call Stack:

1. `run_repl()` reads `"1"`, `try_parse_int("1", choice)` returns `true`, `choice = 1`
2. Calls `handle_search_command()`
3. `handle_search_command()` calls `get_interactive_search()` (in `interactive_search_helper.cpp`):
   - Calls `get_airport_from_user("Select departure airport:")`:
     - Gets `list_all_airports()` reference (20 airports)
     - Prints numbered list — no pagination
     - User enters `"1"`, `try_parse_int("1", selection)` → `selection = 1`
     - Returns `airports[0].iata_code = "ADD"`
   - Calls `get_airport_from_user("Select arrival airport:")` in loop:
     - User selects `5` → `airports[4].iata_code = "DXB"`
     - `"DXB" != "ADD"` so loop exits
   - Calls `get_date_from_user("Departure date (YYYY-MM-DD):")`:
     - `parse_date("2026-05-23")` constructs `std::tm{year=126, mon=4, mday=23}`, calls `mktime()` → timestamp
     - `is_date_today_or_future(timestamp)` returns `true`
     - Returns timestamp
   - Calls `get_search_range_days()` → user enters `"3"` → `try_parse_int` → 3 days
   - Computes `end_date = departure_date + 2 * 86400 + 86399`
   - Returns `SearchCriteria{origin="ADD", destination="DXB", start=..., end=...}`
4. `handle_search_command()` calls `search_flights(criteria)`:
   - Iterates over `g_flights` (3 flights):
     - FL-101: origin `"ADD"` ✓, destination `"DXB"` ✓, departure_time within window ✓ → added to results
     - FL-102: destination `"LHR"` ✗ → skipped
     - FL-201: origin `"DXB"` ✗ → skipped
   - Returns `vector<Flight>` with one flight: FL-101
5. Stores in `g_search_results`
6. Calls `print_search_results(results)`:
   - Calls `print_flight_row(1, flight)` for FL-101:
     - `format_money({45000,"USD"})` → `"450.00 USD"`
     - `format_date_time(departure_time)` → `"2026-05-23 09:00"`
     - Prints seat counts directly from `flight.economy_seats`, `flight.business_seats`, `flight.first_seats`

---

### Phase 3: Option 2 — Flight Booking

#### User Perspective:
```
airline> 2

==============================================================
  Choose a Flight From Your Search
==============================================================
  1. FL-101 | ADD -> DXB | Depart 2026-05-23 09:00 | 450.00 USD | On Time
     Seats: Economy 120, Business 24, First 12
Choose a flight by number:
> 1
  Cabin classes:
    1. Economy
    2. Business
    3. First
Choose a class (press Enter for Economy):
> 1
Passenger first name:
> John
Passenger last name:
> Doe
Passenger passport number (6-9 uppercase letters/digits):
> EP123456
Preferred seat (e.g. A1, 12E) — press Enter to auto-assign:
>

==============================================================
  Booking Confirmed
==============================================================
  PNR:       PNR-X8A4D2
  Passenger: John Doe
  Flight:    FL-101
  Class:     Economy
  Seat:      E1
  Total:     450.00 USD
Hint: Select option 3 to check in for this booking.
```

#### Developer Perspective — Call Stack:

1. `run_repl()` reads `"2"` → calls `handle_book_command()`
2. `handle_book_command()`:
   - Calls `prompt_flight_from_search()`:
     - `g_search_results` is not empty → displays FL-101
     - User enters `"1"` → returns `"FL-101"`
   - Calls `prompt_seat_class()`:
     - User enters `"1"` → returns `SeatClass::Economy`
   - Calls `prompt_non_empty("Passenger first name:")` → `"John"`
   - Calls `prompt_non_empty("Passenger last name:")` → `"Doe"`
   - Calls `prompt_passport()`:
     - User enters `"EP123456"`, converted to uppercase
     - `is_valid_passport("EP123456")`: length 8 ✓, all `is_upper_alnum()` ✓ → returns `true`
     - Returns `"EP123456"`
   - Calls `prompt_seat_number()`:
     - User presses Enter → empty string → returns `""` (auto-assign)
   - Builds `BookingRequest request`: flight_id=`"FL-101"`, passenger={John, Doe, EP123456}, preferred_class=Economy, seat_number=`""`
   - Calls `book_flight(request)`:

3. **Inside `book_flight(request)`:**
   - `find_flight("FL-101")` → returns pointer to FL-101 in `g_flights` (not null ✓)
   - `preferred_class == Economy` → `remaining = flight->economy_seats = 120`, `capacity = flight->economy_capacity = 120`
   - `remaining > 0` ✓
   - `calculate_dynamic_price({45000,"USD"}, 120, 120, pricing_status)`:
     - `ratio = 120/120 = 1.0` → above 0.25 threshold → `numerator=1, denominator=1`
     - `result.amount_cents = (45000 * 1) / 1 = 45000`
     - Sets `pricing_status = make_success()`
     - Returns `{45000, "USD"}`
   - `generate_pnr()` → e.g. `"X8A4D2"` (random)
   - `g_log.find("X8A4D2")` → `nullptr` (no collision) ✓
   - `seat = ""` → calls `assign_seat("FL-101", SeatClass::Economy)`:
     - prefix = `"E"`
     - Loop over `g_log.records`: empty (no bookings yet) → count = 0
     - Returns `"E" + to_string(1) = "E1"`
   - `decrement_seat("FL-101", SeatClass::Economy)`:
     - `find_flight("FL-101")` → pointer
     - `economy_seats = 120 > 0` ✓
     - `economy_seats--` → now 119
     - Returns `make_success()`
   - Builds `ReservationRecord`: pnr=`"X8A4D2"`, request.seat_number=`"E1"`, total_cost={45000,"USD"}, status=Reserved
   - `g_log.add(record)`:
     - `records.push_back(record)` → `records.size() = 1`
     - `total_revenue += 45000` → `total_revenue = 45000`
   - Returns `{"X8A4D2", {45000,"USD"}, make_success()}`

4. Back in `handle_book_command()`:
   - `result.status.success == true` ✓
   - Calls `print_booking_confirmation(result)`:
     - `find_reservation("X8A4D2")` → returns `&g_log.records[0]`
     - Prints PNR, passenger name, flight, class, seat, total
   - Calls `save_data("airline_data.bin")`:
     - Opens binary file, writes 1 reservation (all fields), writes 3 flights (statuses + seat counts with FL-101 now at 119 economy seats)

---

### Phase 4: Option 3 — Passenger Check-in

#### User Perspective:
```
airline> 3
Enter your 6-character PNR code (e.g. AB1234):
> X8A4D2
Number of checked bags (0-5, press Enter for 0):
> 2

==============================================================
  Check-in Complete
==============================================================
  PNR:            PNR-X8A4D2
  Passenger:      John Doe
  Flight:         FL-101
  Seat:           E1
  Gate:           G1
  Boarding group: 3
  Checked bags:   2
Hint: Select option 5 to view the operations summary.
```

#### Developer Perspective — Call Stack:

1. `run_repl()` reads `"3"` → calls `handle_checkin_command()`
2. `handle_checkin_command()`:
   - Calls `prompt_pnr()`:
     - User enters `"X8A4D2"`, no `"PNR-"` prefix to strip
     - `is_valid_pnr("X8A4D2")`: length 6 ✓, all uppercase alphanumeric ✓
     - Returns `"X8A4D2"`
   - Calls `prompt_baggage_count()`:
     - User enters `"2"`, `try_parse_int("2", count)` → count=2, `is_valid_baggage_count(2)` ✓
     - Returns `2`
   - Calls `check_in_passenger("X8A4D2", 2)`:

3. **Inside `check_in_passenger("X8A4D2", 2)`:**
   - `find_reservation("X8A4D2")` → `g_log.find("X8A4D2")` → loop finds records[0] → returns `&records[0]`
   - `rec->status == ReservationStatus::Reserved` ✓ (not yet checked in)
   - Builds `BoardingPass pass`:
     - `pass.pnr_id = "X8A4D2"`
     - `gate_for("FL-101")`: last char is `'1'` (digit) → returns `"G1"`
     - `boarding_group_for(SeatClass::Economy)`: returns `3`
   - Calls `mark_checked_in("X8A4D2")` → `g_log.check_in("X8A4D2")`:
     - `find("X8A4D2")` → `&records[0]`
     - Status is Reserved ✓
     - `records[0].status = ReservationStatus::CheckedIn`
     - `total_checkins++` → now `1`
     - Returns `make_success()`
   - Returns `{pass, make_success(), 2}`

4. Back in `handle_checkin_command()`:
   - `result.status.success == true` ✓
   - `print_checkin_confirmation(result)`: prints gate G1, group 3, bags 2
   - Calls `save_data("airline_data.bin")`: overwrites file; this time the reservation has status=CheckedIn

---

### Phase 5: Option 4 — Update Flight Status

#### User Perspective:
```
airline> 4

==============================================================
  All Available Flights
==============================================================
  1. FL-101 | ADD -> DXB | Depart 2026-05-23 09:00 | 450.00 USD | On Time
     Seats: Economy 119, Business 24, First 12
  2. FL-102 | ADD -> LHR | Depart 2026-05-24 09:00 | 750.00 USD | On Time
     Seats: Economy 120, Business 24, First 12
  3. FL-201 | DXB -> ADD | Depart 2026-05-25 09:00 | 470.00 USD | On Time
     Seats: Economy 120, Business 24, First 12
Choose a flight by number:
> 2
  Flight statuses:
    1. On Time
    2. Delayed
    3. Boarding
    4. Cancelled
Choose a status by number or name:
> 2

==============================================================
  Flight Status Updated
==============================================================
  Flight:     FL-102
  Route:      ADD -> LHR
  New status: Delayed
```

#### Developer Perspective — Call Stack:

1. `run_repl()` reads `"4"` → calls `handle_status_command()`
2. `handle_status_command()`:
   - Calls `prompt_flight_from_registry()`:
     - Calls `get_flight_registry()` → returns `const vector<Flight>& g_flights`
     - Prints all 3 flights with current seat counts
     - User enters `"2"` → returns `"FL-102"`
   - Calls `prompt_flight_status()`:
     - Prints 4 status options
     - User enters `"2"`, `try_parse_int("2", selection)` → `selection=2`
     - `FLIGHT_STATUSES[1] = "Delayed"` → returns `"Delayed"`
   - Calls `set_flight_status("FL-102", "Delayed")`:
     - `find_flight("FL-102")` → returns pointer to FL-102 in `g_flights`
     - `f->status = "Delayed"` (direct modification through pointer)
     - Returns `make_success()`
   - `find_flight("FL-102")` → pointer (now has status `"Delayed"`)
   - Calls `print_status_confirmation(*f)`: prints flight ID, route, new status
   - Calls `save_data("airline_data.bin")`: saves FL-102 with status `"Delayed"`

---

### Phase 6: Option 5 — Revenue and Operations Report

#### User Perspective:
```
airline> 5

==============================================================
  Revenue and Operations Report
==============================================================
  Revenue total:    450.00 USD

  Operations summary:
    Reservations:    1
    Checked-in:      1
    Delayed flights: 1
```

#### Developer Perspective — Call Stack:

1. `run_repl()` reads `"5"` → calls `print_report_summary()`
2. `print_report_summary()`:
   - `get_recorded_revenue()` → `g_log.total_revenue = 45000`
   - `format_money({45000, "USD"})` → `"450.00 USD"`
   - `get_total_reservations()` → `g_log.count()` → `records.size() = 1`
   - `get_total_checkins()` → `g_log.total_checkins = 1`
   - `count_flights_with_status("Delayed")`:
     - Int-indexed loop over `g_flights` (3 flights)
     - FL-101: `"On Time"` ≠ `"Delayed"` → skip
     - FL-102: `"Delayed"` == `"Delayed"` → count++
     - FL-201: `"On Time"` ≠ `"Delayed"` → skip
     - Returns `1`
   - Prints the formatted summary

---

### Phase 7: Option 6 — Show Menu

#### User Perspective:
```
airline> 6
```
The main menu is reprinted immediately.

#### Developer Perspective:
`run_repl()` reads `"6"` → `choice == 6` → calls `print_help()` → reprints the 7-item menu.

---

### Phase 8: Option 7 — Exit

#### User Perspective:
```
airline> 7
C:\...>
```
The program terminates and the shell prompt returns.

#### Developer Perspective — Call Stack:

1. `run_repl()` reads `"7"` → `choice == 7` → `break`
2. The `while(true)` loop in `run_repl()` exits
3. `run_repl()` returns to `main()`
4. `main()` executes `return 0;`
5. The operating system receives exit code 0 (success)
6. All static variables (`g_flights`, `g_log`, `AIRPORTS`) are automatically destroyed as the program terminates — no memory leaks

---

## Part 6: What Happens Across Multiple Runs

When the user runs the program a second time after having made bookings:

1. `initialize_system()` adds the 3 sample flights to `g_flights` (all start fresh with full seat counts and `"On Time"` status)
2. `load_data("airline_data.bin")` opens the saved file
3. Reservations are read and pushed into `g_log.records`
4. `total_revenue` is accumulated from saved records
5. `total_checkins` is counted from CheckedIn records
6. Flight statuses are restored (e.g. FL-102 becomes `"Delayed"` again)
7. Seat counts are restored (e.g. FL-101 `economy_seats` set back to 119)

The system resumes exactly where it left off.

---

## Part 7: Error Handling — How Failures Propagate

The system never uses `throw` or `catch`. Every function that can fail returns a `Status` struct. The pattern is:

```cpp
Status result = some_function(...);
if (!result.success) {
    // handle the error — print message or return early
    return {"", {0,"USD"}, result};  // pass the Status up
}
// continue with success path
```

**Example chain for a failed booking (flight not found):**
```
book_flight("FL-999", ...)
  → find_flight("FL-999") → nullptr
  → return {"", {0,"USD"}, make_failure("Flight not found")}

handle_book_command()
  → result.status.success == false
  → print_status(result.status, "Booking")
     → prints: "Error [Booking]: Flight not found"
  → returns without saving
```

No crash. No exception. The user sees a clear error message and the menu appears again.

---

## Part 8: Concepts Summary for Presentation

### How to say it to the teacher:

**On `struct`:** *"We used structs throughout to group related data together. For example, `Flight` groups all the flight details — ID, route, times, price, and seat counts — into one variable. This makes it easy to pass a complete flight to any function."*

**On `enum class`:** *"We used `enum class` for `SeatClass` and `ReservationStatus`. This prevents using arbitrary integers and makes the code self-documenting. When you see `SeatClass::Economy`, you know exactly what it means."*

**On `class`:** *"We have one class: `ReservationLog`. It holds the vector of all bookings plus the running totals. The constructor sets the totals to zero. The `add()` method updates both the vector and the revenue in one call, so they can never get out of sync."*

**On `vector`:** *"We use `vector<Flight>` to store all flights and `vector<ReservationRecord>` inside the class to store all bookings. Vectors grow automatically as we add items. We access elements with an integer index in a `for` loop."*

**On binary file I/O:** *"We save and load data using binary files. `save_data()` opens a `std::ofstream` in binary mode and writes each value with `file.write((char*)&variable, sizeof(variable))`. The `(char*)` cast tells the compiler to treat the variable's address as a byte pointer. On loading, `file.read((char*)&variable, sizeof(variable))` reads the same bytes back."*

**On functions:** *"Every module has well-named functions with a single responsibility. For example, `decrement_seat()` only decrements a seat count. `is_valid_passport()` only validates a passport. This makes each function short and easy to test and explain."*

**On `static`:** *"We use `static` on file-scope variables like `g_flights` and `g_log` to restrict their visibility to the file that owns them. We also use `static` on helper functions like `generate_pnr()` and `gate_for()` so they are private to their module."*

**On pointer returns:** *"`find_flight()` returns a `Flight*` pointer so the caller can modify the flight in place. When we call `set_flight_status()`, it gets the pointer and writes directly: `f->status = new_status`. If the flight is not found, it returns `nullptr`, which the caller must always check."*

---

*End of walkthrough. All code compiles cleanly with Visual Studio 2026 (`cl /EHsc /std:c++17`) with zero errors and zero warnings.*
