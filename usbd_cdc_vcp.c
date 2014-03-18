/**
  ******************************************************************************
  * @file    usbd_cdc_vcp.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   Generic media access Layer.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Portions COPYRIGHT 2012 STMicroelectronics</center></h2>
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
/**
  ******************************************************************************
  * <h2><center>&copy; Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.</center></h2>
  * @file    usbd_cdc_vcp.c
  * @author  CMP Team
  * @version V1.0.0
  * @date    28-December-2012
  * @brief   Generic media access Layer.
  *          Modified to support the STM32F4DISCOVERY, STM32F4DIS-BB and
  *          STM32F4DIS-LCD modules.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, Embest SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
  * OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
  * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
  * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  ******************************************************************************
  */
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
#pragma     data_alignment = 4
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */

/* Includes ------------------------------------------------------------------*/
#include "usb_core.h"
#include "usb_conf.h"
#include "usbd_cdc_vcp.h"
#include "main.h"
#include "usb_core.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
LINE_CODING linecoding = {
	115200, /* baud rate*/
	0x00,   /* stop bits-1*/
	0x00,   /* parity - none*/
	0x08    /* nb. of bits 8*/
};


//USART_InitTypeDef USART_InitStructure;
static uint8_t outp_active[2] = {0, 0}; // !0 signals that terminal is opened

/*
extern uint8_t  APP_Rx_Buffer [];
extern uint32_t APP_Rx_ptr_in;
*/

/* Private function prototypes -----------------------------------------------*/
static uint16_t cdc_Init     (USB_OTG_CORE_HANDLE *pdev);
static uint16_t cdc_DeInit   (USB_OTG_CORE_HANDLE *pdev);
static uint16_t VCP_Ctrl     (USB_OTG_CORE_HANDLE *pdev, uint32_t Cmd, uint8_t* Buf, uint32_t Len);
static uint16_t VCP_DataRx   (USB_OTG_CORE_HANDLE *pdev, uint8_t* Buf, uint32_t Len);

CDC_IF_Prop_TypeDef VCP_fops = {
	cdc_Init,
	cdc_DeInit,
	VCP_Ctrl,
	VCP_DataTx,
	VCP_DataRx
};

static uint16_t cdc_Init(USB_OTG_CORE_HANDLE *pdev){
	// mark connected port as active for print()
	DBG("cdc_Init\r\n");
	if(USB_active[0]){
		USB_active[1] = pdev;
		DBG("DEV1\r\n");
	}else{
		USB_active[0] = pdev;
		DBG("DEV0\r\n");
	}
	return USBD_OK;
}


static uint16_t cdc_DeInit(USB_OTG_CORE_HANDLE *pdev){
	// remove device from list of active
	DBG("cdc_DeInit\r\n");
	if(USB_active[0] == pdev){
		USB_active[0] = USB_active[1];
		outp_active[0] = outp_active[1];
	}
	USB_active[1] = NULL;
	outp_active[1] = 0;
	return USBD_OK;
}


/**
  * @brief  VCP_Ctrl
  *         Manage the CDC class requests
  * @param  Cmd: Command code
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion (USBD_OK in all cases)
  */
static uint16_t VCP_Ctrl (USB_OTG_CORE_HANDLE *pdev, uint32_t Cmd, uint8_t* Buf, uint32_t Len)
{
	USB_SETUP_REQ *req = (USB_SETUP_REQ *)Buf;   // for No Data request
  switch (Cmd)
  {
  case SEND_ENCAPSULATED_COMMAND:
    DBG("SEND_ENCAPSULATED_COMMAND\r\n");
    break;

  case GET_ENCAPSULATED_RESPONSE:
    DBG("GET_ENCAPSULATED_RESPONSE\r\n");
    break;

  case SET_COMM_FEATURE:
    DBG("SET_COMM_FEATURE\r\n");
    break;

  case GET_COMM_FEATURE:
    DBG("GET_COMM_FEATURE\r\n");
    break;

  case CLEAR_COMM_FEATURE:
    DBG("CLEAR_COMM_FEATURE\r\n");
    break;

  case SET_LINE_CODING:
    DBG("SET_LINE_CODING: speed=");
    linecoding.bitrate = (uint32_t)(Buf[0] | (Buf[1] << 8) | (Buf[2] << 16) | (Buf[3] << 24));
    linecoding.format = Buf[4];
    linecoding.paritytype = Buf[5];
    linecoding.datatype = Buf[6];
    #ifdef EBUG
    printInt(STR(&linecoding.bitrate), 4);
    #endif
    DBG("\r\n");
    /* Set the new configuration */
 //  VCP_COMConfig(OTHER_CONFIG);
    break;

  case GET_LINE_CODING:
    DBG("GET_LINE_CODING\r\n");
    Buf[0] = (uint8_t)(linecoding.bitrate);
    Buf[1] = (uint8_t)(linecoding.bitrate >> 8);
    Buf[2] = (uint8_t)(linecoding.bitrate >> 16);
    Buf[3] = (uint8_t)(linecoding.bitrate >> 24);
    Buf[4] = linecoding.format;
    Buf[5] = linecoding.paritytype;
    Buf[6] = linecoding.datatype;
    break;

  case SET_CONTROL_LINE_STATE:
    if(req->wValue){ // terminal is opened
		if(USB_active[0] == pdev){
			outp_active[0] = 1;
			DBG("\r\n\tactive output: 0\r\n");
		}else{
			outp_active[1] = 1;
			DBG("\r\n\tactive output: 1\r\n");
		}
	}else{ // terminal is closed
		if(USB_active[0] == pdev) outp_active[0] = 0;
		else outp_active[1] = 0;
		//pdev->dev.device_old_status = pdev->dev.device_status; // reset status
		//pdev->dev.device_status  = USB_OTG_ADDRESSED;
	}
    DBG("SET_CONTROL_LINE_STATE: req->wValue=");
    #ifdef EBUG
    printInt(STR(&req->wValue), 2);
	#endif
    DBG("\r\n");
    break;

  case SEND_BREAK:
    DBG("SEND_BREAK: ");
    #ifdef EBUG
    printInt(STR(&req->wValue), 2);
    #endif
    DBG("\r\n");
    break;

  default:
    DBG("UNK\r\n");
    break;
  }

  return USBD_OK;
}

/**
  * @brief  VCP_DataTx
  *         CDC received data to be send over USB IN endpoint are managed in
  *         this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
uint16_t VCP_DataTx(USB_OTG_CORE_HANDLE *pdev, uint8_t* Buf, uint32_t Len){
	uint32_t i;
	if(!pdev) return USBD_FAIL;
	//loop through buffer
	for(i = 0; i < Len; i++){
		if(USBD_FAIL == USB_Send_Data(pdev, Buf[i])) return USBD_FAIL;
	}
  return USBD_OK;
}

/**
 *  a simple interface to send the only byte to pdev
 */
uint16_t USB_Send_Data(USB_OTG_CORE_HANDLE *pdev, uint8_t byte){
	if(pdev->dev.device_status != USB_OTG_CONFIGURED || !pdev) return USBD_FAIL;
	if(USB_active[0] == pdev){ // current device #0
		if(outp_active[0] == 0)
			return USBD_FAIL;
	}else // current device #1
		if(outp_active[1] == 0)
			return USBD_FAIL;
	APP_Rx_Buffer[APP_Rx_ptr_in++] = byte;
	if(APP_Rx_ptr_in == APP_RX_DATA_SIZE) APP_Rx_ptr_in = 0;
	return USBD_OK;
}
/**
  * @brief  VCP_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else VCP_FAIL
  */
static uint16_t VCP_DataRx(USB_OTG_CORE_HANDLE *pdev, uint8_t* Buf, uint32_t Len){
	parce_incoming_buf(Buf, Len);
	//send received data back to sender
	//VCP_DataTx(pdev, Buf, Len );
	return USBD_OK;
}



/*********** Portions COPYRIGHT 2012 Embest Tech. Co., Ltd.*****END OF FILE****/
