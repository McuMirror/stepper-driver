/**
  ******************************************************************************
  * @file    usb_prop.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   All processing related to Audio USB Demo
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
#include "hw_config.h"

#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_pwr.h"

#include "program.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

__IO uint8_t ep2_needs_zlp;
__IO uint8_t ep3_needs_zlp;

__IO uint8_t ep2_needs_data;
__IO uint8_t ep3_needs_data;


DEVICE Device_Table =
  {
    EP_NUM,
    1
  };

DEVICE_PROP Device_Property =
  {
    USB_init,
    USB_Reset,
    USB_Status_In,
    USB_Status_Out,
    USB_Data_Setup,
    USB_NoData_Setup,
    USB_Get_Interface_Setting,
    USB_GetDeviceDescriptor,
    USB_GetConfigDescriptor,
    USB_GetStringDescriptor,
    0,
    0x40 /*MAX PACKET SIZE*/
  };

USER_STANDARD_REQUESTS User_Standard_Requests =
  {
    USB_GetConfiguration,
    USB_SetConfiguration,
    USB_GetInterface,
    USB_SetInterface,
    USB_GetStatus,
    USB_ClearFeature,
    USB_SetEndPointFeature,
    USB_SetDeviceFeature,
    USB_SetDeviceAddress
  };

ONE_DESCRIPTOR Device_Descriptor =
  {
    (uint8_t*)USB_DeviceDescriptor,
    DEVICE_DESCRIPTOR_LENGTH
  };

ONE_DESCRIPTOR Config_Descriptor =
  {
    (uint8_t*)USB_ConfigDescriptor,
    CONFIG_DESCRIPTOR_LENGTH
  };

ONE_DESCRIPTOR String_Descriptor[4] =
  {
    {(uint8_t*)USB_StringLangID, STRING_LANG_ID_LENGTH},
    {(uint8_t*)USB_StringVendor, STRING_VENDOR_LENGTH},
    {(uint8_t*)USB_StringProduct, STRING_PRODUCT_LENGTH},
    {(uint8_t*)USB_StringSerial, STRING_SERIAL_LENGTH},
  };

static uint8_t control_transfer_buffer[64];
uint8_t error_code;

/* Extern variables ----------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

uint8_t* USB_Read_Data(uint16_t Length)
{
  uint32_t wOffset=pInformation->Ctrl_Info.Usb_wOffset;

  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = sizeof(control_transfer_buffer) - wOffset;
    return 0;
  }

  return (uint8_t*)(control_transfer_buffer + wOffset);
}

uint8_t* USB_Get_Error(uint16_t Length)
{
  uint32_t wOffset=pInformation->Ctrl_Info.Usb_wOffset;

  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = sizeof(error_code) - wOffset;
    return 0;
  }

  return (uint8_t*)(&error_code + wOffset);
}

/*******************************************************************************
* Function Name  : USB_init.
* Description    : USB init routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_init()
{
  /* Update the serial number string descriptor with the data from the unique
  ID*/
  Get_SerialNum();

  /* Initialize the current configuration */
  pInformation->Current_Configuration = 0;

  /* Connect the device */
  PowerOn();

  /* Perform basic device initialization operations */
  USB_SIL_Init();

  bDeviceState = UNCONNECTED;
}

/*******************************************************************************
* Function Name  : USB_Reset.
* Description    : USB reset routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_Reset()
{
  // Set USB device as not configured state
  pInformation->Current_Configuration = 0;

  // Current Feature initialization
  pInformation->Current_Feature = USB_ConfigDescriptor[7];

  SetBTABLE(BTABLE_ADDRESS);

  // Initialize Endpoint 0
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_NAK);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxValid(ENDP0);

  // Initialize Endpoint 1
  SetEPType(ENDP1, EP_BULK);
  SetEPRxAddr(ENDP1, ENDP1_RXADDR);
  SetEPRxCount(ENDP1, BULK_PACKET_LEN);
  SetEPRxStatus(ENDP1, EP_RX_VALID);
  SetEPTxStatus(ENDP1, EP_TX_DIS);

  // Initialize Endpoint 2
  SetEPType(ENDP2, EP_BULK);
  SetEPTxAddr(ENDP2, ENDP2_TXADDR);
  SetEPTxStatus(ENDP2, EP_TX_NAK);
  SetEPRxStatus(ENDP2, EP_RX_DIS);
  ep2_needs_zlp = 0;
  ep2_needs_data = 1;

  // Initialize Endpoint 3
  SetEPType(ENDP3, EP_BULK);
  SetEPTxAddr(ENDP3, ENDP3_TXADDR);
  SetEPTxStatus(ENDP3, EP_TX_NAK);
  SetEPRxStatus(ENDP3, EP_RX_DIS);
  ep3_needs_zlp = 0;
  ep3_needs_data = 1;

  // Set this device to response on default address
  SetDeviceAddress(0);

  bDeviceState = ATTACHED;
}
/*******************************************************************************
* Function Name  : USB_SetConfiguration.
* Description    : Update the device state to configured.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_SetConfiguration(void)
{
  DEVICE_INFO *pInfo = &Device_Info;

  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
}
/*******************************************************************************
* Function Name  : USB_SetConfiguration.
* Description    : Update the device state to addressed.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_SetDeviceAddress (void)
{
  bDeviceState = ADDRESSED;
}
/*******************************************************************************
* Function Name  : USB_Status_In.
* Description    : USB Status In routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_Status_In(void)
{
  uint16_t wValue = ByteSwap(pInformation->USBwValue);
  uint16_t wIndex = ByteSwap(pInformation->USBwIndex);
  uint16_t wLength = pInformation->USBwLength;

  switch(pInformation->USBbRequest)
  {
    case USB_PROGRAM_INSTRUCTION:
      error_code = program_instruction(wValue, wIndex, control_transfer_buffer, wLength);
      break;
    case USB_PROGRAM_IMMEDIATE:
      error_code = program_immediate(wValue, wIndex, control_transfer_buffer, wLength);
      break;
    default:
      break;
  }
}

/*******************************************************************************
* Function Name  : USB_Status_Out.
* Description    : USB Status Out routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_Status_Out (void)
{
}

/*******************************************************************************
* Function Name  : USB_Data_Setup
* Description    : Handle the data class specific requests.
* Input          : None.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
RESULT USB_Data_Setup(uint8_t RequestNo)
{
  uint8_t *(*CopyRoutine)(uint16_t);
  CopyRoutine = NULL;

  switch(RequestNo)
  {
    case USB_PROGRAM_INSTRUCTION:
    case USB_PROGRAM_IMMEDIATE:
      CopyRoutine = USB_Read_Data;
      break;
    case USB_GET_ERROR:
      CopyRoutine = USB_Get_Error;
      break;
    default:
      return USB_UNSUPPORT;
  }

  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : USB_NoData_Setup
* Description    : Handle the no data class specific requests.
* Input          : None.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
RESULT USB_NoData_Setup(uint8_t RequestNo)
{
  uint16_t wValue = ByteSwap(pInformation->USBwValue);
  uint16_t wIndex = ByteSwap(pInformation->USBwIndex);

  switch(RequestNo) {
    case USB_PROGRAM_START:
      error_code = program_start(wValue);
      return USB_SUCCESS;
    case USB_PROGRAM_INSTRUCTION:
      error_code = program_instruction(wValue, wIndex, NULL, 0);
      return USB_SUCCESS;
    case USB_PROGRAM_END:
      error_code = program_end(wValue);
      return USB_SUCCESS;
    case USB_PROGRAM_LOAD:
      error_code = program_load(wValue);
      return USB_SUCCESS;
    case USB_PROGRAM_IMMEDIATE:
      error_code = program_immediate(wValue, wIndex, NULL, 0);
      return USB_SUCCESS;
    default:
      return USB_UNSUPPORT;
  }
}

/*******************************************************************************
* Function Name  : USB_GetDeviceDescriptor.
* Description    : Get the device descriptor.
* Input          : Length : uint16_t.
* Output         : None.
* Return         : The address of the device descriptor.
*******************************************************************************/
uint8_t *USB_GetDeviceDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

/*******************************************************************************
* Function Name  : USB_GetConfigDescriptor.
* Description    : Get the configuration descriptor.
* Input          : Length : uint16_t.
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
uint8_t *USB_GetConfigDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/*******************************************************************************
* Function Name  : USB_GetStringDescriptor.
* Description    : Get the string descriptors according to the needed index.
* Input          : Length : uint16_t.
* Output         : None.
* Return         : The address of the string descriptors.
*******************************************************************************/
uint8_t *USB_GetStringDescriptor(uint16_t Length)
{
  uint8_t wValue0 = pInformation->USBwValue0;

  if (wValue0 > 4)
  {
    return NULL;
  }
  else
  {
    return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
  }
}

/*******************************************************************************
* Function Name  : USB_Get_Interface_Setting.
* Description    : test the interface and the alternate setting according to the
*                  supported one.
* Input1         : uint8_t: Interface : interface number.
* Input2         : uint8_t: AlternateSetting : Alternate Setting number.
* Output         : None.
* Return         : The address of the string descriptors.
*******************************************************************************/
RESULT USB_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
  if (AlternateSetting > 1)
  {
    return USB_UNSUPPORT;
  }
  else if (Interface > 1)
  {
    return USB_UNSUPPORT;
  }
  return USB_SUCCESS;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

