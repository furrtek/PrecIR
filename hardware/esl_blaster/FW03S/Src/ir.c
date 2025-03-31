#include "ir.h"
#include "main.h"
#include "usbd_cdc_if.h"

static void symbol_pp4();
static void symbol_pp16();
static void symbol_raw();
static void symbol_nop();

volatile const uint8_t * FrameData;
volatile uint32_t ByteSentCounter, ByteCount;
volatile uint32_t SymbolCounter;
volatile uint32_t TickCounter, Burst, Repeats, ErrorAcc;
volatile uint8_t IRTXBusy = 0, CurrentByte, Symbol, Protocol;
volatile uint32_t RepeatDelay = 0;
volatile const uint8_t * FrameData;
volatile void (*symbol_func)(void) = symbol_nop;

// PP4 symbol times in TIM16@10us steps
// t=1/32768: 30.52us
// 00: 61us  (2t=61.03)		(TIM16 6 *10=60		Err=-1.03)
// 01: 244us (8t=244.14)	(TIM16 24 *10=240	Err=-4.14)
// 10: 122us (4t=122.07)	(TIM16 12 *10=120	Err=-2.07)
// 11: 183us (6t=183.10)	(TIM16 18 *10=180	Err=-3.10)
// Burst: 40us				(TIM16 4 *10=40		Err=0)
static const uint32_t pp4_steps[4] = {
	6-1, 24-1, 12-1, 18-1
};
static const uint32_t pp4_errors[4] = {
	103, 414, 207, 310		// All *100
};

// PP16 symbol times in TIM16@4us steps
// 0000: 27us 	(TIM16 7 *4=28	Err=1)
// 0001: 51us 	(TIM16 13*4=52	Err=1)
// 0010: 35us 	(TIM16 9 *4=36	Err=1)
// 0011: 43us 	(TIM16 11*4=44	Err=1)
// 0100: 147us 	(TIM16 37*4=148	Err=1)
// 0101: 123us 	(TIM16 31*4=124	Err=1)
// 0110: 139us 	(TIM16 35*4=140	Err=1)
// 0111: 131us 	(TIM16 33*4=132	Err=1)
// 0100: 83us 	(TIM16 21*4=84	Err=1)
// 0101: 59us 	(TIM16 15*4=60	Err=1)
// 0110: 75us 	(TIM16 19*4=76	Err=1)
// 0111: 67us 	(TIM16 17*4=68	Err=1)
// 0100: 91us 	(TIM16 23*4=92	Err=1)
// 0101: 115us 	(TIM16 29*4=116	Err=1)
// 0110: 99us 	(TIM16 25*4=100	Err=1)
// 0111: 107us 	(TIM16 27*4=108	Err=1)
// Burst: 21us	(TIM16 5 *4=20	Err=-1)
static const uint32_t pp16_steps[16] = {
	7-2-1, 13-2-1, 9-2-1, 11-2-1, 37-2-1, 31-2-1, 35-2-1, 33-2-1,
	21-2-1, 15-2-1, 19-2-1, 17-2-1, 23-2-1, 29-2-1, 25-2-1, 27-2-1
};
static const uint32_t pp16_errors[16] = {
	100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100		// All *100
};

#define PP4_ARR		484 - 1	// 10.08us
#define PP4_BURST	4 - 1
#define PP16_ARR	192 - 1	// 4us
#define PP16_BURST	5 - 1

void IRStop() {
	// Force inactive level on GPIO and disable symbol timer
	TIM16->CCMR1 &= (uint16_t)(~TIM_CCMR1_OC1M_0);
	TIM16->CR1 &= ~TIM_CR1_CEN;
	IRTXBusy = 0;
	RED_LED_NORMAL
}

static void symbol_pp4() {
	if (!TickCounter) {
		if (Burst == 0) {
			// Start burst ASAP
			TIM16->CCMR1 |= TIM_CCMR1_OC1M_0;	// Force active level
			TickCounter = PP4_BURST;

			// Prepare symbol to come while burst is being generated
			if (!(SymbolCounter & 3))
				CurrentByte = FrameData[ByteSentCounter++];	// Load byte every 4 symbols (2 bits)

			Symbol = CurrentByte & 3;	// Load symbol
			CurrentByte >>= 2;
			SymbolCounter++;

			Burst = 1;
		} else {
			// Stop burst ASAP
			TIM16->CCMR1 &= (uint16_t)(~TIM_CCMR1_OC1M_0);	// Force inactive level

			Burst = 0;
			TickCounter = pp4_steps[Symbol];
			// Auto-adjust symbol time depending on timing error accumulation and protocol time base
			// If accumulated timing error is over the protocol's time base, add a period to compensate
			if (ErrorAcc >= 1000) {
				TickCounter++;
				ErrorAcc -= 1000;
			}
			ErrorAcc += pp4_errors[Symbol];

			if (ByteSentCounter > ByteCount) {
				if (Repeats) {
					Repeats--;
					ByteSentCounter = 0;
					SymbolCounter = 0;
					ErrorAcc = 0;
					TickCounter = RepeatDelay * 50;		// ~500us
				} else {
					IRStop();
					if (!remote_mode) CDC_OK
				}
			}
		}
	} else
		TickCounter--;
}

static void symbol_pp16() {
	if (!TickCounter) {
		if (Burst == 0) {
			// Start burst ASAP
			TIM16->CCMR1 |= TIM_CCMR1_OC1M_0;	// Force active level
			TickCounter = PP16_BURST;

			// Prepare symbol to come while burst is being generated
			if (!(SymbolCounter & 1))
				CurrentByte = FrameData[ByteSentCounter++];	// Load byte every 2 symbols (4 bits)

			Symbol = CurrentByte & 15;	// Load symbol
			CurrentByte >>= 4;
			SymbolCounter++;

			Burst = 1;
		} else {
			// Stop burst ASAP
			TIM16->CCMR1 &= (uint16_t)(~TIM_CCMR1_OC1M_0);	// Force inactive level

			Burst = 0;
			TickCounter = pp16_steps[Symbol];
			// Auto-adjust symbol time depending on timing error accumulation and protocol time base
			// If accumulated timing error is over the protocol's time base, add a period to compensate
			if (ErrorAcc >= 400) {
				TickCounter++;
				ErrorAcc -= 1000;
			}
			ErrorAcc += pp16_errors[Symbol];

			if (ByteSentCounter > ByteCount) {
				if (Repeats) {
					Repeats--;
					ByteSentCounter = 0;
					SymbolCounter = 0;
					ErrorAcc = 0;
					TickCounter = RepeatDelay * 125;	// ~500us
				} else {
					IRStop();
					if (!remote_mode) CDC_OK
				}
			}
		}
	} else
		TickCounter--;
}

static void symbol_raw() {
	if (!(SymbolCounter & 7))
		CurrentByte = FrameData[ByteSentCounter++];	// Load byte every 8 symbols

	if (CurrentByte & 0x80)
		TIM16->CCMR1 |= TIM_CCMR1_OC1M_0;				// Force active level
	else
		TIM16->CCMR1 &= (uint16_t)(~TIM_CCMR1_OC1M_0);	// Force inactive level

	CurrentByte <<= 1;
	SymbolCounter++;

	if (ByteSentCounter > ByteCount) {
		if (Repeats) {
			Repeats--;
			ByteSentCounter = 0;
			SymbolCounter = 0;
			TickCounter = RepeatDelay * 10;	// 10 symbol durations
		} else {
			IRStop();
			if (!remote_mode) CDC_OK
		}
	}
}

static void symbol_nop() {
	IRStop();
	if (!remote_mode) CDC_OK
}

void IRTX(const uint8_t * data, const protocol_t protocol, const uint32_t length, const uint32_t rpt, const uint32_t delay) {
	Protocol = protocol;
	Repeats = rpt;
	RepeatDelay = delay;

	ByteSentCounter = 0;
	TickCounter = 0;
	Burst = 0;
	SymbolCounter = 0;
	ErrorAcc = 0;
	IRTXBusy = 1;

	// Configure carrier timer TIM17
	TIM17->CR1 &= ~TIM_CR1_CEN;			// Disable timer
	TIM17->PSC = 0;
	if ((Protocol == PROTOCOL_PP4) || (Protocol == PROTOCOL_PP16)) {
		TIM17->ARR = 38 - 1;				// 48M / 1.25M = 38.4, 38: 1.26MHz good enough
		TIM17->CCR1 = (uint16_t)(37 / 2);	// 50% duty cycle
	} else if (Protocol == PROTOCOL_RAW) {
		// In raw mode, the first two data bytes represent the carrier frequency
		uint16_t carrier = ((uint16_t)data[1] << 8) | data[0];
		if (!carrier)
			carrier = 38000;	// Default if zero
		uint32_t arr = (48000000UL / carrier) - 1;
		TIM17->ARR = arr;
		TIM17->CCR1 = (uint16_t)(arr >> 1);	// 50% duty cycle
	}
	TIM17->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;	// PWM mode 1
	TIM17->CCER |= TIM_CCER_CC1E;		// CC1 output enable
	TIM17->BDTR |= TIM_BDTR_MOE;
	TIM17->CR1 |= TIM_CR1_CEN;			// Enable timer

	// Configure symbol timer TIM16
	TIM16->CR1 &= ~TIM_CR1_CEN;		// Disable TIM16
	TIM16->CNT = 0;					// Reset TIM16's counter
	if (Protocol == PROTOCOL_PP4) {
		TIM16->ARR = PP4_ARR;
		symbol_func = symbol_pp4;
		FrameData = data;
		ByteCount = length;
	} else if (Protocol == PROTOCOL_PP16) {
		TIM16->ARR = PP16_ARR;
		symbol_func = symbol_pp16;
		FrameData = data;
		ByteCount = length;
	} else if (Protocol == PROTOCOL_RAW) {
		// In raw mode, the second two data bytes represent the baud rate
		uint16_t baudrate = ((uint16_t)data[3] << 8) | data[2];
		if (!baudrate)
			baudrate = 2400;	// Default if zero
		uint32_t arr = (48000000UL / baudrate) - 1;
		TIM16->ARR = arr;
		symbol_func = symbol_raw;
		FrameData = data + 4;
		ByteCount = length - 4;
	} else {
		symbol_func = symbol_nop;	// Just in case
	}

	if (remote_mode)
		RED_LED_WEAK
	else
		RED_LED_MAX

	TIM16->DIER |= TIM_DIER_UIE;	// Enable TIM16 update interrupt
	TIM16->CR1 |= TIM_CR1_CEN;		// Enable TIM16
}
