/*
Worku Wondoson
ETS1459/17
*/

#include "controller.h"

// Program entry point.
// initialize_system sets up flights and loads any saved data.
// run_repl starts the interactive command-line menu loop.
int main() {
    initialize_system();
    run_repl();
    return 0;
}
