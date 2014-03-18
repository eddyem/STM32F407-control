/*
 * interrupts.c - interrupts handlers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "main.h"

void NMI_Handler(){}
void HardFault_Handler(){while (1){};}
void MemManage_Handler(){while (1){};}
void BusFault_Handler(){while (1){};}
void UsageFault_Handler(){while (1){};}
void SVC_Handler(){}
void DebugMon_Handler(){}
void PendSV_Handler(){}

//volatile uint16_t delay_counter = 0;
// Delay in ms
void Delay(uint32_t time){
	uint32_t timingdelay = LocalTime + time;
	while(timingdelay > LocalTime);
}
// SysTick: delay counter
void SysTick_Handler(){
	LocalTime++;// += SYSTEMTICK_PERIOD_MS;
}

/*
 * peripherial interrupts
 */
// button EXTernal Interrupt - change LED blink delay
void EXTI0_IRQHandler(){
	if(LED_delay > 50) LED_delay -= 50;
	else LED_delay = 1000;
	EXTI_ClearITPendingBit(EXTI_Line0);
}


/*
 * USB
 */
#ifdef USE_USB
#if defined USE_USB_OTG_FS
void OTG_FS_WKUP_IRQHandler(){
	if(USB_OTG_devA.cfg.low_power){
		*(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
		Set_System();
		#ifdef USBPA
		USB_OTG_UngateClock(&USB_OTG_devA);
		#endif
		#ifdef USBPB
		USB_OTG_UngateClock(&USB_OTG_devB);
		#endif
	}
	#ifdef USBPA
	EXTI_ClearITPendingBit(EXTI_Line18);
	#endif
	#ifdef USBPB
	EXTI_ClearITPendingBit(EXTI_Line20);
	#endif
}
void OTG_FS_IRQHandler(){
	#ifdef USBPA
	USBD_OTG_ISR_Handler(&USB_OTG_devA);
	#endif
	#ifdef USBPB
	USBD_OTG_ISR_Handler(&USB_OTG_devB);
	#endif
}
#endif // USE_USB_OTG_FS

#ifdef USBPB
void OTG_HS_IRQHandler(){
	USBD_OTG_ISR_Handler(&USB_OTG_devB);
}
#endif // USBPB

#if defined USE_USB_OTG_HS && defined USBPB
void OTG_HS_WKUP_IRQHandler(){
	if(USB_OTG_devB.cfg.low_power){
		*(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
		Set_System();
		USB_OTG_UngateClock(&USB_OTG_devB);
	}
	EXTI_ClearITPendingBit(EXTI_Line20);
}
#endif // defined USE_USB_OTG_HS && defined USBPB

#ifdef USEAD7794
void SPIx_DMA_TX_IRQHandler(){
	if(DMA_GetITStatus(SPIx_TX_DMA_STREAM, SPIx_TX_DMA_IT_TCIF) == SET){ // Tx DMA interrupt
		DMA_ClearITPendingBit(SPIx_TX_DMA_STREAM, SPIx_TX_DMA_IT_TCIF);
		SPI_EOT_FLAG = 1;
	}
}
/* in case of non-DMA
void SPIx_IRQHANDLER(){
	// SPI in Receiver mode
	if(SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_RXNE) == SET){
		if(ubRxIndex < BUFFERSIZE){
		// Receive Transaction data
			aRxBuffer[ubRxIndex++] = SPI_I2S_ReceiveData(SPIx);
		}else{
		// Disable the Rx buffer not empty interrupt
			SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, DISABLE);
		}
	}
	// SPI in Tramitter mode
	if(SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_TXE) == SET){
		if (ubTxIndex < BUFFERSIZE){
		// Send Transaction data
			SPI_I2S_SendData(SPIx, aTxBuffer[ubTxIndex++]);
		}else{
		// Disable the Tx buffer empty interrupt
			SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_TXE, DISABLE);
		}
	}
}*/
#endif // USEAD7794

#endif // USE_USB

#define SPIx_DMA_TX_IRQn               DMA2_Stream3_IRQn
#define SPIx_DMA_TX_IRQHandler         DMA2_Stream3_IRQHandler
#define SPIx_TX_DMA_IT_TCIF            DMA_IT_TCIF3
