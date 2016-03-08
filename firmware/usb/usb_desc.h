/**
  ******************************************************************************
  * @file    usb_desc.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Descriptor Header for Audio Speaker Demo
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DESC_H
#define __USB_DESC_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/

#define DEVICE_DESCRIPTOR_LENGTH 18
#define CONFIG_DESCRIPTOR_LENGTH 32
#define STRING_LANG_ID_LENGTH 4 
#define STRING_VENDOR_LENGTH 32 
#define STRING_PRODUCT_LENGTH 30
#define STRING_SERIAL_LENGTH 26 

#define BULK_PACKET_LEN 64

/* Exported functions ------------------------------------------------------- */
extern const uint8_t USB_DeviceDescriptor[DEVICE_DESCRIPTOR_LENGTH];
extern const uint8_t USB_ConfigDescriptor[CONFIG_DESCRIPTOR_LENGTH];
extern const uint8_t USB_StringLangID[STRING_LANG_ID_LENGTH];
extern const uint8_t USB_StringVendor[STRING_VENDOR_LENGTH];
extern const uint8_t USB_StringProduct[STRING_PRODUCT_LENGTH];
extern uint8_t USB_StringSerial[STRING_SERIAL_LENGTH];

#endif /* __USB_DESC_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


