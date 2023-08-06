#include "main.h"
#include "stm32f0xx_it.h"

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

void USB_IRQHandler(void) {
	HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
