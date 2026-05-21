# Airline Management System: Exhaustive Technical & Architectural Walkthrough

This document provides a highly comprehensive, step-by-step technical and architectural walkthrough of the exception-free C++ Airline Management System. It serves as the ultimate manual for both users and developers to understand the system's runtime behavior, design decisions, data mapping, and calling sequences.

---

## Part 1: Exhaustive Dictionary of System Datatypes (Recursive Analysis)

The system relies on strong, well-defined custom structures and enums to avoid exception handling and type coercion vulnerabilities. Below is the complete dictionary of all datatypes used in the project, mapped recursively from fundamental fields up to compound structures.

### 1. Fundamental Core Types (`common_types.h`)

#### A. `Status` (Structure)
A central structure utilized for return codes and error propagation throughout the entire project. It implements an exception-free error bubble-up paradigm.
* **Fields**:
  * `success` (`bool`): Direct indicator of the function's success (`true` means successful execution, `false` indicates that an error was encountered).
  * `error_code` (`std::string`): A standardized uppercase identifier classifying the failure scenario.
    * *Examples*: `"FLIGHT_DUPLICATE"`, `"SEAT_OCCUPIED"`, `"MONEY_OVERFLOW"`, `"INVENTORY_SOLD_OUT"`.
  * `message` (`std::string`): A descriptive, developer-friendly and user-friendly explanation of why the action failed.
    * *Examples*: `"Flight ID already exists"`, `"Expected USD currency in audit"`.

#### B. `Money` (Structure)
Encapsulates financial amounts and currency codes, eliminating float/double rounding inaccuracies (e.g., standard IEEE-754 issues) by representing monetary amounts entirely in integer cents.
* **Fields**:
  * `amount_cents` (`long long`): The integer cent representation (e.g., `45000` is `$450.00`).
  * `currency` (`std::string`): A capitalized three-letter ISO-4217 currency code (e.g., `"USD"`).

#### C. `SeatClass` (Enum Class)
An explicit strongly-typed enumeration class indicating the passenger's seating cabin class.
* **Underlying Type**: `int`
* **Values**:
  * `SeatClass::Economy` (implicitly `0`)
  * `SeatClass::Business` (implicitly `1`)
  * `SeatClass::First` (implicitly `2`)

---

### 2. Airport Registry Types (`airport_registry.h`)

#### A. `AirportInfo` (Structure)
Holds information detailing a registered physical airport.
* **Fields**:
  * `iata_code` (`std::string`): The 3-character international airport code (e.g., `"ADD"`, `"DXB"`, `"LHR"`).
  * `city_name` (`std::string`): The name of the city serving this airport (e.g., `"Addis Ababa"`, `"Dubai"`).
  * `airport_name` (`std::string`): The full name of the airport terminal (e.g., `"Bole International Airport"`).

---

### 3. Flight Manager Types (`flight_manager.h`)

#### A. `Flight` (Structure)
Represents a scheduled commercial flight in the system database.
* **Fields**:
  * `flight_id` (`std::string`): Unique flight identification alphanumeric code (e.g., `"FL-101"`).
  * `origin_iata` (`std::string`): The 3-character IATA origin airport code.
  * `destination_iata` (`std::string`): The 3-character IATA destination airport code.
  * `departure_time` (`std::time_t`): Standard Unix epoch time (integer representing seconds since Jan 1, 1970) for departure.
  * `arrival_time` (`std::time_t`): Standard Unix epoch time for arrival.
  * `base_price` (`Money`): The starting price of the flight.
    * *Recursive Nested Structure*:
      * `base_price.amount_cents` (`long long`)
      * `base_price.currency` (`std::string`)
  * `status` (`std::string`): The status text (e.g., `"On Time"`, `"Delayed"`, `"Boarding"`, `"Cancelled"`).

#### B. `SearchCriteria` (Structure)
The input criteria packet used to query flights.
* **Fields**:
  * `origin` (`std::string`): 3-character departure IATA code.
  * `destination` (`std::string`): 3-character arrival IATA code.
  * `date_window_start` (`std::time_t`): Unix epoch representing the starting point of the search date window.
  * `date_window_end` (`std::time_t`): Unix epoch representing the ending point of the search date window.

#### C. `FlightQueryResult` (Structure)
The compound return package generated when executing a flight query search.
* **Fields**:
  * `available_flights` (`std::vector<Flight>`): A dynamic vector array storing zero or more matching `Flight` structures.
    * *Recursive Nested Structure (Flight)*:
      * `flight_id` (`std::string`)
      * `origin_iata` (`std::string`)
      * `destination_iata` (`std::string`)
      * `departure_time` (`std::time_t`)
      * `arrival_time` (`std::time_t`)
      * `base_price` (`Money` struct containing `amount_cents` and `currency`)
      * `status` (`std::string`)
  * `status` (`Status`): The status indicating query success or date boundary errors.
    * *Recursive Nested Structure (Status)*:
      * `success` (`bool`)
      * `error_code` (`std::string`)
      * `message` (`std::string`)

---

### 4. Inventory Service Types (`inventory_service.h`)

#### A. `SeatMap` (Structure)
Defines the mapping of seat allocation layouts for a single flight ID.
* **Fields**:
  * `flight_id` (`std::string`): The unique flight ID.
  * `occupied_seats` (`std::map<std::string, bool>`): A red-black tree mapping a seat identifier string (e.g., `"E1"`, `"B12"`) to a boolean flag representing whether the seat is occupied (`true`) or vacant (`false`).

#### B. `InventoryUpdate` (Structure)
The transaction package utilized to alter available seats in inventory.
* **Fields**:
  * `flight_id` (`std::string`): The target flight ID.
  * `economy_delta` (`int`): Change value (can be positive `+1` for cancellation or negative `-1` for booking) for economy class seats.
  * `business_delta` (`int`): Change value for business class seats.
  * `first_delta` (`int`): Change value for first class seats.

#### C. `InventorySnapshot` (Structure)
A read-only snapshot reflecting current seating details for a flight.
* **Fields**:
  * `economy_available` (`int`): Current vacant economy seats.
  * `business_available` (`int`): Current vacant business seats.
  * `first_available` (`int`): Current vacant first class seats.
  * `economy_capacity` (`int`): Maximum economy seating limit.
  * `business_capacity` (`int`): Maximum business seating limit.
  * `first_capacity` (`int`): Maximum first class seating limit.

---

### 5. Reservation Engine Types (`reservation_engine.h`)

#### A. `Passenger` (Structure)
Holds demographic identity details of a passenger.
* **Fields**:
  * `first_name` (`std::string`): The first name.
  * `last_name` (`std::string`): The last name.
  * `passport_number` (`std::string`): The passport alphanumeric ID.

#### B. `BookingRequest` (Structure)
The detailed booking input request packet.
* **Fields**:
  * `flight_id` (`std::string`): Target flight ID.
  * `passenger` (`Passenger`): Nested structure holding the customer's credentials.
    * *Recursive Nested Structure (Passenger)*:
      * `first_name` (`std::string`)
      * `last_name` (`std::string`)
      * `passport_number` (`std::string`)
  * `preferred_class` (`SeatClass`): Strongly-typed seating class value.
    * *Recursive Nested Type*: `SeatClass` enum class (`Economy`, `Business`, `First`).
  * `seat_number` (`std::string`): Selected seat number (or empty string for automated assignment).

#### C. `BookingResult` (Structure)
The detailed transaction summary generated on completion of booking.
* **Fields**:
  * `pnr_id` (`std::string`): The unique generated 6-character booking reference code (Passenger Name Record).
  * `total_cost` (`Money`): The calculated ticket price incorporating dynamic surges.
    * *Recursive Nested Structure (Money)*:
      * `amount_cents` (`long long`)
      * `currency` (`std::string`)
  * `status` (`Status`): Success or error details.
    * *Recursive Nested Structure (Status)*:
      * `success` (`bool`)
      * `error_code` (`std::string`)
      * `message` (`std::string`)

#### D. `ReservationStatus` (Enum Class)
An explicit enum reflecting the lifecycle stages of a ticket reservation.
* **Underlying Type**: `int`
* **Values**:
  * `ReservationStatus::Reserved` (implicitly `0`)
  * `ReservationStatus::CheckedIn` (implicitly `1`)
  * `ReservationStatus::Boarded` (implicitly `2`)

#### E. `ReservationRecord` (Structure)
The entity stored inside the internal flight reservation ledger database.
* **Fields**:
  * `pnr_id` (`std::string`): The unique generated 6-character PNR code.
  * `request` (`BookingRequest`): The original parameter request package.
    * *Recursive Nested Structure (BookingRequest)*:
      * `flight_id` (`std::string`)
      * `passenger` (nested `Passenger` struct containing `first_name`, `last_name`, and `passport_number`)
      * `preferred_class` (nested `SeatClass` enum)
      * `seat_number` (`std::string`)
  * `total_cost` (`Money`): The cost charged.
    * *Recursive Nested Structure (Money)*:
      * `amount_cents` (`long long`)
      * `currency` (`std::string`)
  * `status` (`ReservationStatus`): The current lifecycle state.
    * *Recursive Nested Type*: `ReservationStatus` enum class (`Reserved`, `CheckedIn`, `Boarded`).

---

### 6. Boarding Controller Types (`boarding_controller.h`)

#### A. `BoardingPass` (Structure)
The card record issued to checked-in travelers.
* **Fields**:
  * `pnr_id` (`std::string`): The 6-character booking identifier.
  * `gate` (`std::string`): The boarding gate (e.g. `"G1"`).
  * `boarding_group` (`int`): Boarding sequence number (Group `1`, `2`, or `3`).

#### B. `CheckInResult` (Structure)
The return packet generated upon check-in transaction completion.
* **Fields**:
  * `pass` (`BoardingPass`): The generated boarding pass.
    * *Recursive Nested Structure (BoardingPass)*:
      * `pnr_id` (`std::string`)
      * `gate` (`std::string`)
      * `boarding_group` (`int`)
  * `status` (`Status`): Success or error details.
    * *Recursive Nested Structure (Status)*:
      * `success` (`bool`)
      * `error_code` (`std::string`)
      * `message` (`std::string`)
  * `baggage_count` (`int`): The integer number of checked bags.

---

### 7. Auxiliary System Types

#### A. `OperationalReport` (`report_generator.h`)
Structure representing generated operations data summaries.
* **Fields**:
  * `report` (`std::string`): The formatted report text string.
  * `status` (`Status`): Success or error details.
    * *Recursive Nested Structure (Status)*:
      * `success` (`bool`)
      * `error_code` (`std::string`)
      * `message` (`std::string`)

#### B. `RevenueAuditResult` (`revenue_service.h`)
Structure representing financial audit query summaries.
* **Fields**:
  * `computed_total` (`long long`): Summed transaction values in cents derived from individual bookings.
  * `recorded_total` (`long long`): Central system ledger revenue state in cents.
  * `status` (`Status`): Success or mismatch details.
    * *Recursive Nested Structure (Status)*:
      * `success` (`bool`)
      * `error_code` (`std::string`)
      * `message` (`std::string`)

---

## Part 2: Success/Failure Propagation Mechanics (`make_success` & `make_failure`)

The system implements a robust, exception-free error propagation architecture. Instead of using expensive C++ standard exceptions (`throw/catch`), functions return a custom `Status` struct (or compound structs containing a `Status` object). Let's trace in full recursive detail what happens during success and failure creation, and how the system behaves.

### 1. Inside `make_success()`
* **Declaration**: `Status make_success();`
* **Definition**:
  ```cpp
  Status make_success() {
      return {true, "", ""};
  }
  ```
* **What happens**:
  1. Instantiates a `Status` object on the stack.
  2. Sets `.success` to `true`.
  3. Sets `.error_code` to an empty string `""`.
  4. Sets `.message` to an empty string `""`.
  5. The compiler returns this object by value (often optimized via Return Value Optimization, RVO).
* **Caller Handling**: The caller receives this `Status` object and checks `status.success == true`. Seeing it is successful, the caller proceeds with the happy path.

### 2. Inside `make_failure()`
* **Declaration**: `Status make_failure(const std::string& code, const std::string& message);`
* **Definition**:
  ```cpp
  Status make_failure(const std::string& code, const std::string& message) {
      Status status{false, code, message};
      if (status.error_code.empty()) {
          status.error_code = "UNKNOWN";
      }
      if (status.message.empty()) {
          status.message = "Unknown error";
      }
      return status;
  }
  ```
* **What happens**:
  1. Accepts two string references: `code` (standard classification) and `message` (friendly description).
  2. Instantiates a `Status` object on the stack with `.success = false`.
  3. Validates if `code` is empty. If it is, overwrites it with `"UNKNOWN"`.
  4. Validates if `message` is empty. If it is, overwrites it with `"Unknown error"`.
  5. Returns this populated failure `Status` object by value.
* **Caller Handling**:
  - The calling function receives the `Status` struct and evaluates `if (!status.success)`.
  - Seeing `success` is `false`, it terminates intermediate execution immediately, cleans up local resources, and **bubbles up** the failure.
  - In bubbling up, the function either returns the `Status` object directly or copies its `.error_code` and `.message` into a higher-level compound return structure (such as `BookingResult`, `CheckInResult`, or `FlightQueryResult`).
  - Finally, the top-level handler inside `controller.cpp` catches this returned structure, extracts `.message` and `.error_code`, logs the error using `log_status()`, and displays a clear error warning to the user on screen. **No CPU time is wasted on exception stack-unwinding.**

---

## Part 3: Deep Technical Registry of Every Single Function

This is the complete, comprehensive technical directory of every single function implemented across the entire Airline Management System codebase.

### 1. Core Utilities & Common Types (`common_types.cpp` & `common_types.h`)

#### A. `make_success`
* **Declaration**: `Status make_success();`
* **Accepts**: No arguments.
* **Returns**: `Status` structure.
  * *Returned Structure Contents*: `{.success = true, .error_code = "", .message = ""}`.
* **Detailed Execution Logic**:
  Instantiates and returns a `Status` struct initialized to success state.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### B. `make_failure`
* **Declaration**: `Status make_failure(const std::string& code, const std::string& message);`
* **Accepts**:
  * `code` (`const std::string&`): Error category identifier.
  * `message` (`const std::string&`): Customer-facing explanation.
* **Returns**: `Status` structure.
  * *Returned Structure Contents*: `{.success = false, .error_code = code, .message = message}` (defaults applied if empty).
* **Detailed Execution Logic**:
  Initializes a `Status` struct with `.success = false`. Overwrites empty inputs with default values (`"UNKNOWN"` / `"Unknown error"`), and returns it.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### C. `add_money`
* **Declaration**: `Status add_money(const Money& lhs, const Money& rhs, Money& out);`
* **Accepts**:
  * `lhs` (`const Money&`): Left-hand side operand.
    * *Recursive Member Structure*: `Money` struct containing `amount_cents` (`long long`) and `currency` (`std::string`).
  * `rhs` (`const Money&`): Right-hand side operand.
    * *Recursive Member Structure*: `Money` struct containing `amount_cents` (`long long`) and `currency` (`std::string`).
  * `out` (`Money&`): Output reference where result is written.
* **Returns**: `Status` structure.
  * *Returned Structure Contents*: Success or error code (`"MONEY_CURRENCY_MISMATCH"`, `"MONEY_OVERFLOW"`).
* **Detailed Execution Logic**:
  1. Checks if `lhs.currency != rhs.currency`. If true, returns `make_failure("MONEY_CURRENCY_MISMATCH", ...)`.
  2. Calls the internal anonymous helper `will_add_overflow(lhs.amount_cents, rhs.amount_cents)`.
     - *Helper Logic*: Verifies if addition violates the range limit of `std::numeric_limits<long long>::max()` or `min()`.
     - If addition overflows, returns `make_failure("MONEY_OVERFLOW", ...)`.
  3. Adds cents: `out.amount_cents = lhs.amount_cents + rhs.amount_cents`.
  4. Sets `out.currency = lhs.currency`.
  5. Returns `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `will_add_overflow()` (internal helper).
  - Calls `make_failure()` (if currency mismatched or overflowed).
  - Calls `make_success()` (if successful).

#### D. `subtract_money`
* **Declaration**: `Status subtract_money(const Money& lhs, const Money& rhs, Money& out);`
* **Accepts**:
  * `lhs` (`const Money&`): Left-hand side operand.
  * `rhs` (`const Money&`): Right-hand side operand.
  * `out` (`Money&`): Output reference where result is written.
* **Returns**: `Status` structure.
* **Detailed Execution Logic**:
  1. Checks if `lhs.currency != rhs.currency`. If true, returns `make_failure("MONEY_CURRENCY_MISMATCH", ...)`.
  2. Calls anonymous helper `will_add_overflow(lhs.amount_cents, -rhs.amount_cents)` to check for underflow or overflow.
  3. Subtracts cents: `out.amount_cents = lhs.amount_cents - rhs.amount_cents`.
  4. Sets `out.currency = lhs.currency`.
  5. Returns `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `will_add_overflow()` (internal helper).
  - Calls `make_failure()` (on error).
  - Calls `make_success()` (on success).

#### E. `parse_seat_class`
* **Declaration**: `Status parse_seat_class(const std::string& input, SeatClass& out);`
* **Accepts**:
  * `input` (`const std::string&`): Text entered by user (e.g. `"Economy"`, `"first"`).
  * `out` (`SeatClass&`): Enum reference where output is written.
* **Returns**: `Status` structure.
* **Detailed Execution Logic**:
  1. Converts a local copy of input to uppercase via `to_upper(input)`.
  2. Compares upper value:
     * If `"ECONOMY"`, sets `out = SeatClass::Economy` and returns `make_success()`.
     * If `"BUSINESS"`, sets `out = SeatClass::Business` and returns `make_success()`.
     * If `"FIRST"`, sets `out = SeatClass::First` and returns `make_success()`.
  3. If no matches, returns `make_failure("SEAT_CLASS_INVALID", "Unknown seat class")`.
* **Recursive Operations / Nested Calls**:
  - Calls `to_upper()` (inline utility).
  - Calls `make_success()` or `make_failure()`.

#### F. `seat_class_to_string`
* **Declaration**: `std::string seat_class_to_string(SeatClass seat_class);`
* **Accepts**:
  * `seat_class` (`SeatClass`): Strongly-typed seating class value.
* **Returns**: `std::string` representation (`"Economy"`, `"Business"`, `"First"`, or `"Unknown"`).
* **Detailed Execution Logic**:
  Uses a `switch (seat_class)` block to match the enum and return the corresponding string literal.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### G. `to_upper` (Inline Utility)
* **Declaration**: `inline std::string to_upper(std::string value);`
* **Accepts**:
  * `value` (`std::string`): String passed by value to be transformed.
* **Returns**: `std::string` in uppercase.
* **Detailed Execution Logic**:
  Loops character by character, converting each to uppercase via `toupper()`, and returns the mutated string.
* **Recursive Operations / Nested Calls**:
  Calls `toupper()`.

---

### 2. Airport Registry (`airport_registry.cpp` & `airport_registry.h`)

#### A. `list_all_airports`
* **Declaration**: `std::vector<AirportInfo> list_all_airports();`
* **Accepts**: No arguments.
* **Returns**: `std::vector<AirportInfo>` containing copies of all registered airports.
  * *Returned Item Recursive Structure*: Each `AirportInfo` contains `iata_code` (`std::string`), `city_name` (`std::string`), and `airport_name` (`std::string`).
* **Detailed Execution Logic**:
  Returns a predefined static global list of registered airports in the system.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### B. `find_airport_by_iata`
* **Declaration**: `const AirportInfo* find_airport_by_iata(const std::string& iata_code);`
* **Accepts**:
  * `iata_code` (`const std::string&`): Code to search for.
* **Returns**: `const AirportInfo*` (pointer to matches, or `nullptr` if not found).
* **Detailed Execution Logic**:
  1. Converts input to uppercase.
  2. Iterates over the global `AIRPORTS` list.
  3. If `airport.iata_code == input`, returns its address.
  4. Returns `nullptr` if the loop finishes with no match.
* **Recursive Operations / Nested Calls**:
  Calls `to_upper()`.

#### C. `find_airports_by_city_name`
* **Declaration**: `std::vector<AirportInfo> find_airports_by_city_name(const std::string& city_name);`
* **Accepts**:
  * `city_name` (`const std::string&`): City name to search for.
* **Returns**: `std::vector<AirportInfo>` array of matches.
* **Detailed Execution Logic**:
  1. Converts input to uppercase.
  2. Iterates through `AIRPORTS`.
  3. Converts `airport.city_name` to uppercase, checking if it matches the target city name.
  4. Appends matches to a temporary vector and returns it.
* **Recursive Operations / Nested Calls**:
  Calls `to_upper()`.

#### D. `get_airport_display_string`
* **Declaration**: `std::string get_airport_display_string(const AirportInfo& airport);`
* **Accepts**:
  * `airport` (`const AirportInfo&`): The airport reference to display.
* **Returns**: `std::string` format text.
* **Detailed Execution Logic**:
  Builds a readable single-line format string: `airport.city_name + " (" + airport.iata_code + ") - " + airport.airport_name`.
* **Recursive Operations / Nested Calls**:
  No nested calls.

---

### 3. Flight Manager (`flight_manager.cpp` & `flight_manager.h`)

#### A. `add_flight`
* **Declaration**: `Status add_flight(const Flight& flight);`
* **Accepts**:
  * `flight` (`const Flight&`): Flight to be registered.
    * *Recursive Member Structure*: `Flight` containing `flight_id` (`std::string`), `origin_iata` (`std::string`), `destination_iata` (`std::string`), `departure_time` (`std::time_t`), `arrival_time` (`std::time_t`), `base_price` (nested `Money` struct containing `amount_cents` and `currency`), and `status` (`std::string`).
* **Returns**: `Status` structure.
* **Detailed Execution Logic**:
  1. Checks if `flight.flight_id.empty()`. If true, returns `make_failure("FLIGHT_ID_MISSING", ...)`.
  2. Checks if `flight.arrival_time <= flight.departure_time`. If true, returns `make_failure("FLIGHT_TIME_INVALID", ...)`.
  3. Calls `find_flight(flight.flight_id)` to look for duplicates. If found (pointer != `nullptr`), returns `make_failure("FLIGHT_DUPLICATE", ...)`.
  4. Copies the flight. If status is empty, sets it to `"On Time"`.
  5. Appends the copy to the global private vector `g_flights`.
  6. Returns `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `find_flight()`.
  - Calls `make_failure()` or `make_success()`.

#### B. `search_flights`
* **Declaration**: `FlightQueryResult search_flights(const SearchCriteria& criteria);`
* **Accepts**:
  * `criteria` (`const SearchCriteria&`): Parameters for filtering flights.
    * *Recursive Member Structure*: `SearchCriteria` containing `origin` (`std::string`), `destination` (`std::string`), `date_window_start` (`std::time_t`), and `date_window_end` (`std::time_t`).
* **Returns**: `FlightQueryResult` structure.
  * *Returned Structure Contents*:
    * `.available_flights` (`std::vector<Flight>`): matched flights.
    * `.status` (`Status`): success or error code (`"DATE_WINDOW_INVALID"`).
* **Detailed Execution Logic**:
  1. Checks if `criteria.date_window_start > criteria.date_window_end`. If true, returns an empty result with `make_failure("DATE_WINDOW_INVALID", ...)`.
  2. Loops through `g_flights`:
     * Checks if `flight.origin_iata == criteria.origin`.
     * Checks if `flight.destination_iata == criteria.destination`.
     * Checks if `flight.departure_time >= criteria.date_window_start` and `flight.departure_time <= criteria.date_window_end`.
     * If all are true, appends the flight to a local vector.
  3. Returns the local vector alongside `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `make_failure()` or `make_success()`.

#### C. `find_flight`
* **Declaration**: `const Flight* find_flight(const std::string& flight_id);`
* **Accepts**:
  * `flight_id` (`const std::string&`): Unique flight ID.
* **Returns**: `const Flight*` (pointer to matched flight, or `nullptr`).
* **Detailed Execution Logic**:
  Loops through global vector `g_flights`. If `flight.flight_id == flight_id`, returns its address. Otherwise returns `nullptr`.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### D. `find_flight_mutable` (Internal Helper in `flight_manager.cpp`)
* **Declaration**: `Flight* find_flight_mutable(const std::string& flight_id);`
* **Accepts**:
  * `flight_id` (`const std::string&`): Unique flight ID.
* **Returns**: `Flight*` (mutable pointer to matching flight, or `nullptr`).
* **Detailed Execution Logic**:
  Similar to `find_flight()`, but returns a non-const pointer to allow modification of the flight's fields.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### E. `set_flight_status`
* **Declaration**: `Status set_flight_status(const std::string& flight_id, const std::string& new_status);`
* **Accepts**:
  * `flight_id` (`const std::string&`): Flight to update.
  * `new_status` (`const std::string&`): New status string.
* **Returns**: `Status` structure.
* **Detailed Execution Logic**:
  1. Calls the internal anonymous helper `canonicalize_status(new_status)`.
     - *Helper Logic*: Converts input to uppercase and checks if it matches `"ON TIME"`, `"DELAYED"`, `"BOARDING"`, or `"CANCELLED"`. Returns the capitalized name if valid, or empty string if invalid.
     - If the normalized string is empty, returns `make_failure("STATUS_INVALID", ...)`.
  2. Calls `find_flight_mutable(flight_id)`. If `nullptr`, returns `make_failure("FLIGHT_NOT_FOUND", ...)`.
  3. Mutates the flight: `flight_ptr->status = normalized_status`.
  4. Returns `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `canonicalize_status()` (internal helper).
  - Calls `find_flight_mutable()` (internal utility).
  - Calls `make_failure()` or `make_success()`.

#### F. `get_flight_registry`
* **Declaration**: `const std::vector<Flight>& get_flight_registry();`
* **Accepts**: No arguments.
* **Returns**: `const std::vector<Flight>&` (read-only reference to `g_flights`).
* **Detailed Execution Logic**:
  Provides a read-only getter to the global flight list vector.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### G. `count_flights_with_status`
* **Declaration**: `int count_flights_with_status(const std::string& status);`
* **Accepts**:
  * `status` (`const std::string&`): Status label to filter by.
* **Returns**: `int` (matching flights count).
* **Detailed Execution Logic**:
  Loops through `g_flights`, counts flights that match the target status string, and returns the total.
* **Recursive Operations / Nested Calls**:
  No nested calls.

---

### 4. Inventory Service (`inventory_service.cpp` & `inventory_service.h`)

#### A. `initialize_inventory`
* **Declaration**: `Status initialize_inventory(const std::vector<Flight>& flights, int economy_capacity, int business_capacity, int first_capacity);`
* **Accepts**:
  * `flights` (`const std::vector<Flight>&`): Dynamic array of flights to allocate seats for.
  * `economy_capacity` (`int`): Seats allocated for Economy.
  * `business_capacity` (`int`): Seats allocated for Business.
  * `first_capacity` (`int`): Seats allocated for First.
* **Returns**: `Status` structure.
* **Detailed Execution Logic**:
  1. Checks if any capacity is negative. If so, returns `make_failure("INVALID_CAPACITY", ...)`.
  2. Clears the global `g_inventory` and `g_seat_maps` collections.
  3. Loops over every `Flight` in the input vector:
     * Instantiates an `InventoryState` struct initialized with the respective seat capacities and sets the available counts to match them.
     * Maps `flight_id` to this `InventoryState` in `g_inventory`.
     * Adds an empty `SeatMap` mapping for the `flight_id` in `g_seat_maps`.
  4. Returns `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `make_failure()` or `make_success()`.

#### B. `check_availability`
* **Declaration**: `Status check_availability(const std::string& flight_id, SeatClass seat_class);`
* **Accepts**:
  * `flight_id` (`const std::string&`): Target flight code.
  * `seat_class` (`SeatClass`): Target seating cabin class enum.
* **Returns**: `Status` structure.
* **Detailed Execution Logic**:
  1. Searches `g_inventory` for the key `flight_id`.
  2. If missing, returns `make_failure("INVENTORY_MISSING", ...)`.
  3. Checks the respective seat counter for the target class:
     * If `Economy`, checks `economy_available`.
     * If `Business`, checks `business_available`.
     * If `First`, checks `first_available`.
  4. If the counter is `0`, returns `make_failure("INVENTORY_SOLD_OUT", "No seats available in this class")`.
  5. If seats are available (counter > 0), returns `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `make_failure()` or `make_success()`.

#### C. `update_inventory`
* **Declaration**: `Status update_inventory(const InventoryUpdate& update);`
* **Accepts**:
  * `update` (`const InventoryUpdate&`): Seating update parameters.
    * *Recursive Member Structure*: `InventoryUpdate` containing `flight_id` (`std::string`), `economy_delta` (`int`), `business_delta` (`int`), and `first_delta` (`int`).
* **Returns**: `Status` structure.
* **Detailed Execution Logic**:
  1. Searches `g_inventory` for `update.flight_id`. If missing, returns `make_failure("INVENTORY_MISSING", ...)`.
  2. Performs bounds-checking using the internal anonymous helper `check_delta()`.
     - *Helper Logic*: Verifies that applying the delta will not cause the available seat count to drop below `0` or exceed the maximum cabin capacity.
     - If the bounds check fails, returns `make_failure("INVENTORY_LIMIT_VIOLATION", ...)`.
  3. Updates the available seat counts:
     * `economy_available += update.economy_delta`
     * `business_available += update.business_delta`
     * `first_available += update.first_delta`
  4. Returns `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `check_delta()` (internal helper).
  - Calls `make_failure()` or `make_success()`.

#### D. `reserve_seat`
* **Declaration**: `Status reserve_seat(const std::string& flight_id, const std::string& seat_number);`
* **Accepts**:
  * `flight_id` (`const std::string&`): Target flight code.
  * `seat_number` (`const std::string&`): Seat number to reserve (e.g. `"E1"`).
* **Returns**: `Status` structure.
* **Detailed Execution Logic**:
  1. Searches `g_seat_maps` for `flight_id`. If missing, returns `make_failure("SEAT_MAP_MISSING", ...)`.
  2. Searches the seat map's `occupied_seats` map for the key `seat_number`.
  3. If found and its value is `true` (occupied), returns `make_failure("SEAT_OCCUPIED", "Seat is already occupied")`.
  4. Reserves the seat: sets `occupied_seats[seat_number] = true`.
  5. Returns `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `make_failure()` or `make_success()`.

#### E. `is_seat_taken`
* **Declaration**: `bool is_seat_taken(const std::string& flight_id, const std::string& seat_number);`
* **Accepts**:
  * `flight_id` (`const std::string&`): Target flight ID.
  * `seat_number` (`const std::string&`): Seat number.
* **Returns**: `bool` (`true` if seat is occupied, `false` if vacant).
* **Detailed Execution Logic**:
  1. Searches `g_seat_maps` for `flight_id`. If missing, returns `false`.
  2. Looks up `seat_number` in the seat map. Returns the boolean occupancy value.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### F. `get_inventory_snapshot`
* **Declaration**: `InventorySnapshot get_inventory_snapshot(const std::string& flight_id);`
* **Accepts**:
  * `flight_id` (`const std::string&`): Target flight ID.
* **Returns**: `InventorySnapshot` structure.
  * *Returned Structure Contents*: Current vacant and capacity seating counts (returns all `0`s if flight is not found).
* **Detailed Execution Logic**:
  Searches `g_inventory` for `flight_id`. If found, copies its seating counts and capacities into an `InventorySnapshot` struct and returns it.
* **Recursive Operations / Nested Calls**:
  No nested calls.

---

### 5. Reservation Engine (`reservation_engine.cpp` & `reservation_engine.h`)

#### A. `create_booking`
* **Declaration**: `BookingResult create_booking(const BookingRequest& request);`
* **Accepts**:
  * `request` (`const BookingRequest&`): The request parameters.
    * *Recursive Member Structure*: `BookingRequest` containing `flight_id` (`std::string`), `passenger` (nested `Passenger` struct containing `first_name`, `last_name`, and `passport_number`), `preferred_class` (nested `SeatClass` enum), and `seat_number` (`std::string`).
* **Returns**: `BookingResult` structure.
  * *Returned Structure Contents*:
    * `.pnr_id` (`std::string`): The 6-character PNR code.
    * `.total_cost` (`Money`): The dynamically computed ticket price.
    * `.status` (`Status`): Success or error details.
* **Detailed Execution Logic**:
  1. Calls `find_flight(request.flight_id)` to verify the flight exists. If missing, returns a failure result with `"BOOKING_FLIGHT_UNKNOWN"`.
  2. Calls `check_availability(request.flight_id, request.preferred_class)` to verify seat availability. If sold out, returns the failure status.
  3. Retrieves the flight's inventory snapshot via `get_inventory_snapshot(request.flight_id)`.
  4. Determines seating variables:
     * If class is Economy: `remaining = snapshot.economy_available`, `capacity = snapshot.economy_capacity`.
     * If Business: `remaining = snapshot.business_available`, `capacity = snapshot.business_capacity`.
     * If First: `remaining = snapshot.first_available`, `capacity = snapshot.first_capacity`.
  5. Prepares inventory allocation: sets up an `InventoryUpdate` struct with delta values (decrements the respective class count by `-1`).
  6. Calls `update_inventory(update)`. If it fails, returns the inventory allocation failure status.
  7. Calculates the ticket price using `calculate_dynamic_price(flight.base_price, remaining, capacity, status)`. If pricing calculation fails (e.g. price overflow), returns the failure status.
  8. Assigns the seat:
     * If `request.seat_number` is empty, calls `generate_seat_number(request.flight_id, request.preferred_class)` to automatically assign the next sequential seat number.
     * Otherwise, uses the passenger's preferred seat choice.
  9. Calls `reserve_seat(request.flight_id, assigned_seat)` to register the seat assignment. If the seat is occupied, calls `update_inventory()` with a reverse delta (`+1`) to restore the seat in the inventory counts, and returns a `"SEAT_OCCUPIED"` failure status.
  10. Calls `generate_pnr()` to create a unique 6-character booking identifier.
  11. Calls `record_revenue(final_price)`. If it fails, rolls back the inventory allocation and seat reservation, and returns the failure status.
  12. Registers the booking: creates a `ReservationRecord` struct and inserts it into the global `g_reservations` map.
  13. Returns the successful booking summary: PNR, dynamic ticket price, and `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `find_flight()`.
  - Calls `check_availability()`.
  - Calls `get_inventory_snapshot()`.
  - Calls `update_inventory()`.
  - Calls `calculate_dynamic_price()`.
  - Calls `generate_seat_number()` (internal helper).
  - Calls `reserve_seat()`.
  - Calls `generate_pnr()`.
  - Calls `record_revenue()`.
  - Calls `make_success()` or `make_failure()`.

#### B. `find_reservation`
* **Declaration**: `const ReservationRecord* find_reservation(const std::string& pnr_id);`
* **Accepts**:
  * `pnr_id` (`const std::string&`): The 6-character PNR code to search for.
* **Returns**: `const ReservationRecord*` (pointer to matching record, or `nullptr`).
* **Detailed Execution Logic**:
  Searches the global `g_reservations` map for the key `pnr_id`. If found, returns its address. Otherwise returns `nullptr`.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### C. `update_reservation_status`
* **Declaration**: `Status update_reservation_status(const std::string& pnr_id, ReservationStatus new_status);`
* **Accepts**:
  * `pnr_id` (`const std::string&`): Booking identifier to update.
  * `new_status` (`ReservationStatus`): New reservation lifecycle status.
* **Returns**: `Status` structure.
* **Detailed Execution Logic**:
  1. Searches `g_reservations` for `pnr_id`.
  2. If missing, returns `make_failure("RESERVATION_NOT_FOUND", ...)`.
  3. Modifies the record: updates its status field.
  4. Returns `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `make_failure()` or `make_success()`.

#### D. `get_all_reservations`
* **Declaration**: `std::vector<ReservationRecord> get_all_reservations();`
* **Accepts**: No arguments.
* **Returns**: `std::vector<ReservationRecord>` array of copies of all bookings.
* **Detailed Execution Logic**:
  Loops through the global `g_reservations` map, copying each record into a vector, and returns it.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### E. `get_booking_totals`
* **Declaration**: `std::vector<Money> get_booking_totals();`
* **Accepts**: No arguments.
* **Returns**: `std::vector<Money>` containing individual booking prices.
* **Detailed Execution Logic**:
  Loops through all reservations in `g_reservations`, copies their ticket prices into a vector, and returns it.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### F. `get_total_reservations`
* **Declaration**: `int get_total_reservations();`
* **Accepts**: No arguments.
* **Returns**: `int` (total active bookings count).
* **Detailed Execution Logic**:
  Returns the size of the global `g_reservations` map.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### G. `get_recorded_revenue`
* **Declaration**: `long long get_recorded_revenue();`
* **Accepts**: No arguments.
* **Returns**: `long long` (total ledger balance in cents).
* **Detailed Execution Logic**:
  Returns the value stored in the global ledger counter `g_recorded_revenue`.
* **Recursive Operations / Nested Calls**:
  No nested calls.

---

### 6. Boarding Controller (`boarding_controller.cpp` & `boarding_controller.h`)

#### A. `process_check_in`
* **Declaration**: `CheckInResult process_check_in(const std::string& pnr_id, int baggage_count);`
* **Accepts**:
  * `pnr_id` (`const std::string&`): Passenger's booking PNR reference.
  * `baggage_count` (`int`): Passenger's checked bags count.
* **Returns**: `CheckInResult` structure.
  * *Returned Structure Contents*:
    * `.pass` (`BoardingPass`): Boarding details (PNR, Gate, and Boarding Group).
    * `.status` (`Status`): Success or error details.
    * `.baggage_count` (`int`): Confirmed bags count.
* **Detailed Execution Logic**:
  1. Normalizes the PNR reference by calling the helper `normalize_pnr_input(pnr_id)`.
     - *Helper Logic*: Converts characters to uppercase and strips the `"PNR-"` prefix if present.
  2. Calls `is_valid_pnr(pnr)` to check if the format is valid. If invalid, returns a failure result with `"CHECKIN_PNR_INVALID"`.
  3. Calls `is_valid_baggage_count(baggage_count)` to check if the count is within limits (0 to 5). If invalid, returns a failure result with `"CHECKIN_BAGGAGE_INVALID"`.
  4. Calls `find_reservation(pnr)` to look up the reservation details. If missing, returns a failure result with `"CHECKIN_RESERVATION_MISSING"`.
  5. Verifies that the reservation status is currently set to `ReservationStatus::Reserved`. If not (e.g. passenger already checked in), returns a failure result with `"CHECKIN_ALREADY_PROCESSED"`.
  6. Generates boarding details:
     * Calls `gate_for(flight_id)` to assign a gate.
       - *Helper Logic*: Extracts the last character of the flight ID. If it is a digit, returns it prefixed by `"G"` (e.g. `"G1"` for flight `"FL-101"`). If not, defaults to `"G1"`.
     * Calls `boarding_group_for(seat_class)` to assign a boarding group.
       - *Helper Logic*: First Class = Group `1`, Business Class = Group `2`, Economy = Group `3`.
  7. Calls `update_reservation_status(pnr, ReservationStatus::CheckedIn)` to update the reservation state to `CheckedIn` in the global ledger.
  8. Increments the global counter `g_total_checkins`.
  9. Returns the completed boarding pass: PNR, Gate, Boarding Group, baggage count, and `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `normalize_pnr_input()` (internal helper).
  - Calls `is_valid_pnr()`.
  - Calls `is_valid_baggage_count()`.
  - Calls `find_reservation()`.
  - Calls `gate_for()` (internal helper).
  - Calls `boarding_group_for()` (internal helper).
  - Calls `update_reservation_status()`.
  - Calls `make_success()` or `make_failure()`.

#### B. `update_flight_status` (Delegator in `boarding_controller.cpp`)
* **Declaration**: `Status update_flight_status(const std::string& flight_id, const std::string& new_status);`
* **Accepts**:
  * `flight_id` (`const std::string&`): Flight to update.
  * `new_status` (`const std::string&`): New status string.
* **Returns**: `Status` structure.
* **Detailed Execution Logic**:
  Delegates the call directly to `set_flight_status(flight_id, new_status)` in [flight_manager.cpp](file:///C:/Users/hi/comment/group-project/airline-management-codes/flight_manager.cpp).
* **Recursive Operations / Nested Calls**:
  Calls `set_flight_status()`.

#### C. `get_total_checkins`
* **Declaration**: `int get_total_checkins();`
* **Accepts**: No arguments.
* **Returns**: `int` (total check-ins processed count).
* **Detailed Execution Logic**:
  Returns the value stored in the global counter `g_total_checkins`.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### D. `get_total_boarded`
* **Declaration**: `int get_total_boarded();`
* **Accepts**: No arguments.
* **Returns**: `int` (total boarded passengers count).
* **Detailed Execution Logic**:
  Returns the value stored in the global counter `g_total_boarded`.
* **Recursive Operations / Nested Calls**:
  No nested calls.

---

### 7. Revenue Service (`revenue_service.cpp` & `revenue_service.h`)

#### A. `calculate_dynamic_price`
* **Declaration**: `Money calculate_dynamic_price(const Money& base_price, int remaining_seats, int total_capacity, Status& status);`
* **Accepts**:
  * `base_price` (`const Money&`): Starting ticket price.
    * *Recursive Member Structure*: `Money` struct containing `amount_cents` (`long long`) and `currency` (`std::string`).
  * `remaining_seats` (`int`): Available vacant seats.
  * `total_capacity` (`int`): Maximum cabin seating limit.
  * `status` (`Status&`): Output reference where operation status is written.
* **Returns**: `Money` struct containing the computed ticket price.
* **Detailed Execution Logic**:
  1. Checks if `total_capacity <= 0`. If true, sets status to failure `"PRICE_CAPACITY_INVALID"` and returns `base_price`.
  2. Checks if `remaining_seats <= 0`. If true, sets status to failure `"PRICE_NO_SEATS"` and returns `base_price`.
  3. Calculates availability ratio: `ratio = remaining_seats / total_capacity`.
  4. Compares ratio to demand thresholds:
     * **High Demand**: If `ratio <= 0.10` (10% or less seats left), applies a `3/2` multiplier (`1.5x` price).
     * **Moderate Demand**: If `ratio <= 0.25` (25% or less seats left), applies a `6/5` multiplier (`1.2x` price).
     * **Low Demand**: Otherwise, applies a `1/1` multiplier (base price).
  5. Performs overflow checks using the internal anonymous helper `will_mul_overflow()`.
     - *Helper Logic*: Verifies if multiplying base cents by the numerator exceeds the maximum value of `std::numeric_limits<long long>::max()`.
     - If multiplication overflows, sets status to failure `"PRICE_OVERFLOW"` and returns `base_price`.
  6. Computes the price: `cents = (base_price.amount_cents * numerator) / denominator`.
  7. Sets `status = make_success()` and returns the dynamic `Money` price.
* **Recursive Operations / Nested Calls**:
  - Calls `will_mul_overflow()` (internal helper).
  - Calls `make_success()` or `make_failure()`.

#### B. `audit_revenue`
* **Declaration**: `RevenueAuditResult audit_revenue(const std::vector<Money>& booking_totals, long long recorded_total);`
* **Accepts**:
  * `booking_totals` (`const std::vector<Money>&`): Individual transaction prices.
  * `recorded_total` (`long long`): Total ledger balance in cents.
* **Returns**: `RevenueAuditResult` structure.
  * *Returned Structure Contents*:
    * `.computed_total` (`long long`): Summed cents from individual bookings.
    * `.recorded_total` (`long long`): Ledger balance in cents.
    * `.status` (`Status`): Success or mismatch details.
* **Detailed Execution Logic**:
  1. Initializes `computed_cents = 0`.
  2. Loops over each `Money` struct in the `booking_totals` vector:
     * Checks if the currency is set to `"USD"`. If mismatch, returns a failure result with `"AUDIT_CURRENCY"`.
     * Checks if adding this ticket cost will overflow `long long` limits. If overflow, returns a failure result with `"AUDIT_OVERFLOW"`.
     * Adds the ticket cost to the computed sum: `computed_cents += amount.amount_cents`.
  3. Compares the computed sum against the recorded ledger total. If they do not match, returns a failure result with `"AUDIT_MISMATCH"`.
  4. If the audit succeeds, returns the computed sum, recorded total, and `make_success()`.
* **Recursive Operations / Nested Calls**:
  - Calls `make_failure()` or `make_success()`.

---

### 8. Report Generator (`report_generator.cpp` & `report_generator.h`)

#### A. `generate_operational_report`
* **Declaration**: `OperationalReport generate_operational_report(int total_reservations, int total_checked_in, int total_boarded, int delayed_flights);`
* **Accepts**:
  * `total_reservations` (`int`): Total bookings count.
  * `total_checked_in` (`int`): Total check-ins count.
  * `total_boarded` (`int`): Total boarded passengers count.
  * `delayed_flights` (`int`): Total delayed flights count.
* **Returns**: `OperationalReport` structure.
  * *Returned Structure Contents*: Formatted report string and `Status` struct.
* **Detailed Execution Logic**:
  Formats the operational statistics into a single report summary string using `std::ostringstream` and returns it alongside `make_success()`.
* **Recursive Operations / Nested Calls**:
  Calls `make_success()`.

---

### 9. Security Utilities (`security_utils.cpp` & `security_utils.h`)

#### A. `mask_identifier`
* **Declaration**: `std::string mask_identifier(const std::string& value, std::size_t visible_suffix);`
* **Accepts**:
  * `value` (`const std::string&`): String to mask (e.g. `"EP123456"`).
  * `visible_suffix` (`std::size_t`): Number of trailing characters to leave visible.
* **Returns**: `std::string` (masked string, e.g. `"******56"`).
* **Detailed Execution Logic**:
  If the string length is less than or equal to `visible_suffix`, returns it unmasked. Otherwise, replaces the leading characters with asterisks `'*'` and appends the trailing visible suffix substring.
* **Recursive Operations / Nested Calls**:
  No nested calls.

#### B. `log_status`
* **Declaration**: `void log_status(const Status& status, const std::string& context);`
* **Accepts**:
  * `status` (`const Status&`): Status struct to inspect.
  * `context` (`const std::string&`): Log context text label (e.g. `"System setup"`).
* **Returns**: Nothing (`void`).
* **Detailed Execution Logic**:
  If `status.success` is `true`, returns immediately. Otherwise, writes an error log message containing the context, error message, and standard error code to `std::cerr`.
* **Recursive Operations / Nested Calls**:
  No nested calls.

---

### 10. Validator Services (`validator.cpp` & `validator.h`)

#### A. `is_valid_passport`
* **Declaration**: `bool is_valid_passport(const std::string& passport_number);`
* **Accepts**:
  * `passport_number` (`const std::string&`): Passport to validate.
* **Returns**: `bool` (`true` if valid, `false` if invalid).
* **Detailed Execution Logic**:
  1. Checks if length is between 6 and 9 characters. If not, returns `false`.
  2. Iterates over each character, calling the helper `is_upper_alnum(ch)`.
     - *Helper Logic*: Returns `true` if the character is an uppercase letter or digit.
     - If any character is not uppercase alphanumeric, returns `false`.
  3. Returns `true`.
* **Recursive Operations / Nested Calls**:
  - Calls `is_upper_alnum()` (internal helper).

#### B. `is_valid_pnr`
* **Declaration**: `bool is_valid_pnr(const std::string& pnr_id);`
* **Accepts**:
  * `pnr_id` (`const std::string&`): PNR reference to validate.
* **Returns**: `bool` (`true` if valid, `false` if invalid).
* **Detailed Execution Logic**:
  1. Checks if length is exactly 6 characters. If not, returns `false`.
  2. Checks each character using `is_upper_alnum(ch)`. If any character is not uppercase alphanumeric, returns `false`.
  3. Returns `true`.
* **Recursive Operations / Nested Calls**:
  - Calls `is_upper_alnum()` (internal helper).

#### C. `is_valid_seat_number`
* **Declaration**: `bool is_valid_seat_number(const std::string& seat_number);`
* **Accepts**:
  * `seat_number` (`const std::string&`): Seat number to validate.
* **Returns**: `bool` (`true` if valid, `false` if invalid).
* **Detailed Execution Logic**:
  1. Checks if length is between 2 and 5 characters. If not, returns `false`.
  2. Verifies if format is either Letter+Digits or Digits+Letter by calling:
     * `is_letter_followed_by_digits(seat_number)`: Checks if the seat number is a single letter followed entirely by digits (e.g., `"A12"`).
     * `is_digits_followed_by_letter(seat_number)`: Checks if the seat number starts with digits followed by a single letter (e.g., `"12A"`).
     * If neither format matches, returns `false`.
  3. Returns `true`.
* **Recursive Operations / Nested Calls**:
  - Calls `is_letter_followed_by_digits()` (internal helper).
  - Calls `is_digits_followed_by_letter()` (internal helper).

#### D. `is_valid_baggage_count`
* **Declaration**: `bool is_valid_baggage_count(int baggage_count);`
* **Accepts**:
  * `baggage_count` (`int`): Checked bags count.
* **Returns**: `bool` (`true` if valid, `false` if invalid).
* **Detailed Execution Logic**:
  Checks if baggage count is between `0` and `5` inclusive. Returns the result.
* **Recursive Operations / Nested Calls**:
  No nested calls.

---

### 11. Controller Module (`controller.cpp` & `controller.h`)

#### A. `initialize_system`
* **Declaration**: `void initialize_system();`
* **Accepts**: No arguments.
* **Returns**: Nothing (`void`).
* **Detailed Execution Logic**:
  1. Prepares static mock flights:
     * Sets up dates via `now_plus_days()` helpers.
     * Triggers `add_flight()` for flight `"FL-101"` (Addis Ababa `"ADD"` to Dubai `"DXB"`, base price `$450.00`).
     * Triggers `add_flight()` for flight `"FL-102"` (Addis Ababa `"ADD"` to London `"LHR"`, base price `$850.00`).
     * Triggers `add_flight()` for flight `"FL-201"` (Dubai `"DXB"` to Addis Ababa `"ADD"`, base price `$400.00`).
  2. Calls `get_flight_registry()` to retrieve the flights vector.
  3. Calls `initialize_inventory(flights, 120, 24, 12)` to prepare seats for booking (120 Economy, 24 Business, 12 First Class).
* **Recursive Operations / Nested Calls**:
  - Calls `now_plus_days()` (internal helper).
  - Calls `add_flight()`.
  - Calls `get_flight_registry()`.
  - Calls `initialize_inventory()`.

#### B. `run_repl`
* **Declaration**: `void run_repl();`
* **Accepts**: No arguments.
* **Returns**: Nothing (`void`).
* **Detailed Execution Logic**:
  1. Displays the system options list menu by calling `print_help()`.
  2. Enters a `while (true)` interactive CLI loop:
     * Prints prompt `airline> ` and captures user input string via `std::getline(std::cin, line)`.
     * Skips empty lines.
     * Parses selection using `try_parse_int(line, choice)`. If parsing fails, prints an error and loops back.
     * Routes selection:
       * `case 1`: Calls `handle_search_command()`.
       * `case 2`: Calls `handle_book_command()`.
       * `case 3`: Calls `handle_checkin_command()`.
       * `case 4`: Calls `handle_status_command()`.
       * `case 5`: Calls `handle_report_command()`.
       * `case 6`: Calls `print_help()`.
       * `case 7`: Prints goodbye message and exits the `while` loop, returning control to `main()`.
       * If choice is outside range `[1, 7]`, prints an error and loops back.
* **Recursive Operations / Nested Calls**:
  - Calls `print_help()`.
  - Calls `try_parse_int()`.
  - Calls `handle_search_command()`.
  - Calls `handle_book_command()`.
  - Calls `handle_checkin_command()`.
  - Calls `handle_status_command()`.
  - Calls `handle_report_command()`.

#### C. `try_parse_int`
* **Declaration**: `bool try_parse_int(const std::string& str, int& out);`
* **Accepts**:
  * `str` (`const std::string&`): Input string to parse.
  * `out` (`int&`): Output reference where parsed integer is written.
* **Returns**: `bool` (`true` if parsing succeeded, `false` if failed).
* **Detailed Execution Logic**:
  1. Identifies the string bounds and skips leading whitespace characters using a pointer loop.
  2. If the remaining string is empty, returns `false`.
  3. Checks for negative signs: if `str[i] == '-'`, returns `false` (negative options are invalid).
  4. Runs a loop over the remaining characters:
     * If any character is not a digit (`!std::isdigit`), returns `false`.
     * Calculates the accumulating integer value safely:
       `test_val = accumulated_val * 10 + (ch - '0')`.
     * Checks for integer overflow: if `test_val > std::numeric_limits<int>::max()`, returns `false`.
  5. Assigns the parsed value: `out = static_cast<int>(test_val)`.
  6. Returns `true`. **No exceptions are thrown, and no try-catch blocks are used.**
* **Recursive Operations / Nested Calls**:
  Calls `std::isdigit()`.

#### D. `handle_search_command`
* **Declaration**: `void handle_search_command();`
* **Accepts**: No arguments.
* **Returns**: Nothing.
* **Detailed Execution Logic**:
  1. Prints search heading.
  2. Calls `get_interactive_search()` to collect search parameters from the user.
  3. Executes `search_flights(criteria)`.
  4. If query fails, prints the error message and returns.
  5. Caches matches: clears the global vector `g_last_search_results` in `controller.cpp` and stores the search matches.
  6. Displays the matching flights, base prices, status, and remaining seat counts (Economy, Business, First) from the inventory snap.
* **Recursive Operations / Nested Calls**:
  - Calls `get_interactive_search()` (helper in `interactive_search_helper.cpp`).
  - Calls `search_flights()`.
  - Calls `get_inventory_snapshot()`.
  - Calls `seat_class_to_string()`.

#### E. `handle_book_command`
* **Declaration**: `void handle_book_command();`
* **Accepts**: No arguments.
* **Returns**: Nothing.
* **Detailed Execution Logic**:
  1. Checks if `g_last_search_results` is empty. If it is, prints a warning to search first, and returns.
  2. Displays the last search results and prompts the user: `Choose a flight by number or enter a flight ID:`.
  3. Captures input. If the input is parsed as a number within range, resolves the flight ID from the cached results. Otherwise, uses the input directly.
  4. Prompts for cabin class: Economy, Business, First. Captures and maps input to `SeatClass`.
  5. Prompts for passenger's first name, last name, and passport number.
  6. Calls `is_valid_passport(passport)` to validate the passport format. If invalid, aborts the booking flow.
  7. Prompts for seat selection. If entered, calls `is_valid_seat_number(seat)` to validate format. If invalid, aborts.
  8. Prepares request: populates `BookingRequest` struct.
  9. Calls `create_booking(request)`.
  10. Displays transaction summary on success: PNR, passenger name, flight ID, seat assignment, and dynamically computed price. If booking fails, calls `log_status()` and displays error.
* **Recursive Operations / Nested Calls**:
  - Calls `try_parse_int()`.
  - Calls `is_valid_passport()`.
  - Calls `is_valid_seat_number()`.
  - Calls `create_booking()`.
  - Calls `log_status()`.

#### F. `handle_checkin_command`
* **Declaration**: `void handle_checkin_command();`
* **Accepts**: No arguments.
* **Returns**: Nothing.
* **Detailed Execution Logic**:
  1. Calls `get_all_reservations()` to list active reservations in `"Reserved"` status.
  2. Prompts the user: `Choose a booking by number or enter a PNR:`.
  3. Resolves PNR: if input is a number in range, maps to corresponding booking's PNR. Otherwise, uses the input directly.
  4. Prompts for checked bags count. Captures input and parses via `try_parse_int()`. If empty, defaults to `0`. If parsing fails, aborts.
  5. Calls `process_check_in(pnr, baggage_count)`.
  6. On success, prints the boarding pass details: PNR, passenger name, flight ID, seat assignment, gate assignment, boarding group, and checked baggage count. If check-in fails, calls `log_status()` and displays error.
* **Recursive Operations / Nested Calls**:
  - Calls `get_all_reservations()`.
  - Calls `try_parse_int()`.
  - Calls `process_check_in()`.
  - Calls `log_status()`.

#### G. `handle_status_command`
* **Declaration**: `void handle_status_command();`
* **Accepts**: No arguments.
* **Returns**: Nothing.
* **Detailed Execution Logic**:
  1. Calls `get_flight_registry()` to retrieve flights, and prints each flight's ID, route, and current status.
  2. Prompts the user: `Choose a flight by number or enter a flight ID:`.
  3. Resolves flight ID: if input is a number in range, maps to corresponding flight's ID. Otherwise, uses the input directly.
  4. Prints status options: `1. On Time, 2. Delayed, 3. Boarding, 4. Cancelled`.
  5. Captures selection and parses via `try_parse_int()`.
  6. Maps choice to target status string: `"On Time"`, `"Delayed"`, `"Boarding"`, `"Cancelled"`. If invalid, aborts.
  7. Calls `update_flight_status(flight_id, target_status)`.
  8. Displays status update confirmation. If update fails, calls `log_status()` and displays error.
* **Recursive Operations / Nested Calls**:
  - Calls `get_flight_registry()`.
  - Calls `try_parse_int()`.
  - Calls `update_flight_status()` (delegator in boarding controller).
  - Calls `log_status()`.

#### H. `handle_report_command`
* **Declaration**: `void handle_report_command();`
* **Accepts**: No arguments.
* **Returns**: Nothing.
* **Detailed Execution Logic**:
  1. Calls `get_booking_totals()` to compile transaction prices, and `get_recorded_revenue()` to retrieve ledger balance.
  2. Calls `audit_revenue(booking_totals, recorded_total)` to run a financial audit.
  3. Prints audit status result: Passed or Failed (includes mismatch details on failure).
  4. Gathers operational stats:
     * `get_total_reservations()`
     * `get_total_checkins()`
     * `get_total_boarded()`
     * `count_flights_with_status("Delayed")`
  5. Calls `generate_operational_report(reservations, checkins, boarded, delays)` to compile statistics.
  6. Prints the formatted summary report.
* **Recursive Operations / Nested Calls**:
  - Calls `get_booking_totals()`.
  - Calls `get_recorded_revenue()`.
  - Calls `audit_revenue()`.
  - Calls `get_total_reservations()`.
  - Calls `get_total_checkins()`.
  - Calls `get_total_boarded()`.
  - Calls `count_flights_with_status()`.
  - Calls `generate_operational_report()`.

#### I. `print_help`
* **Declaration**: `void print_help();`
* **Accepts**: No arguments.
* **Returns**: Nothing (`void`).
* **Detailed Execution Logic**:
  Prints the structured CLI options list menu to the terminal.
* **Recursive Operations / Nested Calls**:
  No nested calls.

---

### 12. Main Program Entry Point (`main.cpp`)

#### A. `main`
* **Declaration**: `int main();`
* **Accepts**: No arguments.
* **Returns**: `int` (Exit code `0` on clean exit).
* **Detailed Execution Logic**:
  1. Serves as the program entry point.
  2. Calls `initialize_system()` to load default flights and allocate seating capacities in the inventory map.
  3. Calls `run_repl()` to enter the interactive REPL CLI loop.
  4. Once `run_repl()` completes (user selects Choice 7), returns `0` to the operating system, cleanly releasing all in-memory database structures and terminating the process.
* **Recursive Operations / Nested Calls**:
  - Calls `initialize_system()`.
  - Calls `run_repl()`.

---

## Part 4: Complete End-to-End Dynamic Trace of CLI REPL Options

Below is the exhaustive, detailed execution flow walkthrough of each CLI option. Every scenario outlines both the **User Perspective** (CLI actions and responses) and the **Developer Perspective** (internal function call stacks, parameters, return values, and step-by-step logic execution).

---

### Phase 1: Program Startup & Database Loading

#### 1. User Perspective
The user opens a terminal and runs the executable:
```powershell
.\airline.exe
```
Immediately, the system loads, initializes the flight database, and displays the main command loop menu.

#### 2. Developer Perspective & Deep Execution Call-Stack
When `airline.exe` starts, the operating system loads the binary and calls the entry point `main()` in [main.cpp](file:///C:/Users/hi/comment/group-project/airline-management-codes/main.cpp).

##### Execution Flow:
1. `main()` invokes `initialize_system()` located in [controller.cpp](file:///C:/Users/hi/comment/group-project/airline-management-codes/controller.cpp).
2. Inside `initialize_system()`, three default mock flights are loaded:
   * **Flight 1**: ID `"FL-101"`, Addis Ababa (`"ADD"`) to Dubai (`"DXB"`), departing in 1 day. Base ticket price is `$450.00`.
   * **Flight 2**: ID `"FL-102"`, Addis Ababa `"ADD"` to London `"LHR"`, departing in 2 days. Base ticket price is `$850.00`.
   * **Flight 3**: ID `"FL-201"`, Dubai `"DXB"` to Addis Ababa `"ADD"`, departing in 3 days. Base ticket price is `$400.00`.
3. To load each flight, `initialize_system()` calls:
   ```cpp
   Status add_flight(const Flight& flight)
   ```
   * *Parameters*: `flight` = `{flight_id = "FL-101", origin_iata = "ADD", destination_iata = "DXB", departure_time = timestamp, arrival_time = timestamp, base_price = {amount_cents = 45000, currency = "USD"}, status = ""}`.
   * *Internal Logic*:
     - Checks if flight ID is empty (it is not).
     - Checks if arrival time is after departure time (it is).
     - Calls `find_flight("FL-101")` to scan `g_flights` for duplicates.
     - Seeing no duplicates, sets status to `"On Time"`, copies the struct, and appends it to the global private vector `g_flights`.
     - Returns `make_success()`.
4. After loading the flights, `initialize_system()` calls:
   ```cpp
   Status initialize_inventory(const std::vector<Flight>& flights, int economy_capacity, int business_capacity, int first_capacity)
   ```
   * *Parameters*: `flights` = reference to `g_flights`, capacities = `120`, `24`, `12`.
   * *Internal Logic*:
     - Verifies capacities are non-negative.
     - Iterates over each flight:
       - Creates an `InventoryState` struct with available seats set to capacities: `{.economy_available = 120, .business_available = 24, .first_available = 12, .economy_capacity = 120, .business_capacity = 24, .first_capacity = 12}`.
       - Maps the flight ID to this `InventoryState` in `g_inventory`.
       - Adds a blank `SeatMap` entry in `g_seat_maps`.
     - Returns `make_success()`.
5. Control returns to `main()`, which invokes `run_repl()`.

---

### Phase 2: Choice 1 - Interactive Flight Search

#### 1. User Perspective
1. User enters `1` in the main menu:
   ```text
   airline> 1
   ```
2. The CLI starts `Flight Search Setup`.
3. Displays a paginated list of departure airports:
   ```text
   Available Airports:
     1. Addis Ababa (ADD) - Addis Ababa Bole International Airport
     ...
     [N] Next page
   ? Select airport (enter number)
   > 1
   OK: Addis Ababa (ADD) - Addis Ababa Bole International Airport
   ```
4. Displays the arrival airport selection page. The user enters `5` (Dubai - DXB).
5. Prompts: `Departure date (YYYY-MM-DD):`. User enters `2026-05-22`.
6. Prompts: `Search range in days? (1-365):`. User enters `2`.
7. Shows results:
   ```text
   Searching: ADD -> DXB
   Dates: 2026-05-22 to 2026-05-24
   ==============================================================================
   Search Results
   ==============================================================================
   1 flight found.
     1. FL-101 | ADD -> DXB | Depart 2026-05-22 11:17 | 450.00 USD | On Time
        Seats: Economy 120, Business 24, First 12
   Hint: Select option 2 from the main menu to book one of these flights.
   ```

#### 2. Developer Perspective & Call-Stack
1. `run_repl()` captures the input `"1"`, parses it via `try_parse_int()`, and dispatches to `handle_search_command()`.
2. `handle_search_command()` invokes:
   ```cpp
   SearchCriteria get_interactive_search()
   ```
   * *Returns*: `SearchCriteria` struct.
   * *Internal Logic*:
     - Calls `get_airport_from_user("departure")` to display the paginated airports registry using `list_all_airports()`, captures user input, and returns `"ADD"`.
     - Calls `get_airport_from_user("arrival")` using the same flow, returning `"DXB"`.
     - Calls `get_date_from_user("Departure date (YYYY-MM-DD): ")`. Prompts for date, parses characters safely, constructs `std::tm`, and calls `std::mktime()` to generate the departure start timestamp.
     - Calls `get_search_range_days()`. Prompts for days, validates range (1 to 365) via `try_parse_int()`, and returns `2`.
     - Calculates search window: `start = parsed_date`, `end = start + (2 * 24 * 3600)`.
     - Returns the compiled `SearchCriteria` struct.
3. `handle_search_command()` then calls:
   ```cpp
   FlightQueryResult search_flights(const SearchCriteria& criteria)
   ```
   * *Parameters*: `criteria` = `{origin = "ADD", destination = "DXB", date_window_start = start, date_window_end = end}`.
   * *Internal Logic*:
     - Filters `g_flights` to find flights departing from `"ADD"` to `"DXB"` within the start and end timestamp window.
     - Locates flight `"FL-101"` departing in 1 day.
     - Returns a `FlightQueryResult` containing a vector copy of `"FL-101"` and `make_success()`.
4. `handle_search_command()` clears `g_last_search_results` and saves the matched flight `"FL-101"` to the search cache.
5. Queries the flight's current seat counts by calling `get_inventory_snapshot("FL-101")` and displays the flight details and available seats to the user.

---

### Phase 3: Choice 2 - Flight Booking

#### 1. User Perspective
1. User enters `2` in the main menu:
   ```text
   airline> 2
   ```
2. System displays the flight found in the previous search:
   ```text
   Choose a flight by number or enter a flight ID:
   > 1
   ```
3. Lists available classes: `1. Economy, 2. Business, 3. First`. User enters `1`.
4. Prompts for passenger information:
   * First name: `John`
   * Last name: `Doe`
   * Passport: `EP123456`
   * Preferred seat: Presses Enter (auto-assign).
5. A booking receipt is generated in-memory and printed:
   ```text
   ==============================================================================
   Booking Confirmed
   ==============================================================================
   PNR:       PNR-X8A4D2
   Passenger: John Doe
   Flight:    FL-101
   Class:     Economy
   Seat:      E1
   Total:     450.00 USD
   Hint: Select option 3 from the main menu to start guided check-in.
   ```

#### 2. Developer Perspective & Call-Stack
1. `run_repl()` dispatches to `handle_book_command()`.
2. `handle_book_command()` verifies that `g_last_search_results` is not empty.
3. User selects option `1`. The code maps index `0` to flight `"FL-101"` from the cached search vector.
4. User selects choice `1` for class. Maps to `SeatClass::Economy`.
5. Prompts and captures traveler's name (`John Doe`) and passport number (`EP123456`).
6. Calls `is_valid_passport("EP123456")` which returns `true` (length is 8, all characters are uppercase alphanumeric).
7. Passenger inputs an empty seat preference (automated assignment).
8. Instantiates `BookingRequest` struct: `{flight_id = "FL-101", passenger = {first_name = "John", last_name = "Doe", passport_number = "EP123456"}, preferred_class = SeatClass::Economy, seat_number = ""}`.
9. Invokes the booking transaction:
   ```cpp
   BookingResult create_booking(const BookingRequest& request)
   ```
   * *Parameters*: `request` = populated booking request.
   * *Returns*: `BookingResult` struct.
   * *Internal Logic Flow*:
     - Calls `find_flight("FL-101")` to verify the flight exists.
     - Calls `check_availability("FL-101", SeatClass::Economy)` to verify seats:
       - Looks up `flight_id` in `g_inventory`.
       - Sees `economy_available = 120` (which is > 0).
       - Returns `make_success()`.
     - Calls `get_inventory_snapshot("FL-101")` to retrieve current counts.
     - Prepares inventory allocation: sets up `InventoryUpdate update` with `economy_delta = -1`.
     - Calls `update_inventory(update)`:
       - Verifies that applying the delta will not drop seats below 0.
       - Deducts seat: `economy_available = 119`.
       - Returns `make_success()`.
     - Calculates dynamic price:
       - Calls `calculate_dynamic_price({amount_cents = 45000, currency = "USD"}, remaining = 119, capacity = 120, status)`:
         - Computes ratio: `119 / 120 = 0.991`.
         - Ratio is higher than `0.25` demand thresholds, so the price multiplier is `1/1` (base price).
         - Returns price copy: `{amount_cents = 45000, currency = "USD"}`.
     - Allocates seat number:
       - Calls `generate_seat_number("FL-101", SeatClass::Economy)`:
         - Retrieves current seat counter for Economy. Returns seat ID `"E1"`.
     - Registers seat allocation:
       - Calls `reserve_seat("FL-101", "E1")`:
         - Checks if `"E1"` is occupied in the seat map (it is vacant).
         - Sets `occupied_seats["E1"] = true` in the seat map registry.
         - Returns `make_success()`.
     - Generates booking reference (PNR):
       - Calls `generate_pnr()`. Generates `"X8A4D2"`. Verifies there is no PNR collision in the registry.
     - Records system revenue:
       - Calls `record_revenue({amount_cents = 45000, currency = "USD"})`:
         - Adds `$450.00` to the global central ledger ledger balance: `g_recorded_revenue += 45000`.
         - Returns `make_success()`.
     - Registers the reservation:
       - Creates a `ReservationRecord` containing the PNR (`"X8A4D2"`), passenger info, total cost, and status (`ReservationStatus::Reserved`).
       - Inserts the record into the global `g_reservations` map.
     - Returns a `BookingResult` containing the PNR reference (`"X8A4D2"`), total cost (`$450.00`), and `make_success()`.
10. `handle_book_command()` displays the confirmation receipt to the screen.

---

### Phase 4: Choice 3 - Passenger Check-in

#### 1. User Perspective
1. User enters `3` in the main menu:
   ```text
   airline> 3
   ```
2. Displays reservations ready for check-in:
   ```text
   ==============================================================================
   Recent Bookings Ready for Check-in
   ==============================================================================
     1. PNR-X8A4D2 | John Doe | FL-101 | Reserved
   Choose a booking by number or enter a PNR:
   > 1
   ```
3. Prompts: `Checked bags (press Enter for 0):`. User enters `2`.
4. A boarding pass is generated and printed:
   ```text
   ==============================================================================
   Check-in Complete
   ==============================================================================
   PNR:            PNR-X8A4D2
   Passenger:      John Doe
   Flight:         FL-101
   Seat:           E1
   Gate:           G1
   Boarding group: 3
   Checked bags:   2
   ```

#### 2. Developer Perspective & Call-Stack
1. `run_repl()` dispatches control to `handle_checkin_command()`.
2. `handle_checkin_command()` calls `get_all_reservations()` to list reservations currently in a `"Reserved"` state.
3. User enters selection `1`, resolving the PNR to `"X8A4D2"`.
4. User enters baggage count `2`. Safely parses the input to `int` using `try_parse_int()`.
5. Invokes the check-in transaction:
   ```cpp
   CheckInResult process_check_in(const std::string& pnr_id, int baggage_count)
   ```
   * *Parameters*: `pnr_id` = `"X8A4D2"`, `baggage_count` = `2`.
   * *Returns*: `CheckInResult` struct.
   * *Internal Logic Flow*:
     - Calls `normalize_pnr_input("X8A4D2")` which returns `"X8A4D2"`.
     - Calls `is_valid_pnr("X8A4D2")` which returns `true` (length is 6, alphanumeric).
     - Calls `is_valid_baggage_count(2)` which returns `true` (within limits).
     - Calls `find_reservation("X8A4D2")` to look up the reservation record.
     - Verifies that reservation status is `ReservationStatus::Reserved`.
     - Generates boarding details:
       - Calls `gate_for("FL-101")`:
         - Extracts last character of flight ID (`'1'`).
         - Returns gate code `"G1"`.
       - Calls `boarding_group_for(SeatClass::Economy)`:
         - Returns boarding group `3`.
     - Calls `update_reservation_status("X8A4D2", ReservationStatus::CheckedIn)` to update reservation status:
       - Looks up `"X8A4D2"` in `g_reservations`.
       - Updates status to `CheckedIn`.
       - Returns `make_success()`.
     - Increments the check-ins count: `g_total_checkins = 1`.
     - Returns a `CheckInResult` initialized with the `BoardingPass` struct (`{.pnr_id = "X8A4D2", .gate = "G1", .boarding_group = 3}`), baggage count `2`, and `make_success()`.
6. `handle_checkin_command()` prints the boarding pass details to the terminal.

---

### Phase 5: Choice 4 - Flight Status Update

#### 1. User Perspective
1. User enters `4` in the main menu:
   ```text
   airline> 4
   ```
2. Displays all flights and their current statuses:
   ```text
     1. FL-101 | ADD -> DXB | Depart 2026-05-22 11:17 | On Time
     2. FL-102 | ADD -> LHR | Depart 2026-05-23 11:17 | On Time
     3. FL-201 | DXB -> ADD | Depart 2026-05-24 11:17 | On Time
   Choose a flight by number or enter a flight ID:
   > 1
   ```
3. Displays status options: `1. On Time, 2. Delayed, 3. Boarding, 4. Cancelled`. User enters `2`.
4. Prints confirmation:
   ```text
   ==============================================================================
   Flight Status Updated
   ==============================================================================
   Flight:     FL-101
   Route:      ADD -> DXB
   New status: Delayed
   ```

#### 2. Developer Perspective & Call-Stack
1. `run_repl()` dispatches control to `handle_status_command()`.
2. `handle_status_command()` calls `get_flight_registry()` and lists flights.
3. User selects choice `1`, resolving the flight ID to `"FL-101"`.
4. User selects choice `2` for status, mapping option index `2` to `"Delayed"`.
5. Invokes the status update process:
   ```cpp
   Status update_flight_status(const std::string& flight_id, const std::string& new_status)
   ```
   * *Parameters*: `flight_id` = `"FL-101"`, `new_status` = `"Delayed"`.
   * *Returns*: `Status` struct.
   * *Internal Logic Flow*:
     - Delegates the call to `set_flight_status("FL-101", "Delayed")`:
       - Calls the helper `canonicalize_status("Delayed")` which returns normalized `"Delayed"`.
       - Calls `find_flight_mutable("FL-101")` to locate the flight in `g_flights`.
       - Modifies the flight's status field: `flight_ptr->status = "Delayed"`.
       - Returns `make_success()`.
     - Returns `make_success()`.
6. `handle_status_command()` prints the status update confirmation to the user.

---

### Phase 6: Choice 5 - Operations & Financial Summary

#### 1. User Perspective
1. User enters `5` in the main menu:
   ```text
   airline> 5
   ```
2. Displays the operational and financial audit report:
   ```text
   ==============================================================================
   Revenue and Operations Report
   ==============================================================================
   Revenue audit: Passed
   Revenue total: 450.00 USD

   Operations summary:
     Reservations: 1, Checked-in: 1, Boarded: 0, Delayed flights: 1
   ```

#### 2. Developer Perspective & Call-Stack
1. `run_repl()` dispatches control to `handle_report_command()`.
2. `handle_report_command()` calls:
   * `get_booking_totals()` to retrieve individual booking costs. Returns a vector containing one item: `{amount_cents = 45000, currency = "USD"}`.
   * `get_recorded_revenue()` to retrieve ledger balance. Returns `45000`.
3. Invokes the financial audit:
   ```cpp
   RevenueAuditResult audit_revenue(const std::vector<Money>& booking_totals, long long recorded_total)
   ```
   * *Parameters*: `booking_totals` = vector of ticket costs, `recorded_total` = `45000`.
   * *Returns*: `RevenueAuditResult` struct.
   * *Internal Logic Flow*:
     - Initializes `computed = 0`.
     - Iterates through the list of ticket costs:
       - Verifies that currency code is set to `"USD"`.
       - Verifies that adding cents will not overflow bounds.
       - Adds cents: `computed += 45000`.
     - Compares calculated sum to recorded total: `45000 == 45000` (they match).
     - Returns a `RevenueAuditResult` initialized with `computed_total = 45000`, `recorded_total = 45000`, and `make_success()`.
4. Prints the audit result state `"Passed"` and total revenue `$450.00`.
5. Compiles operational statistics by querying:
   * `get_total_reservations()` which returns size of map (`1`).
   * `get_total_checkins()` which returns checked in count (`1`).
   * `get_total_boarded()` which returns boarded count (`0`).
   * `count_flights_with_status("Delayed")` which returns delayed flights count (`1`).
6. Generates the operational report:
   ```cpp
   OperationalReport generate_operational_report(int total_reservations, int total_checked_in, int total_boarded, int delayed_flights)
   ```
   * *Parameters*: `1`, `1`, `0`, `1`.
   * *Returns*: `OperationalReport` containing the formatted summary report text.
7. `handle_report_command()` prints the formatted statistics summary report to the terminal.

---

### Phase 7: Choice 7 - Exit the System

#### 1. User Perspective
1. User enters `7` in the main menu:
   ```text
   airline> 7
   Thank you for using the Airline Management System. Goodbye!
   ```
2. The CLI terminal process terminates and returns control to the shell prompt.

#### 2. Developer Perspective & Call-Stack
1. `run_repl()` captures choice `7`.
2. Prints the exit message.
3. Breaks out of the main interactive `while (true)` loop in `run_repl()`.
4. Control returns from `run_repl()` to the entry point `main()` in [main.cpp](file:///C:/Users/hi/comment/group-project/airline-management-codes/main.cpp).
5. `main()` executes `return 0;`, cleanly releasing all allocated heap and static structures (`g_flights`, `g_reservations`, `g_inventory`, `g_seat_maps`) and terminating the CLI process.
