#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_gpio.h"

#define FIFO_SIZE 256  // Must be 2^N

#define FIFO_INCR(x) (((x)+1)&((FIFO_SIZE)-1))

#define RED_LED_OFF 	TIM3->CCR1 = 0x0000;
#define RED_LED_NORMAL 	TIM3->CCR1 = (charging >= 5) ? 0x0100 : 0x0000;	// Off/Charge
#define RED_LED_MAX 	TIM3->CCR1 = 0x4000;	// IR transmit
#define RED_LED_WEAK 	TIM3->CCR1 = 0x0800;	// IR transmit in remote mode

#define CDC_NOK CDC_Transmit_FS((uint8_t*)"N", 1);
#define CDC_OK CDC_Transmit_FS((uint8_t*)"K", 1);

typedef struct {
	uint16_t flags;	// Bit 0: Protocol
	uint16_t repeats;
	uint16_t delay;
	uint16_t frame_size;
	uint8_t frame_data[64];
} entry_t;

typedef struct {
	uint16_t entry_count;	// 0~13
	entry_t entries[14];
} userdata_t;

typedef struct FIFO {
	uint32_t head;
	uint32_t tail;
	uint8_t data[FIFO_SIZE];
} FIFO;

extern volatile FIFO RX_FIFO;
extern volatile uint32_t remote_mode;
extern uint8_t charging;

void Error_Handler(void);

#endif
