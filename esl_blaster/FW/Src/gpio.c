/**
  ******************************************************************************
  * File Name          : gpio.c
  * Description        : This file provides code for the configuration
  *                      of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "gpio.h"

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
