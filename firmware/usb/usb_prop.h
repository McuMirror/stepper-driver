/**
  ******************************************************************************
  * @file    usb_prop.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   All processing related to Mass Storage Demo (Endpoint 0)
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
#ifndef __usb_prop_H
#define __usb_prop_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void USB_init(void);
void USB_Reset(void);
void USB_SetConfiguration(void);
void USB_SetDeviceAddress (void);
void USB_Status_In (void);
void USB_Status_Out (void);
RESULT USB_Data_Setup(uint8_t);
RESULT USB_NoData_Setup(uint8_t);
RESULT USB_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t *USB_GetDeviceDescriptor(uint16_t );
uint8_t *USB_GetConfigDescriptor(uint16_t);
uint8_t *USB_GetStringDescriptor(uint16_t);

extern __IO uint8_t ep2_needs_zlp;
extern __IO uint8_t ep3_needs_zlp;

extern __IO uint8_t ep2_needs_data;
extern __IO uint8_t ep3_needs_data;

/* Exported define -----------------------------------------------------------*/
#define USB_GetConfiguration          NOP_Process
//#define USB_SetConfiguration          NOP_Process
#define USB_GetInterface              NOP_Process
#define USB_SetInterface              NOP_Process
#define USB_GetStatus                 NOP_Process
#define USB_ClearFeature              NOP_Process
#define USB_SetEndPointFeature        NOP_Process
#define USB_SetDeviceFeature          NOP_Process
//#define USB_SetDeviceAddress          NOP_Process

#define USB_PROGRAM_START               0x20
#define USB_PROGRAM_INSTRUCTION         0x21
#define USB_PROGRAM_END                 0x22
#define USB_PROGRAM_LOAD                0x23
#define USB_PROGRAM_IMMEDIATE           0x24
#define USB_GET_ERROR                   0x25
#define USB_GET_N_COMMANDS              0x26
#define USB_GET_COMMAND_NAME            0x27
#define USB_GET_COMMAND_DATA_DESCRIPTOR 0x28

#endif /* __usb_prop_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

