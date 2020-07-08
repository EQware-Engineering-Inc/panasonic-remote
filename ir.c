#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "ir.h"

static volatile uint8_t code[CODE_LEN];
static volatile uint8_t * code_byte = code;
static volatile uint8_t code_bit;

ISR (TIMER1_COMPB_vect)
{
    // Will be reenabled when the next bit is processed in TIMER1_COMPA_vect
    STOP_CARRIER();

    // Bring PD0 low
    PORTD &= ~_BV(PD0);
}

ISR (TIMER1_COMPA_vect)
{
    START_CARRIER();

    if (!code_bit) {
        // Advance to the next byte in the code
        code_bit = 8;
        if (++code_byte == code + CODE_LEN) {
            // We are done.
            BURST_LEAD_OUT();
            // Disable this interrupt so that we don't reenable the carrier
            TIMSK1 &= ~_BV(OCIE1A);
            goto timer1_compa_done;
        }
    }

    if (0x01 & (*code_byte)) {
        BURST_ONE();
    } else {
        BURST_ZERO();
    }

    // Advance to the next bit in the code
    --code_bit;
    *code_byte >>= 1;

timer1_compa_done:

    // Bring PD0 high
    PORTD |= _BV(PD0);
}

void send_code(enum button button)
{
    // Code information from jonsfine at
    // http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?26152

    // Panasonic's Kaseikyo IR standard identifier
    code[0] = 0x02;
    code[1] = 0x20;
    // Device and subdevice like fields
    code[2] = 0x90;
    code[3] = 0x00;
    // Here is the actual code
    code[4] = button;
    // Checksum is XOR of the final 3 bytes
    code[5] = code[2] ^ code[3] ^ code[4];

    code_byte = code;
    code_bit = 8;

    cli(); // Let's not have anything funny happen

    BURST_LEAD_IN(); // We start with the lead-in burst
    START_CARRIER();
    TCNT1 = 0x0000; // Start the countdown

    //Enable TIMER1_COMPA_vect. This is what advances through the bits in the
    //code.
    TIMSK1 |= _BV(OCIE1A);

    sei();

    // Wait for the code to be consumed
    //while (code_byte < code + CODE_LEN) ;
    while (TIMSK1 & _BV(OCIE1A)) ;
}

void init_ir()
{
    // Set PD3 as output for the carrier
    DDRD |= _BV(PD3);

    // Timer Counter 2 Control Registers
    TCCR2A = 0;
    TCCR2B = 0;

    // Fast PWM. Top is OCRA
    TCCR2A |= _BV(WGM21) | _BV(WGM20);
    TCCR2B |= _BV(WGM22);

    TCCR2B |= _BV(CS20); // No clock prescaling

    // Counter will run from 0 to 27 and will toggle the pin off at 13
    // The main clock is 1MHz so: 1MHz / 27 = 37KHz (the Panasonic carier
    // frequency)
    OCR2A = CARRIER_COUNTER;
    OCR2B = CARRIER_COUNTER / 2;

    // Timer Counter 1 Control Registers
    TCCR1A = 0x00;
    TCCR1B = 0x00;
    TIMSK1 = 0x00; // Interrupt Mask Register

    // CTC (Clear Timer on Compare match) with OCR1A as the top
    TCCR1B |= _BV(WGM12);

    TCCR1B |= _BV(CS10); // No clock prescaling

    // Output Compare B Match Interrupt Enable
    // We will enable the A Match Interrupt Enable later. We don't want to do
    // it now because we don't have anything loaded in the code array and have
    // nothing to send.
    // Enabling B Match now is harmless as it only disables the carrier signal.
    TIMSK1 |= _BV(OCIE1B);
}

