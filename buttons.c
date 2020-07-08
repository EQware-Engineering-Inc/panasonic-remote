#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>

#include "buttons.h"

static const uint8_t matrix_out[] = {PD2, PD5, PD6, PD7};
#define MATRIX_OUT_BITS (_BV(PD2) | _BV(PD5) | _BV(PD6) | _BV(PD7))
static const uint8_t matrix_in[] = {PC5, PC4, PC3, PC2, PC1, PC0};
#define PCICR_ENABLE _BV(PCIE1)
#define PCIFR_CLEAR _BV(PCIF1)

static const enum button matrix[sizeof(matrix_in)][sizeof(matrix_out)] = {
 /*               4 (PD2)  , 11 (PD5) , 12 (PD6) , 13 (PD7) */
 /* 26 (PC5) */ { POWER    , EJECT    , STOP     , PLAY     },
 /* 27 (PC4) */ { PAUSE    , VSS      , FF       , RW       },
 /* 28 (PC3) */ { FORWARD  , BACKWARD , MENU     , UP       },
 /* 29 (PC2) */ { DOWN     , LEFT     , RIGHT    , ENTER    },
 /* 30 (PC1) */ { SUBTITLE , ACTION   , TITLE    , BTN_NA   },
 /* 31 (PC0) */ { BTN_NA   , BTN_NA   , BTN_NA   , BTN_NA   },
};

static volatile int16_t current_out_pin = -1;
static volatile int16_t current_in_pin = -1;
volatile enum button current_button = BTN_NA;

ISR (PCINT1_vect)
{
    sleep_disable();

    // We invert PINC because buttons pull us low
    uint8_t npinc = ~PINC;
    for (int in = 0; in < sizeof(matrix_in); in++) {
        if (npinc & _BV(matrix_in[in])) {
            // We have found the input pin which triggered the interrupt
            // Now we have to find which output pin
            PORTD |= MATRIX_OUT_BITS;
            // Toggle them on one at a time
            for (int out = 0; out < sizeof(matrix_out); out++) {
                PORTD &= ~_BV(matrix_out[out]);
                __asm__ __volatile__ ("nop");
                npinc = ~PINC;
                PORTD |= _BV(matrix_out[out]);
                if (npinc & _BV(matrix_in[in])) {
                    // We now have the proper in, out pair
                    current_button = matrix[in][out];
                    current_out_pin = out;
                    current_in_pin = in;
                    break;
                }
            }
            break;
        }
    }

    // Disable button press interrupts until wait_for_unpress is called
    PCICR &= ~PCICR_ENABLE;
}

void enable_button_interrupt()
{
    PORTD &= ~MATRIX_OUT_BITS;
    PCIFR |= PCIFR_CLEAR;
    PCICR |= PCICR_ENABLE;
}

void wait_for_unpress()
{
    if (current_out_pin != -1) {
        PORTD &= ~_BV(matrix_out[current_out_pin]);
        __asm__ __volatile__ ("nop");
        // Wait for input pin to go high again (i.e. the button is no longer
        // pulling us low)
        while (~PINC & _BV(matrix_in[current_in_pin])) {
            // XXX As a possible improvement, we could use the PC ISR to sleep
            // while waiting for this change. Not a big benefit though.
        }
        PORTD |= _BV(matrix_out[current_out_pin]);
    } else {
        // We have no memory of this button you speak of...
    }
    current_in_pin = current_out_pin = -1;
}

void init_buttons()
{
    // Set all the button matrix outputs and bring them low
    PORTD &= ~MATRIX_OUT_BITS;
    DDRD |= MATRIX_OUT_BITS;

    // Set all the button matrix inputs with pullups
    MCUCR &= ~_BV(PUD);
    PORTC |= _BV(PCINT13) | _BV(PCINT12) | _BV(PCINT11) | _BV(PCINT10) |
             _BV(PCINT9) | _BV(PCINT8);
    PCMSK1 |= _BV(PCINT13) | _BV(PCINT12) | _BV(PCINT11) | _BV(PCINT10) |
              _BV(PCINT9) | _BV(PCINT8);
}
