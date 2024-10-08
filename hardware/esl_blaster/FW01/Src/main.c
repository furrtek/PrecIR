// ESL Blaster firmware v1.00
// For board rev. B
// 2019 furrtek
// DO NOT COMPILE WITH -O3 ! Only -O1, or USB won't work anymore

// TODO: Load custom symbol lengths and IR timer settings ?

// Bloated stuff:
// HAL_GPIO_Init 0x1b4
// HAL_PCD_Init 0x240
// HAL_PCD_EP_Open 0x35c
// HAL_PCD_IRQHandler 0x5cc
// USBD_StdDevReq 0x338

#include "main.h"
#include "usb_device.h"
#include "gpio.h"

void SystemClock_Config(void);
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

volatile FIFO RX_FIFO = {.head=0, .tail=0};

volatile uint32_t ByteSentCounter, ByteCount;
volatile uint32_t SymbolCounter;
volatile uint32_t TickCounter, Burst, Repeats, ErrorAcc;
volatile uint8_t SendOperationReady = 0, CurrentByte, Symbol;
volatile const uint8_t * FrameData;
volatile uint32_t comm_reset_flag = 0, ram_frame_repeat_delay = 0;
volatile uint32_t remote_mode = 0;

typedef struct {
	uint16_t flags;
	uint16_t repeats;
	uint16_t delay;
	uint16_t frame_size;
	uint8_t frame_data[64];
} entry_t;

typedef struct {
	uint16_t entry_count;	// 0~8
	entry_t entries[8];
} userdata_t;

#define RED_LED_OFF TIM3->CCR1 = 0x0000;
#define RED_LED_NORMAL TIM3->CCR1 = (GPIOA->IDR & GPIO_IDR_4) ? 0x0000 : 0x0100;	// Off/Charge
#define RED_LED_MAX TIM3->CCR1 = 0x4000;	// IR transmit

#define CDC_NOK CDC_Transmit_FS((uint8_t*)"N", 1);
#define CDC_OK CDC_Transmit_FS((uint8_t*)"K", 1);

/*
static const uint8_t frame_button1[11] = {
	0x84, 0x00, 0x00, 0x00, 0x00, 0xAB, 0x09, 0x00, 0x00, 0xF2, 0xA7
};

static const uint8_t frame_dmchange1[11] = {
	0x85, 0x00, 0x00, 0x00, 0x00, 0x06, 0x09, 0x00, 0x00, 0xBD, 0xC3
};

static const uint8_t frame_dmchange2[11] = {
	0x85, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x00, 0x00, 0xEA, 0x80
};*/

// PP4 symbol times in TIM16 steps
static const uint32_t symbol_times[4] = {
	6-1, 24-1, 12-1, 18-1
};
static const uint32_t symbol_error[4] = {
	103, 414, 207, 310		// All *100
};

void TIM16_IRQHandler(void) {
	if (SendOperationReady) {
		if (!TickCounter) {
			if (Burst == 0) {
				// Start burst ASAP
				TIM16->CCMR1 |= TIM_CCMR1_OC1M_0;	// OC1REF forced high
				TickCounter = 4-1;

				if (!(SymbolCounter & 3))
					CurrentByte = FrameData[ByteSentCounter++];	// Load byte

				// Load symbol
				Symbol = CurrentByte & 3;
				CurrentByte >>= 2;
				SymbolCounter++;

				Burst = 1;
			} else {
				// Stop burst ASAP
				TIM16->CCMR1 &= (uint16_t)(~TIM_CCMR1_OC1M_0);	// OC1REF forced low

				Burst = 0;

				TickCounter = symbol_times[Symbol];
				// Auto-adjust symbol time depending on timing error accumulation
				if (ErrorAcc >= 1000) {
					TickCounter++;
					ErrorAcc -= 1000;
				}
				ErrorAcc += symbol_error[Symbol];

				if (ByteSentCounter > ByteCount) {
					if (Repeats) {
						Repeats--;
						ByteSentCounter = 0;
						SymbolCounter = 0;
						ErrorAcc = 0;
						TickCounter = ram_frame_repeat_delay * 50;	//2000;
					} else {
						SendOperationReady = 0;
						RED_LED_NORMAL
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

void TIM14_IRQHandler(void) {
	comm_reset_flag = 1;

	// Clear TIM_ENV update interrupt
	TIM14->SR &= (uint16_t)(~TIM_SR_UIF);
}

// Check MCP73831 STAT output
// Low: Charging
// High: Charge done
// Hi-z: No battery

void IRTX(const uint8_t * data, const uint32_t length, const uint32_t rpt) {
	FrameData = data;
	ByteSentCounter = 0;
	ByteCount = length;
	TickCounter = 0;
	Burst = 0;
	SymbolCounter = 0;
	Repeats = rpt;
	ErrorAcc = 0;
	SendOperationReady = 1;

	// RED LED ON
	RED_LED_MAX

	TIM16->DIER |= TIM_DIER_UIE;	// Enable TIM16 update interrupt
	TIM16->CR1 |= TIM_CR1_CEN;		// Enable all TIM16 interrupts
}

uint32_t adc_read() {
	ADC1->CR = ADC_CR_ADEN;
	while (!(ADC1->ISR & ADC_ISR_ADRDY));	// Wait for ADRDY
	ADC1->CR |= ADC_CR_ADSTART;
	while (!(ADC1->ISR & ADC_ISR_EOC));		// Wait for EOC flag
	return ADC1->DR;
}

userdata_t userdata __attribute__((section(".flash_data_array")));

uint8_t flash_data_buffer[1024];

int main(void) {
	enum comm_states {
		STATE_IDLE,
		STATE_GET_FRAME_SIZE,
		STATE_GET_FRAME_REPEAT_DELAY,
		STATE_GET_FRAME_REPEATS_LOW,
		STATE_GET_FRAME_REPEATS_HIGH,
		STATE_GET_FRAME_DATA,
		STATE_FLASH_LOAD
	};

	enum comm_states comm_state = STATE_IDLE;
	uint8_t ram_frame_data[256];
	uint32_t ram_frame_size = 0, ram_frame_repeats = 0, ram_frame_data_counter = 0;
	uint32_t flash_data_counter = 0;

	// Reset all peripherals, initializes the Flash interface and the Systick
	HAL_Init();

	// Configure the system clock
	SystemClock_Config();

	// Initialize all configured peripherals
	MX_GPIO_Init();
	MX_USB_DEVICE_Init();

	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM14);
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_ADC1);

	// TIM17: Carrier, TIM16: Envelope
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM16);	// These can be ORed
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM17);
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);

	GPIO_InitTypeDef GPIO_InitStruct;
	// Configure PA13 as push-pull output for IR_OUT
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate = GPIO_AF1_IR;	// IR_OUT
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// t=1/32768: 30.52us
	// 00: 61us  (2t=61.03)		(TIM16 6 *10=60		Err=-1.03)
	// 01: 244us (8t=244.14)	(TIM16 24 *10=240	Err=-4.14)
	// 10: 122us (4t=122.07)	(TIM16 12 *10=120	Err=-2.07)
	// 11: 183us (6t=183.10)	(TIM16 18 *10=180	Err=-3.10)
	// Burst: 40us				(TIM16 4 *10=40	Err=0)

	// TIM17: Carrier
	TIM17->PSC = 0;
	TIM17->ARR = 38 - 1;	// 48M / 1.25M = 38.4, 38: 1.26MHz
	TIM17->CCR1 = (uint16_t)(37 / 2);	// 50% duty cycle
	TIM17->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;	// PWM mode 1
	TIM17->CCER |= TIM_CCER_CC1E;	// CC1 output enable
	TIM17->BDTR |= TIM_BDTR_MOE;

	// TIM16: Symbol timing
	TIM16->PSC = 0;
	TIM16->ARR = 484 - 1;	// 48M / 100k = 480 (10us steps)
	TIM16->CCMR1 |= TIM_CCMR1_OC1M_2;	// Output forced low
	TIM16->CCER |= TIM_CCER_CC1E;		// CC1 output enable
	TIM16->BDTR |= TIM_BDTR_MOE;
	TIM16->DIER |= TIM_DIER_UIE;		// Update interrupt enable

	TIM17->CR1 |= TIM_CR1_CEN;			// Enable timer
	TIM17->EGR |= TIM_EGR_UG;

	NVIC_EnableIRQ(TIM16_IRQn);
	NVIC_SetPriority(TIM16_IRQn, 0);

	// TIM14: Comm timeout
	TIM14->PSC = (uint16_t)(100);		// 48M/100/65535 = ~7.32Hz (137ms)
	TIM14->ARR = (uint16_t)(0xFFFF);
	TIM14->CCMR1 = 0;
	TIM14->CCER = 0;
	TIM14->DIER |= TIM_DIER_UIE;		// Update interrupt enable
	TIM14->CR1 |= TIM_CR1_CEN;			// Enable timer
	TIM14->EGR |= TIM_EGR_UG;

	NVIC_EnableIRQ(TIM14_IRQn);
	NVIC_SetPriority(TIM14_IRQn, 1);

	// TIM3: Red LED PWM
	TIM3->ARR = (uint16_t)(0xFFFF);
	TIM3->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0;	// PWM mode 2
	TIM3->CCER = TIM_CCER_CC1E;
	TIM3->CCR1 = 0x1000;
	TIM3->CR1 |= TIM_CR1_CEN;			// Enable timer
	TIM3->EGR |= TIM_EGR_UG;

	ADC1->CFGR1 = 0;
	ADC1->SMPR = ADC_SMPR_SMP_2 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_0;
	ADC1->CHSELR = ADC_CHSELR_CHSEL5;

	// ADC step = 3.3/(2^12-1) = ~806uV
	// USB: 5V
	// Batt full charge: 4.2V
	// 5/2 = 2.5V -> v = ~3102
	// 4.2/2 = 2.1V -> v = ~2605
	// (3102+2605)/2 = 2854

	if (adc_read() < 2700) {
		uint32_t remote_frame_count = userdata.entry_count;
		uint32_t remote_frame_index = 0;

		remote_mode = 1;

		for (;;) {
			//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);	// Battery mode
			/*RED_LED_MAX
			for (uint32_t c = 0; c < 1000000; c++) {};
			RED_LED_OFF
			for (uint32_t c = 0; c < 1000000; c++) {};*/

			/*if (userdata.entries[1].frame_data[0] == 0x85) {
				for (;;) { RED_LED_OFF };
			}*/

			ram_frame_size = userdata.entries[remote_frame_index].frame_size;
			ram_frame_repeats = userdata.entries[remote_frame_index].repeats;
			ram_frame_repeat_delay = userdata.entries[remote_frame_index].delay;
			for (uint32_t c = 0; c < ram_frame_size; c++)
				ram_frame_data[c] = userdata.entries[remote_frame_index].frame_data[c];

			IRTX(ram_frame_data, ram_frame_size, ram_frame_repeats);

			while (SendOperationReady) {};

			remote_frame_index++;
			if (remote_frame_index >= remote_frame_count) remote_frame_index = 0;
		}
	}

	// Main loop
	while (1) {

		if (comm_reset_flag) {
			comm_reset_flag = 0;
			comm_state = STATE_IDLE;
			if (!SendOperationReady) RED_LED_NORMAL	// Update LED
			//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6); // DEBUG
		}

		while (RX_FIFO.head != RX_FIFO.tail) {
			TIM14->CNT = (uint16_t)0;	// Reset comm. timeout

			// Process one byte from the RX FIFO
			uint8_t byte = RX_FIFO.data[RX_FIFO.tail];

			if (comm_state == STATE_IDLE) {
				if (byte == 'L') {
					// Load new frame data
					comm_state = STATE_GET_FRAME_SIZE;
				} else if (byte == 'T') {
					// Transmit loaded frame
					IRTX(ram_frame_data, ram_frame_size, ram_frame_repeats);
				} else if (byte == '?') {
					// Reply ID
					CDC_Transmit_FS((uint8_t*)"ESLBlasterB0", 12);
				} else if (byte == 'W') {
					flash_data_counter = 0;
					comm_state = STATE_FLASH_LOAD;
				} else if (byte == 'R') {
					// Read flash
					uint16_t buff;
					uint16_t* ptr = (uint16_t*)0x8007C00;
					for (uint32_t d = 0; d < (1024/4); d++) {
						buff = *ptr;
						while(CDC_Transmit_FS((uint8_t*)&buff, 2) != USBD_OK) {};
						ptr += 2;
					}
				} else if (byte == 'S') {
					// E-Stop
					// Stop burst
					TIM16->CCMR1 &= (uint16_t)(~TIM_CCMR1_OC1M_0);	// OC1REF forced low
					SendOperationReady = 0;
				}
			} else if (comm_state == STATE_GET_FRAME_SIZE) {
				ram_frame_size = byte;
				comm_state = STATE_GET_FRAME_REPEAT_DELAY;
			} else if (comm_state == STATE_GET_FRAME_REPEAT_DELAY) {
				ram_frame_repeat_delay = byte;
				comm_state = STATE_GET_FRAME_REPEATS_LOW;
			} else if (comm_state == STATE_GET_FRAME_REPEATS_LOW) {
				ram_frame_repeats = byte;
				comm_state = STATE_GET_FRAME_REPEATS_HIGH;
			} else if (comm_state == STATE_GET_FRAME_REPEATS_HIGH) {
				ram_frame_repeats |= ((uint32_t)byte << 8);
				ram_frame_data_counter = 0;
				comm_state = STATE_GET_FRAME_DATA;
			} else if (comm_state == STATE_GET_FRAME_DATA) {
				ram_frame_data[ram_frame_data_counter++] = byte;
				if (ram_frame_data_counter == ram_frame_size)
					comm_state = STATE_IDLE;
			} else if (comm_state == STATE_FLASH_LOAD) {
				flash_data_buffer[flash_data_counter++] = byte;

				if (!(flash_data_counter & 0x7F))
					CDC_OK

				if (flash_data_counter == 1024) {
					// Write flash
					// Unlock
					while ((FLASH->SR & FLASH_SR_BSY) != 0) {}
					if (FLASH->CR & FLASH_CR_LOCK) {
						FLASH->KEYR = FLASH_KEY1;
						FLASH->KEYR = FLASH_KEY2;
					}
					// Main flash: 0x0800 0000 - 0x0800 7FFF
					// 32 pages de 1kB
					// Page #31: data

					// Erase
					FLASH->CR |= FLASH_CR_PER;
					FLASH->AR = (uint32_t)0x8007C00;
					FLASH->CR |= FLASH_CR_STRT;
					while ((FLASH->SR & FLASH_SR_BSY) != 0) {}
					if (FLASH->SR & FLASH_SR_EOP)
						FLASH->SR = FLASH_SR_EOP;
					else
						CDC_NOK
					FLASH->CR &= ~FLASH_CR_PER;

					// Program halfword
					FLASH->CR |= FLASH_CR_PG;
					__IO uint16_t* ptr = (uint16_t*)0x8007C00;
					uint16_t* src = (uint16_t*)flash_data_buffer;
					for (uint32_t d = 0; d < 1024; d += 2) {
						*ptr++ = *src++;
					}
					while (FLASH->SR & FLASH_SR_BSY) {}
					if (FLASH->SR & FLASH_SR_EOP)
						FLASH->SR = FLASH_SR_EOP;
					else
						CDC_NOK
					FLASH->CR &= ~FLASH_CR_PG;

					CDC_OK
					RED_LED_MAX

					comm_state = STATE_IDLE;
				}
			}

			RX_FIFO.tail = FIFO_INCR(RX_FIFO.tail);
		}
	}
}

void SystemClock_Config(void) {
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);	// 1ws

	if (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1)
	  Error_Handler();

	LL_RCC_HSE_Enable();

	// Wait for HSE ready
	while (LL_RCC_HSE_IsReady() != 1) {};

	// 4 * 12 = 48MHz
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLL_MUL_6, LL_RCC_PREDIV_DIV_1);
	LL_RCC_PLL_Enable();

	// Wait for PLL ready
	while (LL_RCC_PLL_IsReady() != 1) {};

	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

	// Wait for System clock ready
	while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {};

	LL_Init1msTick(48000000);
	LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
	LL_SetSystemCoreClock(48000000);
	LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_PLL);
}

void Error_Handler(void) {
	/* User can add his own implementation to report the HAL error return state */
}
