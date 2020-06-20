#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdint.h>

//// Panasonic Power
////0000 0070 0000 0064
////0080 003e
//0100 0000 0000 0100 0000 1001 0000 0000 1011 1100 1011 0101
//0x02      0x20      0x90      0x00      0x3D      (XOR of last three bytes)
//
////0010 0aa3
////0080 003e
//0100 0000 0000 0100 0000 1001 0000 0000 1011 1100 1011 0101
////0010 09d1

// CARRIER_COUNTER is how many ticks of TIMER2
#define CARRIER_COUNTER (0x1A)

#define ENABLE_CARRIER() do { \
    /* Enable TIMER1_COMPA_vect */ \
    TIMSK1 |= _BV(OCIE1A); \
} while (0)
#define DISABLE_CARRIER() do { \
    /* Disable TIMER1_COMPA_vect so that when the carrier does not get
     * restarted */ \
    TIMSK1 |= _BV(OCIE1A); \
} while (0)
#define START_CARRIER() do { \
    /* Clear OC2B (PD3) on compare match. Set it at counter bottom. */ \
    TCCR2A |= _BV(COM2B1); \
} while (0)
#define STOP_CARRIER() do { \
    /* Never set OC2B (PD3) */ \
    TCCR2A &= ~_BV(COM2B1); \
    /* Make sure PD3 (OC2B) is low */ \
    PORTD &= ~_BV(PD3); \
} while (0)

#define BURST(on, off) do { \
    /* B is when the carrier is deactivated */ \
    OCR1B = CARRIER_COUNTER * on; \
    /* A happens after and starts the next burst pair */ \
    OCR1A = CARRIER_COUNTER * (on + off); \
} while (0)

#define BURST_LEAD_IN()  BURST(0x80, 0x3E)
#define BURST_ONE()      BURST(0x10, 0x30)
#define BURST_ZERO()     BURST(0x10, 0x10)
#define BURST_LEAD_OUT() BURST(0x10, 0xFF)

#define CODE_LEN (6) // Panasonic uses a 6 byte code

enum button {
    POWER = 0x3D,
};

uint8_t code[CODE_LEN];
uint8_t * code_byte = code;
uint8_t code_bit;

void start_code(enum button button_code) {
    // Information from jonsfine at
    // http://www.remotecentral.com/cgi-bin/mboard/rc-pronto/thread.cgi?26152

    // Panasonic's Kaseikyo IR standard identifier
    code[0] = 0x02;
    code[1] = 0x20;
    // Device and subdevice like fields
    code[2] = 0x90;
    code[3] = 0x00;
    // Here is the actual code
    code[4] = button_code;
    // Checksum is XOR of the final 3 bytes
    code[5] = code[2] ^ code[3] ^ code[4];

    code_byte = code;
    code_bit = 8;

    BURST_LEAD_IN();
    ENABLE_CARRIER();
    START_CARRIER();
}

ISR (TIMER1_COMPB_vect)
{
    cli();

    // Will be reenabled when the next bit is processed in TIMER1_COMPA_vect
    STOP_CARRIER();

    // Bring PD0 low
    PORTD &= ~_BV(PD0);

    sei();
}

ISR (TIMER1_COMPA_vect)
{
    cli();

    START_CARRIER();

    if (!code_bit) {
        // Advance to the next byte in the code
        code_bit = 8;
        if (++code_byte == code + CODE_LEN) {
            // We are done.
            //BURST_LEAD_OUT();
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

    sei();
}

int main() {

    cli();

    // Set PD0 and PD3 as outputs
    // PD3 will be the carrier
    // PD0 will be toggled when carrier signal is toggled
    DDRD = _BV(PD0) | _BV(PD3);

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

    // Output Compare A Match & B Match Interrupt Enable
    TIMSK1 |= _BV(OCIE1A) | _BV(OCIE1B);

    start_code(POWER);

    sei();

    for(;;) ;
}
