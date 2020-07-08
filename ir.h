#ifndef __IR_H__
#define __IR_H__
#include "buttons.h"

// CARRIER_COUNTER is how many ticks of TIMER2 for the width of each IR pulse
// (ie half the carrier frequency period)
#define CARRIER_COUNTER (0x1A)

#define START_CARRIER() do { \
    TCNT2 = 0x0000; \
    /* Clear OC2B (PD3) on compare match. Set it at counter bottom. */ \
    TCCR2A |= _BV(COM2B1); \
} while (0)
#define STOP_CARRIER() do { \
    /* Stop toggling OC2B (PD3) */ \
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

void send_code(enum button button); // Blocks until code has been sent
void init_ir();

#endif // __IR_H__
