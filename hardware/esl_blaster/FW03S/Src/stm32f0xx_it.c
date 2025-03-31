#include "main.h"
#include "stm32f0xx_it.h"
#include "ir.h"

extern PCD_HandleTypeDef hpcd_USB_FS;

void NMI_Handler(void) {
}

void HardFault_Handler(void) {
	while (1) {
	}
}

void SVC_Handler(void) {
}

void PendSV_Handler(void) {
}

void SysTick_Handler(void) {
	HAL_IncTick();
}

void TIM16_IRQHandler(void) {
	if (IRTXBusy)
		symbol_func();

	TIM16->SR &= (uint16_t)(~TIM_SR_UIF);	// Clear update interrupt flag
}

void USB_IRQHandler(void) {
	HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
