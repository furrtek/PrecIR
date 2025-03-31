// ESL Blaster firmware v3.00 SECIAL
// For board rev. C
// 2024 furrtek
// This firmware handles special commands to transmit raw frames

// DO NOT COMPILE WITH -O3 ! Only -O1, or USB won't work anymore
// Don't forget to change USBD_PRODUCT_STRING_DATA to reflect HW version

// Bugfix: Only one NACK sent if flash programming failed instead of two
// Bugfix: Flash readback returns full 1kb instead of half
// OK: If powered at 5V but no USB enumeration after 5s, then act as remote (useful is plugged in powerbank)
// OK: Remote mode red LED weak to preserve battery, make it blink
// OK: 5V power red LED hello blink

// USB mode:
// Faint LED: Charging battery
// LED off: Battery charged / no battery
// Bright LED: Transmitting IR
// Remote mode:
// Faint LED: Transmitting IR

// Ack after T command when frame is transmitted

// STATE_IDLE
// STATE_FLASH_LOAD (1024 bytes, ack every 128 bytes)
// Ack or Nack depending on flash programming success
// STATE_IDLE

#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "init.h"
#include "ir.h"

uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

volatile FIFO RX_FIFO = {.head=0, .tail=0};

volatile uint32_t comm_reset_flag = 0;
volatile uint32_t remote_mode = 0;
volatile uint32_t ram_frame_repeat_delay;
uint8_t charging = 0;

userdata_t userdata __attribute__((section(".flash_data_array")));
uint8_t flash_data_buffer[1024];
uint8_t ram_frame_data[256];

void TIM14_IRQHandler(void) {
	comm_reset_flag = 1;
	TIM14->SR &= (uint16_t)(~TIM_SR_UIF);	// Clear update interrupt flag
}

uint32_t adc_read() {
	ADC1->CR = ADC_CR_ADEN;
	while (!(ADC1->ISR & ADC_ISR_ADRDY));	// Wait for ADRDY
	ADC1->CR |= ADC_CR_ADSTART;
	while (!(ADC1->ISR & ADC_ISR_EOC));		// Wait for EOC flag
	return ADC1->DR;
}

void remote() {
	uint32_t remote_frame_count = userdata.entry_count;
	uint32_t remote_frame_index = 0;
	remote_mode = 1;

	if (remote_frame_count > 13) while (1) { };	// Don't transmit anything if frame count is invalid (userdata not programmed ?)

	while (1) {
		uint32_t frame_size = userdata.entries[remote_frame_index].frame_size;

		// Do we really have to copy from flash to RAM ?
		for (uint32_t c = 0; c < frame_size; c++)
			ram_frame_data[c] = userdata.entries[remote_frame_index].frame_data[c];

		IRTX(
			ram_frame_data,
			userdata.entries[remote_frame_index].flags & 1,
			frame_size,
			userdata.entries[remote_frame_index].repeats,
			userdata.entries[remote_frame_index].delay
		);

		while (IRTXBusy) {};

		remote_frame_index++;
		if (remote_frame_index >= remote_frame_count) {
			remote_frame_index = 0;
			// Short blink each time the frame list has been transmitted
			RED_LED_OFF
			LL_mDelay(50);
		}
	}
}

int main(void) {
	protocol_t ram_frame_protocol = PROTOCOL_PP4;
	comm_state_t comm_state = STATE_IDLE;
	uint32_t ram_frame_size = 0, ram_frame_repeats = 0, ram_frame_data_counter = 0;
	uint32_t flash_data_counter = 0;
	uint8_t usb_configured = 0, usb_timer = 0;

	static_assert (sizeof(userdata) == 1010, "userdata should be exactly 1010 bytes");

	Init();
	LL_mDelay(100);

	// ADC step = 3.3/(2^12-1) = ~806uV
	// USB: 5V				5/2 = 2.5V -> v = ~3102
	// Batt full charge:	4.2/2 = 2.1V -> v = ~2605

	// Powered by less than 4.35V: running from battery, go to remote mode
	if (adc_read() < 2700) remote();

	// Blink hello
	for (uint32_t c = 0; c < 3; c++) {
		RED_LED_MAX
		LL_mDelay(300);
		RED_LED_OFF
		LL_mDelay(300);
	}

	// Main loop
	while (1) {

		if (comm_reset_flag) {
			comm_reset_flag = 0;
			comm_state = STATE_IDLE;

			// Check if USB is configured
			if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
				usb_configured = 1;	// USB was configured at least once, never go to remote mode
			} else {
				if (!usb_configured) {
					if (usb_timer >= 36) {
						// ~5s with a comm_reset_flag period of 137ms, USB never got configured
						// Probably on a powerbank, go to remote mode
						remote();
					} else
						usb_timer++;
				}
			}

			// Check MCP73831 STAT output, debounce state to avoid LED flicker when no battery
			// Low: Charging
			// High: Charge done
			// Hi-z/erratic: No battery
			if (!(GPIOA->IDR & GPIO_IDR_4)) {
				if (charging < 10)
					charging++;
			} else {
				if (charging)
					charging--;
			}

			if (!IRTXBusy) RED_LED_NORMAL	// Update LED
		}

		while (RX_FIFO.head != RX_FIFO.tail) {
			TIM14->CNT = (uint16_t)0;	// Reset comm. timeout

			// Process one byte from the USB RX FIFO
			uint8_t byte = RX_FIFO.data[RX_FIFO.tail];

			if (comm_state == STATE_IDLE) {
				if (byte == 'L') {
					// Load new frame data
					comm_state = STATE_GET_FRAME_SIZE;
				} else if (byte == 'T') {
					// Transmit last loaded frame
					if (((ram_frame_protocol == PROTOCOL_RAW) && (ram_frame_size > 4)) ||
						(ram_frame_protocol != PROTOCOL_RAW))	// Minimum raw mode frame size includes carrier and baudrate
					IRTX(ram_frame_data, ram_frame_protocol, ram_frame_size, ram_frame_repeats, ram_frame_repeat_delay);
				} else if (byte == '?') {
					// Reply ID
					// Letter: HW revision
					// Number: FW version
					CDC_Transmit_FS((uint8_t*)"ESLBlasterC3S", 13);
				} else if (byte == 'W') {
					// Write flash
					flash_data_counter = 0;
					comm_state = STATE_FLASH_LOAD;
				} else if (byte == 'R') {
					// Read flash
					uint16_t buff;
					uint16_t* ptr = (uint16_t*)0x8007C00;
					for (uint32_t d = 0; d < (1024/2); d++) {
						buff = *(ptr++);
						while(CDC_Transmit_FS((uint8_t*)&buff, 2) != USBD_OK) {};
					}
				} else if (byte == 'S') {
					// E-Stop
					IRStop();
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
				// Frame repeats bits [7:6] indicate protocol type
				ram_frame_repeats |= (((uint32_t)byte & 0x3F) << 8);
				ram_frame_protocol = (byte & 0xC0) >> 6;
				ram_frame_data_counter = 0;
				comm_state = STATE_GET_FRAME_DATA;
			} else if (comm_state == STATE_GET_FRAME_DATA) {
				ram_frame_data[ram_frame_data_counter++] = byte;
				if (ram_frame_data_counter >= ram_frame_size)
					comm_state = STATE_IDLE;	// Overflow - CDC_NOK ?
			} else if (comm_state == STATE_FLASH_LOAD) {
				flash_data_buffer[flash_data_counter++] = byte;

				if (!(flash_data_counter & 0x7F))
					CDC_OK	// Ack every 128 bytes

				if (flash_data_counter == 1024) {
					// Write flash
					// Unlock
					while ((FLASH->SR & FLASH_SR_BSY) != 0) {}
					if (FLASH->CR & FLASH_CR_LOCK) {
						FLASH->KEYR = FLASH_KEY1;
						FLASH->KEYR = FLASH_KEY2;
					}
					// Main flash: 0x0800 0000 - 0x0800 7FFF
					// 32x 1kB pages - User data is in last page

					// Erase
					FLASH->CR |= FLASH_CR_PER;
					FLASH->AR = (uint32_t)0x8007C00;
					FLASH->CR |= FLASH_CR_STRT;
					while (FLASH->SR & FLASH_SR_BSY) {};
					FLASH->CR &= ~FLASH_CR_PER;
					if (FLASH->SR & FLASH_SR_EOP) {
						FLASH->SR = FLASH_SR_EOP;

						// Program halfwords
						FLASH->CR |= FLASH_CR_PG;
						__IO uint16_t* ptr = (uint16_t*)0x8007C00;
						uint16_t* src = (uint16_t*)flash_data_buffer;
						for (uint32_t d = 0; d < 1024; d += 2) {
							*ptr++ = *src++;
						}
						while (FLASH->SR & FLASH_SR_BSY) {};
						FLASH->CR &= ~FLASH_CR_PG;
						if (FLASH->SR & FLASH_SR_EOP)
							FLASH->SR = FLASH_SR_EOP;
						else
							CDC_NOK

						CDC_OK
						RED_LED_MAX
					} else
						CDC_NOK

					comm_state = STATE_IDLE;
				}
			}

			RX_FIFO.tail = FIFO_INCR(RX_FIFO.tail);
		}
	}
}

void Error_Handler(void) {
}
