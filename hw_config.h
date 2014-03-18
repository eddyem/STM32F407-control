/*
 * hw_config.h - some definitions of hardware configuration, be careful:
 *      interrupts.c also have hardware-dependent parts!
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

#pragma once
#ifndef __HW_CONFIG_H__
#define __HW_CONFIG_H__

/*
 * USB ports
 */
// define USBPA and/or USBPB to use port A (FS) or/and port B (HS in FS mode)
#if defined USBPA || defined USBPB
	#define USE_USB
#endif // USBPA||USBPB

#include "main.h"
/*
 * here nailed next config pins:
 *
 * User button:			PA0
 * LED:					PC13
 */
#define LED_GPIOx	GPIOC
#define LED_PIN		GPIO_Pin_13
void Ports_Config();
#ifdef USE_ETH
	void ETH_BSP_Config();
#endif
/*
 * network configuration
 */
#ifdef USE_ETH
#define LAN8710a_PHY_ADDRESS       0x01
extern volatile uint32_t  EthInitStatus;
extern volatile uint8_t EthLinkStatus;
#endif // USE_ETH

/*
 * SPI configuration
 */
#ifdef USEAD7794
// buffers
extern volatile uint8_t aTxBuffer[];
extern volatile uint8_t aRxBuffer[];
// flag "end of transmission"
extern volatile uint8_t SPI_EOT_FLAG;

void SPI_Config();
uint8_t write_SPI(uint8_t *data, uint8_t len);
volatile uint8_t *read_SPI();

#define DMA_BUFFERSIZE					4
// SPI ports:   (SPI1)
#define SPIx                           SPI1
#define SPIx_CLK                       RCC_APB2Periph_SPI1
#define SPIx_CLK_INIT                  RCC_APB2PeriphClockCmd
#define SPIx_IRQn                      SPI1_IRQn
#define SPIx_IRQHANDLER                SPI1_IRQHandler
		// SCK: PA5 // PI1
#define SPIx_SCK_PIN                   GPIO_Pin_5
#define SPIx_SCK_GPIO_PORT             GPIOA
#define SPIx_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOA
#define SPIx_SCK_SOURCE                GPIO_PinSource5
#define SPIx_SCK_AF                    GPIO_AF_SPI1
		// MISO: PA6 // PI2
#define SPIx_MISO_PIN                  GPIO_Pin_6
#define SPIx_MISO_GPIO_PORT            GPIOA
#define SPIx_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOA
#define SPIx_MISO_SOURCE               GPIO_PinSource6
#define SPIx_MISO_AF                   GPIO_AF_SPI1
		// MOSI: PB5 // PI3
#define SPIx_MOSI_PIN                  GPIO_Pin_5
#define SPIx_MOSI_GPIO_PORT            GPIOB
#define SPIx_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define SPIx_MOSI_SOURCE               GPIO_PinSource5
#define SPIx_MOSI_AF                   GPIO_AF_SPI1
		// SPI DMA
#define SPIx_DMA                       DMA2
#define SPIx_DMA_CLK                   RCC_AHB1Periph_DMA2
#define SPIx_TX_DMA_CHANNEL            DMA_Channel_3
#define SPIx_TX_DMA_STREAM             DMA2_Stream3
#define SPIx_TX_DMA_FLAG_TCIF          DMA_FLAG_TCIF3
#define SPIx_RX_DMA_CHANNEL            DMA_Channel_3
#define SPIx_RX_DMA_STREAM             DMA2_Stream0
#define SPIx_RX_DMA_FLAG_TCIF          DMA_FLAG_TCIF0
		// SPI DMA IRQs
#define SPIx_DMA_TX_IRQn               DMA2_Stream3_IRQn
#define SPIx_DMA_TX_IRQHandler         DMA2_Stream3_IRQHandler
#define SPIx_TX_DMA_IT_TCIF            DMA_IT_TCIF3

/*
// SPI ports:   (SPI2 // SPI1)
#define SPIx                           SPI2 // SPI1
#define SPIx_CLK                       RCC_APB1Periph_SPI2 // RCC_APB2Periph_SPI1
#define SPIx_CLK_INIT                  RCC_APB1PeriphClockCmd // RCC_APB2PeriphClockCmd
#define SPIx_IRQn                      SPI2_IRQn // SPI1_IRQn
#define SPIx_IRQHANDLER                SPI2_IRQHandler // SPI1_IRQHandler
		// SCK: PI1 // PA5
#define SPIx_SCK_PIN                   GPIO_Pin_1 // GPIO_Pin_5
#define SPIx_SCK_GPIO_PORT             GPIOI // GPIOA
#define SPIx_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOI // RCC_AHB1Periph_GPIOA
#define SPIx_SCK_SOURCE                GPIO_PinSource1 // GPIO_PinSource5
#define SPIx_SCK_AF                    GPIO_AF_SPI2 // GPIO_AF_SPI1
		// MISO: PI2 // PA6
#define SPIx_MISO_PIN                  GPIO_Pin_2 // GPIO_Pin_6
#define SPIx_MISO_GPIO_PORT            GPIOI // GPIOA
#define SPIx_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOI // RCC_AHB1Periph_GPIOA
#define SPIx_MISO_SOURCE               GPIO_PinSource2 // GPIO_PinSource6
#define SPIx_MISO_AF                   GPIO_AF_SPI2 // GPIO_AF_SPI1
		// MOSI: PI3 // PB5
#define SPIx_MOSI_PIN                  GPIO_Pin_3 // GPIO_Pin_5
#define SPIx_MOSI_GPIO_PORT            GPIOI // GPIOB
#define SPIx_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOI // RCC_AHB1Periph_GPIOB
#define SPIx_MOSI_SOURCE               GPIO_PinSource3 // GPIO_PinSource5
#define SPIx_MOSI_AF                   GPIO_AF_SPI2 // GPIO_AF_SPI1
		// SPI DMA
#define SPIx_DMA                       DMA1 // DMA2
#define SPIx_DMA_CLK                   RCC_AHB1Periph_DMA1 // RCC_AHB1Periph_DMA2
#define SPIx_TX_DMA_CHANNEL            DMA_Channel_0 // DMA_Channel_3
#define SPIx_TX_DMA_STREAM             DMA1_Stream4 // DMA2_Stream3
#define SPIx_TX_DMA_FLAG_TCIF          DMA_FLAG_TCIF4 // DMA_FLAG_TCIF3
#define SPIx_RX_DMA_CHANNEL            DMA_Channel_0 // DMA_Channel_3
#define SPIx_RX_DMA_STREAM             DMA1_Stream3 // DMA2_Stream0
#define SPIx_RX_DMA_FLAG_TCIF          DMA_FLAG_TCIF3 // DMA_FLAG_TCIF0
*/
#endif // USEAD7794
#endif // __HW_CONFIG_H__
