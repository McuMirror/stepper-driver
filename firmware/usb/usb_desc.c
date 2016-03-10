/**
  ******************************************************************************
  * @file    usb_desc.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Descriptors for Audio Speaker Demo
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants----------------------------------------------------------*/
/* USB Standard Device Descriptor */
const uint8_t USB_DeviceDescriptor[] =
  {
    CONFIG_DESCRIPTOR_LENGTH,             /* bLength */
    0x01,                                 /* bDescriptorType */
    0x00,          /* 2.00 */             /* bcdUSB */
    0x02,
    0x00,                                 /* bDeviceClass */
    0x00,                                 /* bDeviceSubClass */
    0x00,                                 /* bDeviceProtocol */
    0x40,                                 /* bMaxPacketSize 40 */
    0xFF,                                 /* idVendor = 0xFFFF */
    0xFF,
    0x66,                                 /* idProduct  = 0x7A66*/
    0x7A,
    0x00,          /* 2.00 */             /* bcdDevice */
    0x02,
    1,                                    /* iManufacturer */
    2,                                    /* iProduct */
    3,                                    /* iSerialNumber */
    0x01,                                 /* bNumConfigurations */
  };

/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
const uint8_t USB_ConfigDescriptor[] =
  {
    /* Configuration 1 */
    0x09,                                 /* bLength */
    0x02,                                 /* bDescriptorType */
    CONFIG_DESCRIPTOR_LENGTH,             /* wTotalLength  110 bytes*/
    0x00,
    0x01,                                 /* bNumInterfaces */
    0x01,                                 /* bConfigurationValue */
    0x00,                                 /* iConfiguration */
    0x80,                                 /* bmAttributes Self Powred*/
    0xFA,                                 /* bMaxPower = 500 mA*/
    /* 09 byte*/

    /* Standard interface descriptor */
    9,                                    /* bLength */
    0x04,                                 /* bDescriptorType */
    0x00,                                 /* bInterfaceNumber */
    0x00,                                 /* bAlternateSetting */
    0x03,                                 /* bNumEndpoints */
    0xFF,                                 /* bInterfaceClass */
    0xFF,                                 /* bInterfaceSubClass */
    0xFF,                                 /* bInterfaceProtocol */
    0x00,                                 /* iInterface */
    /* 09 byte*/

    /* Endpoint 1 - Standard Descriptor */
    7,                                    /* bLength */
    0x05,                                 /* bDescriptorType */
    0x01,                                 /* bEndpointAddress 1 out endpoint*/
    0x02,                                 /* bmAttributes */
    BULK_PACKET_LEN,                      /* wMaxPacketSize 64 bytes*/
    0x00,
    0x00,                                 /* bInterval */
    /* 07 byte*/

    /* Endpoint 2 - Standard Descriptor */
    7,                                    /* bLength */
    0x05,                                 /* bDescriptorType */
    0x82,                                 /* bEndpointAddress 2 in endpoint*/
    0x02,                                 /* bmAttributes */
    BULK_PACKET_LEN,                      /* wMaxPacketSize 64 bytes*/
    0x00,
    0x00,                                 /* bInterval */
    /* 07 byte*/

    /* Endpoint 3 - Standard Descriptor */
    7,                                    /* bLength */
    0x05,                                 /* bDescriptorType */
    0x83,                                 /* bEndpointAddress 3 in endpoint*/
    0x02,                                 /* bmAttributes */
    BULK_PACKET_LEN,                      /* wMaxPacketSize 64 bytes*/
    0x00,
    0x00,                                 /* bInterval */
    /* 07 byte*/
 };

/* USB String Descriptor (optional) */
const uint8_t USB_StringLangID[STRING_LANG_ID_LENGTH] =
  {
    STRING_LANG_ID_LENGTH,
    0x03,  /* bDescriptorType*/
    0x09,
    0x04
  }
  ; /* LangID = 0x0409: U.S. English */

const uint8_t USB_StringVendor[STRING_VENDOR_LENGTH] =
  {
    STRING_VENDOR_LENGTH, /* Size of manufacturer string */
    0x03,  /* bDescriptorType*/
    /* Manufacturer: "STMicroelectronics" */
    'E', 0, 'r', 0, 'i', 0, 'c', 0, ' ', 0, 'V', 0, 'a', 0, 'n', 0, ' ', 0,
    'A', 0, 'l', 0, 'b', 0, 'e', 0, 'r', 0, 't', 0
  };

const uint8_t USB_StringProduct[STRING_PRODUCT_LENGTH] =
  {
    STRING_PRODUCT_LENGTH,  /* bLength */
    0x03,        /* bDescriptorType */
    'S', 0, 't', 0, 'e', 0, 'p', 0, 'p', 0, 'e', 0, 'r', 0, ' ', 0, 'D', 0,
    'r', 0, 'i', 0, 'v', 0, 'e', 0, 'r', 0
  };

uint8_t USB_StringSerial[STRING_SERIAL_LENGTH] =
  {
    STRING_SERIAL_LENGTH,  /* bLength */
    0x03,        /* bDescriptorType */
    'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0
  };
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
