#include "ir.h"

volatile const uint8_t * FrameData;
volatile uint32_t ByteSentCounter, ByteCount;
volatile uint32_t SymbolCounter;
volatile uint32_t TickCounter, Burst, Repeats, ErrorAcc, burst_time;
volatile uint8_t IRTXBusy = 0, CurrentByte, Symbol, Protocol;
volatile uint32_t RepeatDelay = 0;
volatile const uint8_t * FrameData;

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
	// Force inactive level on GPIO and disable timer
	TIM16->CCMR1 &= (uint16_t)(~TIM_CCMR1_OC1M_0);
	TIM16->CR1 &= ~TIM_CR1_CEN;
	IRTXBusy = 0;
	RED_LED_NORMAL
}

void TIM16_IRQHandler(void) {
	if (IRTXBusy) {
		if (!TickCounter) {
			if (Burst == 0) {
				// Start burst ASAP
				TIM16->CCMR1 |= TIM_CCMR1_OC1M_0;	// Force active level
				TickCounter = burst_time;

				if (Protocol) {
					// PP16
					if (!(SymbolCounter & 1))
						CurrentByte = FrameData[ByteSentCounter++];	// Load byte

					// Load symbol
					Symbol = CurrentByte & 15;
					CurrentByte >>= 4;
					SymbolCounter++;
				} else {
					// PP4
					if (!(SymbolCounter & 3))
						CurrentByte = FrameData[ByteSentCounter++];	// Load byte

					// Load symbol
					Symbol = CurrentByte & 3;
					CurrentByte >>= 2;
					SymbolCounter++;
				}

				Burst = 1;
			} else {
				// Stop burst ASAP
				TIM16->CCMR1 &= (uint16_t)(~TIM_CCMR1_OC1M_0);	// Force inactive level

				Burst = 0;
				TickCounter = Protocol ? pp16_steps[Symbol] : pp4_steps[Symbol];
				// Auto-adjust symbol time depending on timing error accumulation and protocol time base
				// If accumulated timing error is over the protocol's time base, add a period to compensate
				if (ErrorAcc >= (Protocol ? 400 : 1000)) {
					TickCounter++;
					ErrorAcc -= 1000;
				}
				ErrorAcc += Protocol ? pp16_errors[Symbol] : pp4_errors[Symbol];

				if (ByteSentCounter > ByteCount) {
					if (Repeats) {
						Repeats--;
						ByteSentCounter = 0;
						SymbolCounter = 0;
						ErrorAcc = 0;
						TickCounter = RepeatDelay * (Protocol ? 125 : 50);	// 125*4 = 50*10 = 500us
					} else {
						IRStop();
						if (!remote_mode) CDC_OK
					}
				}
			}
		} else
			TickCounter--;
	}

	// Clear TIM_ENV update interrupt
	TIM16->SR &= (uint16_t)(~TIM_SR_UIF);
}

void IRTX(const uint8_t * data, const uint32_t pp16, const uint32_t length, const uint32_t rpt, const uint32_t delay) {
	FrameData = data;
	Protocol = pp16;
	ByteCount = length;
	Repeats = rpt;
	RepeatDelay = delay;

	ByteSentCounter = 0;
	TickCounter = 0;
	Burst = 0;
	SymbolCounter = 0;
	ErrorAcc = 0;
	IRTXBusy = 1;

	TIM16->CR1 &= ~TIM_CR1_CEN;		// Disable TIM16
	TIM16->CNT = 0;					// Reset TIM16's counter
	if (pp16) {
		TIM16->ARR = PP16_ARR;
		burst_time = PP16_BURST;
	} else {
		TIM16->ARR = PP4_ARR;
		burst_time = PP4_BURST;
	}

	if (remote_mode)
		RED_LED_WEAK
	else
		RED_LED_MAX

	TIM16->DIER |= TIM_DIER_UIE;	// Enable TIM16 update interrupt
	TIM16->CR1 |= TIM_CR1_CEN;		// Enable TIM16
}
