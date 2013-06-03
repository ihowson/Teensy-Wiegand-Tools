/* Keyboard example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_keyboard.html
 * Copyright (c) 2008 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_keyboard.h"

#define LED_CONFIG    (DDRD |= (1<<6))
#define LED_ON        (PORTD &= ~(1<<6))
#define LED_OFF        (PORTD |= (1<<6))
#define CPU_PRESCALE(n)    (CLKPR = 0x80, CLKPR = (n))

uint8_t number_keys[10]=
    {KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9};

uint16_t idle_count=0;

#define MAX_BUFLEN 255
uint8_t buf[MAX_BUFLEN];
uint8_t buflen = 0;

uint8_t d0() {
	return PINB & 0x01;
}

uint8_t d1() {
	return PINB & 0x02;
}

int main(void)
{
    uint8_t b, d, mask, i, reset_idle;
    uint8_t b_prev=0xFF, d_prev=0xFF;

    // set for 16 MHz clock
    CPU_PRESCALE(0);

    // Configure all port B and port D pins as inputs with pullup resistors.
    // See the "Using I/O Pins" page for details.
    // http://www.pjrc.com/teensy/pins.html
    DDRD = 0x00;
    DDRB = 0x00;
    PORTB = 0xFF;
    PORTD = 0xFF;

    // Initialize the USB, and then wait for the host to set configuration.
    // If the Teensy is powered without a PC connected to the USB port,
    // this will wait forever.
    usb_init();
    while (!usb_configured()) /* wait */ ;

    // Wait an extra second for the PC's operating system to load drivers
    // and do whatever it does to actually be ready for input
    _delay_ms(1000);

    // Configure timer 0 to generate a timer overflow interrupt every
    // 256*1024 clock cycles, or approx 61 Hz when using 16 MHz clock
    // This demonstrates how to use interrupts to implement a simple
    // inactivity timeout.
    TCCR0A = 0x00;
    TCCR0B = 0x05;
    TIMSK0 = (1<<TOIE0);

    while (1) {
        if (buflen == MAX_BUFLEN)
            continue; // spin until we flush the buffer

        if (d0() == 0) {
            buf[buflen] = 0;
            buflen++;
            idle_count = 0;

            while (d0() == 0) {
                idle_count = 0;
            } // spin until it goes low
        } else if (d1() == 0) {
            buf[buflen] = 1;
            buflen++;
            idle_count = 0;

            while (d1() == 0) {
            	idle_count = 0;
            } // spin until it goes low
        }
    }
}

void press_digit(uint8_t digit) {
	if (digit == 0)
		usb_keyboard_press(KEY_0, 0);
	else
		usb_keyboard_press(KEY_1 + digit - 1, 0);
}

void typedec(uint8_t val) {
	uint8_t r; // remainder

	r = val / 100;
	press_digit(r);
	val -= r * 100;

	r = val / 10;
	press_digit(r);
	val -= r * 10;

	press_digit(r);
    usb_keyboard_press(KEY_ENTER, 0);
}

// This interrupt routine is run approx 61 times per second.
// A very simple inactivity timeout is implemented, where we
// will send a space character.
ISR(TIMER0_OVF_vect)
{
    idle_count++;
    if (idle_count > 61 * 2) {
        idle_count = 0;

        if (buflen)
        {
            // probably no more bits coming, send the buffer
            int i;
            for (i = 0; i < buflen; i++) {
                if (buf[i] == 1)
                    usb_keyboard_press(KEY_1, 0);
                else
                    usb_keyboard_press(KEY_0, 0);
            }
            //usb_keyboard_press(KEY_ENTER, 0);
            usb_keyboard_press(KEY_SPACE, 0);

            // attempt to parse it
            if (buflen == 26) {
            	uint8_t digitvalue = 128;
            	uint8_t facilitycode = 0;
            	for (i = 1; i < 9; i++) {
            		if (buf[i] == 1) {
            			facilitycode += digitvalue;
            		}
            		digitvalue >>= 1;
            		typedec(digitvalue);
            	}

            	typedec(facilitycode);
            	// FIXME: this reports a different number to what you calculate

            } else {
            	// can't interpret
            	usb_keyboard_press(KEY_L, 0);
            	typedec(buflen);
            }
        }

        // start again
        buflen = 0;
    }
}


