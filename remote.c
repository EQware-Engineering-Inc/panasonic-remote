#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "ir.h"
#include "buttons.h"

/*
 * --- PINMUX ------------------------------------------------------------------
 *  Pin      | Func    | Usage
 * ----------+---------+--------------------------------------------------------
 *   1 (PC6) | RESET   | Use a pullup to prevent reset
 *   4 (PD2) | GPIO    | Output for button matrix
 *   5 (PD3) | OC2B    | Output to activate IR LED
 *  11 (PD5) | GPIO    | Output for button matrix
 *  12 (PD6) | GPIO    | Output for button matrix
 *  13 (PD7) | GPIO    | Output for button matrix
 *  14 (PB0) | CLKO    | Normally disabled unless compiled with TUNE_CLOCK
 *  26 (PC5) | PCINT13 | Input for button matrix
 *  27 (PC4) | PCINT12 | Input for button matrix
 *  28 (PC3) | PCINT11 | Input for button matrix
 *  29 (PC3) | PCINT10 | Input for button matrix
 *  30 (PC1) | PCINT9  | Input for button matrix
 *  31 (PC0) | PCINT8  | Input for button matrix
 *
 * Inputs for the button matrix are pulled-up. The outputs are held low. When a
 * button is pressed, the output is connected to the input and thus the input
 * is pulled low. For button matrix, see buttons.c
 */

int main() {

#ifdef TUNE_CLOCK
    // Just do nothing. We are tuning the clock
    for (;;) ;
#endif

    // Disable the ADC
    ADCSRA = 0;

    init_ir();
    init_buttons();

    while (1) {
        cli();
        enable_button_interrupt();

        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        // No race condition because this will not go into effect until the
        // next instruction
        sei();
        sleep_cpu();

        // We are awake again and returned from the button PC ISR. Let's see
        // what the user wants us to do shall we...
        if (current_button != BTN_NA) {
            send_code(current_button);
        } else {
            // This shouldn't happen. If it does, maybe the user only grazed
            // the button, but the press wasn't long enough for the CPU to pick
            // it up.
        }

        // Make sure that the button that woke us (if we could determine it) is
        // no longer pressed
        wait_for_unpress();
    }
}
