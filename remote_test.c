#include<avr/io.h>
#include<avr/interrupt.h>

ISR (TIMER1_COMPA_vect)    // Timer1 ISR
{
    cli();

    TCNT2 = 0;

    TCCR2A ^= _BV(COM2B1);
    PORTD &= ~_BV(PD3);

    PORTD ^= _BV(PD0);

    sei();
}

int main() {
    DDRD = _BV(PD0) | _BV(PD3);
    TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(WGM22) | _BV(CS20);
    OCR2A = 26;
    OCR2B = 13;

    TCCR1A = 0x00;
    TCCR1B = _BV(WGM12) | _BV(CS10);
    TIMSK1 = _BV(OCIE1A);
    OCR1A = 1024 * 8;

    sei();        // Enable global interrupts by setting global interrupt enable bit in SREG

    for(;;) ;
}
