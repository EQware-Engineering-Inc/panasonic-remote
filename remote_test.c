#include<avr/io.h>
#include<avr/interrupt.h>

ISR (TIMER1_COMPA_vect)
{
    cli();

    // Toggle the set and clear of OC2B (PD3) on timer2 match
    TCCR2A ^= _BV(COM2B1);
    // Make sure PD3 is low
    PORTD &= ~_BV(PD3);

    // Toggle PD0
    PORTD ^= _BV(PD0);

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

    // Clear OC2B (PD3) on compare match. Set it at counter bottom.
    TCCR2A |= _BV(COM2B1);

    // Fast PWM. Top is OCRA
    TCCR2A |= _BV(WGM21) | _BV(WGM20);
    TCCR2B |= _BV(WGM22);

    TCCR2B |= _BV(CS20); // No clock prescaling

    // Counter will run from 0 to 27 and will toggle the pin off at 13
    // The main clock is 1MHz so: 1MHz / 27 = 37KHz (the Panasonic carier
    // frequency)
    OCR2A = 27;
    OCR2B = 13;

    // Timer Counter 1 Control Registers
    TCCR1A = 0x00;
    TCCR1B = 0x00;
    TIMSK1 = 0x00; // Interrupt Mask Register

    // CTC (Clear Timer on Compare match) with OCR1A as the top
    TCCR1B |= _BV(WGM12);

    TCCR1B |= _BV(CS10); // No clock prescaling

    TIMSK1 |= _BV(OCIE1A); // Output Compare A Match Interrupt Enable

    // Counter will run from 0 to 8192. The main clock is 1MHz so:
    // 1MHz / 8192 = 122Hz
    OCR1A = 8192;

    sei();        // Enable global interrupts by setting global interrupt enable bit in SREG

    for(;;) ;
}
