/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_desc.c
  * @version        : v2.0_Cube
  * @brief          : This file implements the USB device descriptors.
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"

#define USBD_VID 1155
#define USBD_PID_FS 22336
#define USBD_LANGID_STRING 1033

//#define USBD_MANUFACTURER_STRING "Furrtek engineering"
// USB uses UTF-16LE
// unicode[idx++] = *len;
// unicode[idx++] = USB_DESC_TYPE_STRING;
static const uint8_t USBD_MANUFACTURER_STRING_DATA[] = {
	"\x28\x03" "F\0u\0r\0r\0t\0e\0k\0 \0e\0n\0g\0i\0n\0e\0e\0r\0i\0n\0g\0"
};

//#define USBD_PRODUCT_STRING_FS "ESL Blaster Rev. B"
static const uint8_t USBD_PRODUCT_STRING_DATA[] = {
	"\x26\x03" "E\0S\0L\0 \0B\0l\0a\0s\0t\0e\0r\0 \0R\0e\0v\0.\0 \0B\0"
};

//#define USBD_CONFIGURATION_STRING_F "CDC Config"
static const uint8_t USBD_CONFIGURATION_STRING_DATA[] = {
	"\x16\x03" "C\0D\0C\0 \0C\0o\0n\0f\0i\0g\0"
};

//#define USBD_INTERFACE_STRING_FS "CDC Interface"
static const uint8_t USBD_INTERFACE_STRING_DATA[] = {
	"\x1C\x03" "C\0D\0C\0 \0I\0n\0t\0e\0r\0f\0a\0c\0e\0"
};

static void Get_SerialNum(void);
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len);

uint8_t * USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
uint8_t * USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);

#ifdef USBD_SUPPORT_USER_STRING_DESC
uint8_t * USBD_FS_USRStringDesc(USBD_SpeedTypeDef speed, uint8_t idx, uint16_t *length);
#endif /* USBD_SUPPORT_USER_STRING_DESC */

USBD_DescriptorsTypeDef FS_Desc = {
	USBD_FS_DeviceDescriptor,
	USBD_FS_LangIDStrDescriptor,
	USBD_FS_ManufacturerStrDescriptor,
	USBD_FS_ProductStrDescriptor,
	USBD_FS_SerialStrDescriptor,
	USBD_FS_ConfigStrDescriptor,
	USBD_FS_InterfaceStrDescriptor
};

#if defined ( __ICCARM__ ) /* IAR Compiler */
  #pragma data_alignment=4
#endif /* defined ( __ICCARM__ ) */
/** USB standard device descriptor. */
__ALIGN_BEGIN uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END = {
	0x12,                       /*bLength */
	USB_DESC_TYPE_DEVICE,       /*bDescriptorType*/
	0x00,                       /*bcdUSB */
	0x02,
	0x02,                       /*bDeviceClass*/
	0x02,                       /*bDeviceSubClass*/
	0x00,                       /*bDeviceProtocol*/
	USB_MAX_EP0_SIZE,           /*bMaxPacketSize*/
	LOBYTE(USBD_VID),           /*idVendor*/
	HIBYTE(USBD_VID),           /*idVendor*/
	LOBYTE(USBD_PID_FS),        /*idProduct*/
	HIBYTE(USBD_PID_FS),        /*idProduct*/
	0x00,                       /*bcdDevice rel. 2.00*/
	0x02,
	USBD_IDX_MFC_STR,           /*Index of manufacturer  string*/
	USBD_IDX_PRODUCT_STR,       /*Index of product string*/
	USBD_IDX_SERIAL_STR,        /*Index of serial number string*/
	USBD_MAX_NUM_CONFIGURATION  /*bNumConfigurations*/
};

/** USB lang indentifier descriptor. */
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END = {
     USB_LEN_LANGID_STR_DESC,
     USB_DESC_TYPE_STRING,
     LOBYTE(USBD_LANGID_STRING),
     HIBYTE(USBD_LANGID_STRING)
};

/*
// Internal string descriptor.
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;
*/

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4   
#endif
__ALIGN_BEGIN uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END = {
	USB_SIZ_STRING_SERIAL,
	USB_DESC_TYPE_STRING,
};

/**
  * @brief  Return the device descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	*length = sizeof(USBD_FS_DeviceDesc);
	return USBD_FS_DeviceDesc;
}

/**
  * @brief  Return the LangID string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	*length = sizeof(USBD_LangIDDesc);
	return USBD_LangIDDesc;
}

/**
  * @brief  Return the product string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	//if (speed == 0)
	//	USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, USBD_StrDesc, length);
	//else
		//USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, USBD_StrDesc, length);
	//return USBD_StrDesc;

	*length = sizeof(USBD_PRODUCT_STRING_DATA);	//38
	return (uint8_t*)USBD_PRODUCT_STRING_DATA;
}

/**
  * @brief  Return the manufacturer string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	//USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
	//return USBD_StrDesc;

	*length = sizeof(USBD_MANUFACTURER_STRING_DATA);	//40
	return (uint8_t*)USBD_MANUFACTURER_STRING_DATA;
}

/**
  * @brief  Return the serial number string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	*length = USB_SIZ_STRING_SERIAL;

	Get_SerialNum();

	return (uint8_t *) USBD_StringSerial;
}

/**
  * @brief  Return the configuration string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	//if (speed == USBD_SPEED_HIGH)
	//	USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING_FS, USBD_StrDesc, length);
	//else
		//USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING_FS, USBD_StrDesc, length);
	//return USBD_StrDesc;

	*length = sizeof(USBD_CONFIGURATION_STRING_DATA);	//22
	return (uint8_t*)USBD_CONFIGURATION_STRING_DATA;
}

/**
  * @brief  Return the interface string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t * USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length) {
	//if (speed == 0)
	//	USBD_GetString((uint8_t *)USBD_INTERFACE_STRING_FS, USBD_StrDesc, length);
	//else
		//USBD_GetString((uint8_t *)USBD_INTERFACE_STRING_FS, USBD_StrDesc, length);
	//return USBD_StrDesc;

	*length = sizeof(USBD_INTERFACE_STRING_DATA);	//28
	return (uint8_t*)USBD_INTERFACE_STRING_DATA;
}

/**
  * @brief  Create the serial number string descriptor 
  * @param  None 
  * @retval None
  */
static void Get_SerialNum(void) {
	uint32_t deviceserial0, deviceserial1, deviceserial2;

	deviceserial0 = *(uint32_t *) DEVICE_ID1;
	deviceserial1 = *(uint32_t *) DEVICE_ID2;
	deviceserial2 = *(uint32_t *) DEVICE_ID3;

	deviceserial0 += deviceserial2;

	if (deviceserial0 != 0) {
		IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
		IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
	}
}

/**
  * @brief  Convert Hex 32Bits value into char 
  * @param  value: value to convert
  * @param  pbuf: pointer to the buffer 
  * @param  len: buffer length
  * @retval None
  */
static void IntToUnicode(uint32_t value, uint8_t * pbuf, uint8_t len) {
	uint32_t idx = 0, temp;

	for (idx = 0; idx < (len << 1); idx += 2) {
		temp = value >> 28;

		if (temp < 10)
			pbuf[idx] = temp + '0';
		else
			pbuf[idx] = temp + 'A' - 10;

		pbuf[idx + 1] = 0;

		value = value << 4;
	}
}
