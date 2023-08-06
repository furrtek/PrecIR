#include "init.h"
#include "usb_device.h"

void Init() {
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
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM16);
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM17);
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);

	GPIO_InitTypeDef GPIO_InitStruct;
	// Configure PA13 as push-pull output for IR_OUT
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate = GPIO_AF1_IR;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// TIM17: Carrier
	TIM17->PSC = 0;
	TIM17->ARR = 38 - 1;				// 48M / 1.25M = 38.4, 38: 1.26MHz good enough
	TIM17->CCR1 = (uint16_t)(37 / 2);	// 50% duty cycle
	TIM17->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;	// PWM mode 1
	TIM17->CCER |= TIM_CCER_CC1E;	// CC1 output enable
	TIM17->BDTR |= TIM_BDTR_MOE;

	// TIM16: Symbol timing
	TIM16->PSC = 0;
	TIM16->CCMR1 |= TIM_CCMR1_OC1M_2;	// Keep OC1M bit 2 high: Force level defined by OC1M bit 0
	TIM16->CCER |= TIM_CCER_CC1E;		// CC1 output enable
	TIM16->BDTR |= TIM_BDTR_MOE;
	TIM16->DIER |= TIM_DIER_UIE;		// Enable update interrupt

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
	TIM3->ARR = (uint16_t)(0xFFFF);		// 48M/65535 = 732Hz
	TIM3->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0;	// PWM mode 2
	TIM3->CCER = TIM_CCER_CC1E;
	TIM3->CCR1 = 0x1000;
	TIM3->CR1 |= TIM_CR1_CEN;			// Enable timer
	TIM3->EGR |= TIM_EGR_UG;

	ADC1->CFGR1 = 0;
	ADC1->SMPR = ADC_SMPR_SMP_2 | ADC_SMPR_SMP_1 | ADC_SMPR_SMP_0;
	ADC1->CHSELR = ADC_CHSELR_CHSEL5;
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

void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct;

	// GPIO Ports Clock Enable, GPIOB isn't used
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

	// Configure PA4 as input
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// Red LED off
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

	// Configure PA6 as push-pull output
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate = GPIO_AF1_IR;	// TIM3_CH1
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// IR LEDs off
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_SET);

	// Configure PA7 as analog input
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
