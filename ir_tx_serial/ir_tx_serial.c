// Serial IR transmitter ;-)
// furrtek 2018
// ATTiny2313 10MHz 3.3V
// Uses UART RX/TX 57600bps 8N1, IR out on PB2 (OC0A)

// The serial protocol is:
// (byte) data size (in bytes, max 100)
// (byte) repeat count
// Data bytes 0bAABBCCDD as pair of bytes in the form 0bxxxxAABB 0bxxxxCCDD
// Transmission will be done right to left (DD to AA)
// Transmission begins as soon as the last data byte is received
// Once done, the MCU replies with character 'A'

// There's a ~50ms timeout/reset on the serial data
// So the program returns to idle state if the serial stream is interrupted

#define F_CPU 10000000

#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/interrupt.h>

volatile uint8_t do_tx = 0, put_index = 0, data_size = 0, repeat = 0, rxstate = 0;
volatile uint8_t buffer[100];

#define NOP __asm__ __volatile__ ("nop")

void tx() {
	uint8_t byte = 0;
	uint16_t r, s, sym_count;

	// Symbol count (1 byte / 2 bits per symbol for = 4 symbols per byte)
	sym_count = data_size << 2;

	for (r = 0; r < repeat; r++) {
		for (s = 0; s < sym_count; s++) {
			if ((s & 3) == 0)				// Load new byte
				byte = buffer[s >> 2];

			// Burst
			TCCR0A = 0b01000010;			// Timer 0 output ON
			_delay_us(39);
			NOP;
			NOP;
			NOP;
			NOP;
			NOP;
			NOP;
			NOP;
			NOP;
			TCCR0A = 0b00000010;			// Timer 0 output OFF
			PORTB = 0;
		
			// Symbol pause, timing is critical !
			switch(byte & 3) {
				case 0:
					_delay_us(56);
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					break;
				case 1:
					_delay_us(237);
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					break;
				case 2:
					_delay_us(117);
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					NOP;
					break;
				case 3:
					_delay_us(178);
					break;
			}

			byte >>= 2;			// Next symbol
		}

		// Final burst
		TCCR0A = 0b01000010;	// Timer 0 output ON
		_delay_us(39);
		NOP;
		NOP;
		NOP;
		NOP;
		NOP;
		NOP;
		NOP;
		NOP;
		TCCR0A = 0b00000010;	// Timer 0 output OFF
		PORTB = 0;
		_delay_us(2000);

	}

	// Send 'A' to indicate end of tx
	while (!(UCSRA & (1<<UDRE))) {};
	UDR = 'A';
	
	put_index = 0;
	data_size = 0;
	do_tx = 0;
}

ISR(TIMER1_COMPA_vect) {
	// Timer 1 OVF resets the buffer pointer
	if (!do_tx) {
		put_index = 0;
		data_size = 0;
	}
}

ISR(USART_RX_vect) {
	TCNT1 = 0;	// Reset timeout

	if (!data_size) {
		// First byte is data size (in bytes)
		data_size = UDR;
		rxstate = 0;
	} else {
		if (!rxstate) {
			// Second byte is repeat count (really transmit count, as 0 won't transmit)
			repeat = UDR;
			rxstate = 1;
		} else {
			if (put_index < 100) {
				buffer[put_index] = UDR;
				put_index++;
				// Trigger IR tx when all bytes are received
				if (put_index == data_size)
					do_tx = 1;
			}
		}
	}
}

int main(void) {
	MCUSR &= ~(1<<WDRF);			// Watchdog sleepies u_u
	WDTCSR = (1<<WDCE) | (1<<WDE);
	WDTCSR = 0x00;

	PORTD = 0b00000000;
	DDRD = 0b00000010;				// PD1 (TXD) as output
	PORTB = 0b00000000;
	DDRB = 0b00000100;				// PB2 (OCOA) as output

	UCSRB = 0b10011000;				// RX interrupt, RX and TX enable
	UCSRC = 0b00000110;
	UBRRH = 0;						// 57600 8N1
	UBRRL = 10;

	OCR0A = 3;						// Burst gen 10/2/(3+1) = 1.25MHz

	TCCR0A = 0b00000010;			// Timer 0 CTC mode, output OFF
	TCCR0B = 0b00000001;			// Timer 0 On

	OCR1A = 2000;					// ~50ms timeout
	TCCR1A = 0b00000000;
	TCCR1B = 0b00001100;			// Prescaler = 256
	TIMSK = 0b01000000;				// Timer 1 OCIE1A interrupt

	sei();

	for(;;) {
		if (do_tx) {
			cli();
			tx();
			TIFR = 0;				// Clear pending interrupts
			UCSRA = 0;
			sei();
		}
	}
}
