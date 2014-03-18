/*
 * hw_config.c - USB & USART configuration
 *
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

#include "hw_config.h"

#define USART1_IRQHandler Dummy
#define USART2_IRQHandler Dummy
#define USART3_IRQHandler Dummy
void Dummy(){};

/**
 * Configure GPIO ports
 * @param mode  - port mode: GPIO_Mode_IN/GPIO_Mode_OUT/GPIO_Mode_AF/GPIO_Mode_AN
 * @param GPIOx - port address
 * @param pin   - pin[s] to configure
 * @param spd   - GPIO speed: 0..3 (2MHz..100MHz)
 * @param otype - output type: GPIO_OType_PP / GPIO_OType_OD
 * @param pupd  - PP resistors state: GPIO_PuPd_NOPULL/GPIO_PuPd_UP/GPIO_PuPd_DOWN
 */
void GPIO_pin_config_common(GPIOMode_TypeDef mode, GPIO_TypeDef* GPIOx,
		uint16_t pin, GPIOSpeed_TypeDef spd,
		GPIOOType_TypeDef otype, GPIOPuPd_TypeDef pupd){
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Speed = spd;
	GPIO_InitStructure.GPIO_Pin = pin;
	GPIO_InitStructure.GPIO_Mode = mode;
	GPIO_InitStructure.GPIO_OType = otype;
	GPIO_InitStructure.GPIO_PuPd = pupd;
	GPIO_Init(GPIOx, &GPIO_InitStructure);
}

// macros for default PP pin configuration with no PuPd
#define GPIO_pin_config(a,b,c)  \
		GPIO_pin_config_common(a,b,c, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL)
#define GPIO_pin_config_fast(a,b,c) \
		GPIO_pin_config_common(a,b,c, GPIO_Speed_100MHz, GPIO_OType_PP, GPIO_PuPd_NOPULL)

void Ports_Config(){
	EXTI_InitTypeDef EXTI_InitStructure;
	// enable clocks for used ports
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	/*
	 * Button (A0)
	 */
	GPIO_pin_config(GPIO_Mode_IN, GPIOA, GPIO_Pin_0);
	// Connect Button EXTI Line to Button GPIO Pin
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
	// Configure Button EXTI line
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	// Enable Button EXTI Interrupt
	NVIC_EnableIRQ(EXTI0_IRQn);
	/*
	 * LED (PC13) pin
	 */
	GPIO_pin_config(GPIO_Mode_OUT, GPIOC, GPIO_Pin_13);
	/*
	 * USB timing
	 */
	#ifdef USE_USB
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE);
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_OTG_HS, ENABLE);
	#endif
}



/*
 * USB configuration functions (for SPL)
 */
#ifdef USE_USB
void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev){
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	#ifdef USBPA
	/*
	 * PORT 1
	 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	// PA11 - OTG_FS DM, PA12 - OTG_FS DP
	GPIO_pin_config_fast(GPIO_Mode_AF, GPIOA, GPIO_Pin_11 | GPIO_Pin_12);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_OTG1_FS);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource12,GPIO_AF_OTG1_FS);
	// PA9 - OTG_FS_VBUS
	GPIO_pin_config_common(GPIO_Mode_IN, GPIOA, GPIO_Pin_9, GPIO_Speed_100MHz,
						GPIO_OType_OD, GPIO_PuPd_NOPULL);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_OTG1_FS);
	// PA10 - OTG_FS_ID
	GPIO_pin_config_common(GPIO_Mode_IN, GPIOA, GPIO_Pin_10, GPIO_Speed_100MHz,
						GPIO_OType_OD, GPIO_PuPd_UP);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_OTG1_FS);
	// clocking
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE);
	// wakeup by usb connection power
	EXTI_InitStructure.EXTI_Line = EXTI_Line18;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	EXTI_ClearITPendingBit(EXTI_Line18);
	#endif // USBPA

	#ifdef USBPB
	/*
	 * PORT 2
	 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	// PB14 - OTG_HS DM, PB15 - OTG_FS DP
	GPIO_pin_config_fast(GPIO_Mode_AF, GPIOB, GPIO_Pin_14 | GPIO_Pin_15);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_OTG2_FS);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_OTG2_FS);
	// PB13 - OTG_HS_VBUS
	GPIO_pin_config_common(GPIO_Mode_IN, GPIOB, GPIO_Pin_13, GPIO_Speed_100MHz,
						GPIO_OType_OD, GPIO_PuPd_NOPULL);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_OTG2_FS);
	// PB12 - OTG_HS_ID
	GPIO_pin_config_common(GPIO_Mode_IN, GPIOB, GPIO_Pin_12, GPIO_Speed_100MHz,
						GPIO_OType_OD, GPIO_PuPd_UP);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource12,GPIO_AF_OTG2_FS);
	// clocking
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_OTG_HS, ENABLE);
	// wakeup by usb connection power
	EXTI_InitStructure.EXTI_Line = EXTI_Line20;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	EXTI_ClearITPendingBit(EXTI_Line20);
	#endif // USBPB

	RCC_APB1PeriphResetCmd(RCC_APB1Periph_PWR, ENABLE);

}

void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev){
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	#ifdef USBPA
	NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	#endif
	#ifdef USBPB
	NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	#endif
}
void USB_OTG_BSP_uDelay (const uint32_t usec){
	uint32_t count = 0;
	const uint32_t utime = (120 * usec / 7);
	do{
		if( ++count > utime ) return ;
	}while (1);
}
void USB_OTG_BSP_mDelay (const uint32_t msec){
	USB_OTG_BSP_uDelay(msec * 1000);
}
#endif // USE_USB

/*
 * ETH config
 */
#ifdef USE_ETH
volatile uint32_t  EthInitStatus = 0;
volatile uint8_t EthLinkStatus = 0;

static void ETH_GPIO_Config();
static void ETH_MACDMA_Config();

void ETH_BSP_Config(){
	//RCC_ClocksTypeDef RCC_Clocks;
	ETH_GPIO_Config();
	ETH_MACDMA_Config();
  /* // Configure Systick clock source as HCLK
  SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

  // SystTick configuration: an interrupt every 10ms
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 100); */
}

static void ETH_MACDMA_Config(void){
	ETH_InitTypeDef ETH_InitStructure;
	/* Enable ETHERNET clock  */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx |
					RCC_AHB1Periph_ETH_MAC_Rx | RCC_AHB1Periph_ETH_MAC_PTP, ENABLE);
	/* Reset ETHERNET on AHB Bus */
	ETH_DeInit();
	/* Software reset */
	ETH_SoftwareReset();
	/* Wait for software reset */
	while (ETH_GetSoftwareResetStatus() == SET);
	/* ETHERNET Configuration --------------------------------------------------*/
	/* Call ETH_StructInit if you don't like to configure all ETH_InitStructure parameter */
	ETH_StructInit(&ETH_InitStructure);
	/*------------------------   MAC   -----------------------------------*/
	// HERE ARE NON-default values:
	//ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable;
	//ETH_InitStructure.ETH_Speed = ETH_Speed_10M;
	//ETH_InitStructure.ETH_Mode = ETH_Mode_HalfDuplex;
	//ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Enable;
	//ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Enable;
	//ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Enable;
	//ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Enable;
	//ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_None;
	//ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_HashTable;
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
	#ifdef CHECKSUM_BY_HARDWARE
		ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
	#endif
	/*------------------------   DMA   -----------------------------------*/
	/* When we use the Checksum offload feature, we need to enable the Store and Forward mode:
	the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum,
	if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
	//ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Disable;
	//ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Disable;
	//ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Enable;
	//ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Enable;
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
	//ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Disable;
	//ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Disable;
	//ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_8Beat;
	//ETH_InitStructure.ETH_TxDMABurstLength = ETH_RxDMABurstLength_8Beat;
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
	/* Configure Ethernet */
	EthInitStatus = ETH_Init(&ETH_InitStructure, LAN8720_PHY_ADDRESS);
}

void ETH_GPIO_Config(void){
	//GPIO_InitTypeDef GPIO_InitStructure;
	// Enable GPIOs clocks
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |	RCC_AHB1Periph_GPIOB |
						RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOG, ENABLE);
	// Enable SYSCFG clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	// Select RMII Interface
	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);
	// ETHERNET pins configuration
	/*
	 * *********** PA ***********
	 * ETH_RMII_REF_CLK: PA1
	 * ETH_RMII_MDIO:    PA2
	 * ETH_RMII_MDINT:   PA3
	 * ETH_RMII_CRS_DV:  PA7
   */
	GPIO_pin_config_fast(GPIO_Mode_AF, GPIOA,
							GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7);
	// Alternate functions
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);
	/*
	 * *********** PG ***********
	 * ETH_RMII_TX_EN: PG11
	 * ETH_RMII_TXD0:  PG13
	 * ETH_RMII_TXD1:  PG14
	 */
	GPIO_pin_config_fast(GPIO_Mode_AF, GPIOG,
						GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14);
	// Alternate functions
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_ETH);
	/*
	 * *********** PC ***********
	 * ETH_RMII_MDC:  PC1
	 * ETH_RMII_RXD0: PC4
	 * ETH_RMII_RXD1: PC5
	 */
	GPIO_pin_config_fast(GPIO_Mode_AF, GPIOC,
						GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5);
	// Alternate functions
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);
}
#endif // USE_ETH

#ifdef USEAD7794
/**
 * Initialize SPI to master mode
 */
volatile uint8_t aTxBuffer[DMA_BUFFERSIZE];
volatile uint8_t aRxBuffer[DMA_BUFFERSIZE];

volatile uint8_t SPI_EOT_FLAG = 1; // end of transmission flag, set by DMA interrupt

/**
 * Configure SPI & DMA
 */
void SPI_Config(){
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

	SPIx_CLK_INIT(SPIx_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(SPIx_SCK_GPIO_CLK | SPIx_MISO_GPIO_CLK | SPIx_MOSI_GPIO_CLK, ENABLE);
	RCC_AHB1PeriphClockCmd(SPIx_DMA_CLK, ENABLE);

	GPIO_PinAFConfig(SPIx_SCK_GPIO_PORT, SPIx_SCK_SOURCE, SPIx_SCK_AF);
	GPIO_PinAFConfig(SPIx_MISO_GPIO_PORT, SPIx_MISO_SOURCE, SPIx_MISO_AF);
	GPIO_PinAFConfig(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_SOURCE, SPIx_MOSI_AF);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

	GPIO_InitStructure.GPIO_Pin = SPIx_SCK_PIN;
	GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  SPIx_MISO_PIN;
	GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  SPIx_MOSI_PIN;
	GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/* SPI configuration -------------------------------------------------------*/
	SPI_I2S_DeInit(SPIx);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_Init(SPIx, &SPI_InitStructure);

	/* Configure DMA Initialization Structure */
	DMA_DeInit(SPIx_TX_DMA_STREAM);
	DMA_DeInit(SPIx_RX_DMA_STREAM);
	DMA_InitStructure.DMA_BufferSize = DMA_BUFFERSIZE ;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(SPIx->DR)) ;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	/* Configure TX DMA */
	DMA_InitStructure.DMA_Channel = SPIx_TX_DMA_CHANNEL ;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
	DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)aTxBuffer ;
	DMA_Init(SPIx_TX_DMA_STREAM, &DMA_InitStructure);
	/* Configure RX DMA */
	DMA_InitStructure.DMA_Channel = SPIx_RX_DMA_CHANNEL ;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
	DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)aRxBuffer ;
	DMA_Init(SPIx_RX_DMA_STREAM, &DMA_InitStructure);

	// Enable DMA SPI TX Stream
	DMA_Cmd(SPIx_TX_DMA_STREAM,ENABLE);
	// Enable DMA SPI RX Stream
	DMA_Cmd(SPIx_RX_DMA_STREAM,ENABLE);
	// Enable SPI DMA TX Requests
	SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Tx, ENABLE);
	// Enable SPI DMA RX Requests
	SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Rx, ENABLE);
	// Enable the SPI peripheral
	SPI_Cmd(SPIx, ENABLE);

	// Enable DMA Tx IRQ
	NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);
}

volatile uint8_t *copy_buf(volatile uint8_t *dest, uint8_t *src, uint8_t len){
	uint8_t i;
	for(i = 0; i < len; i++)
		*dest++ = *src++;
	for(i = len; i < DMA_BUFFERSIZE; i++)
		*dest++ = NOT_WEN;
	return dest;
}

/**
 * Write data to SPI by DMA
 * @param data - buffer with data
 * @param len  - buffer length (<= DMA_BUFFERSIZE)
 * @return 0 in case of error (or 1 in case of success)
 */
uint8_t write_SPI(uint8_t *data, uint8_t len){
	#ifdef EBUG
	DBG("write to SPI: ");
	printInt(data, len);
	#endif
	uint32_t tend = LocalTime + 50; // we will wait for end of previous transmission not more than 50ms
	while(!SPI_EOT_FLAG && LocalTime < tend); // wait for previous DMA interrupt
	if(SPI_EOT_FLAG) return 0; // error: there's no receiver???
	if(len > DMA_BUFFERSIZE) len = DMA_BUFFERSIZE;
	SPI_EOT_FLAG = 0;
	copy_buf(aTxBuffer, data, len);
	return 1;
}

/**
 * read data from SPI
 * @return buffer with data or NULL
 */
volatile uint8_t *read_SPI(){
	if(!SPI_EOT_FLAG) return NULL;
	#ifdef EBUG
	DBG("SPI buffer: ");
	printInt(aRxBuffer, 4);
	#endif
	return aRxBuffer;
}

#endif // USEAD7794
