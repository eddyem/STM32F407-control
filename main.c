/*
 * main.c
 *
 * Copyright 2013 Edward V. Emelianoff <eddy@sao.ru>
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

volatile uint16_t LED_delay = 1000; // led blinking delay in ms
// array of pointers to USB handlers, if active == USB_OTG_devX,
//    circular: USB_active[0] always points to first or only active buffer
#ifdef USE_USB
USB_OTG_CORE_HANDLE *USB_active[2] = {NULL, NULL};
#endif
#ifdef USBPA
USB_OTG_CORE_HANDLE USB_OTG_devA;
#endif
#ifdef USBPB
USB_OTG_CORE_HANDLE USB_OTG_devB;
#endif

volatile uint32_t LocalTime = 0;

volatile uint32_t FLAG = 0;
#ifdef USE_ETH
static uint32_t  eth_on = 0;
#endif

#ifdef USEAD7794
static uint32_t ad7794_on = 0;
#endif

#ifdef USE_ETH
void ETH_LWIP_init(){
	if(EthInitStatus == 0){
		DBG("Error: can't initialize ethernet!\r\n");
	}else{
		if(!eth_on){
			if(LwIP_Init() && tcp_echoserver_init()) eth_on = 1;
		}
	}
}
#endif

#ifdef USEAD7794
void ADC_init(){
	ad7794_on = 0;
	if(!setup_AD7794(1)){
		DBG("Error: can't initialize AD7794!\r\n");
	}else{
		ad7794_on = 1;
	}
}
#endif // USEAD7794

int main(){
	uint32_t nxt_LED_swch;
	Set_System();
	nxt_LED_swch = LocalTime + LED_delay;
	while(1){
		if(nxt_LED_swch <= LocalTime){
			ToggleBit(LED_GPIOx, LED_PIN);
			nxt_LED_swch = LocalTime + LED_delay;
			//P("toggle!\r\n");
			//newline();
		}
		if(FLAG & FLAG_INIT_PERIPH){
			FLAG ^= FLAG_INIT_PERIPH;
			#ifdef USE_ETH
			ETH_LWIP_init();
			#endif
			#ifdef USEAD7794
			ADC_init();
			#endif // USEAD7794
		}
		#ifdef USE_ETH
		if(FLAG & FLAG_ETH_RESET){
			FLAG ^= FLAG_ETH_RESET;
			ETH_SoftwareReset();
			ETH_BSP_Config();
			ETH_LWIP_init();
		}
		if(eth_on){
			if(ETH_CheckFrameReceived()){
				LwIP_Pkt_Handle();
			}
			LwIP_Periodic_Handle(LocalTime);
		}
		#endif // USE_ETH
		#ifdef USEAD7794
		if(ad7794_on){
			if(FLAG & FLAG_READ_ADC){
				FLAG ^= FLAG_READ_ADC;
				uint32_t ADCval = read_AD7794(1);
				printInt(STR(ADCval), 4);
			}
			if(FLAG & FLAG_RESET_ADC){
				FLAG ^= FLAG_RESET_ADC;
				reset_AD7794();
				ADC_init();
			}
		}
		#endif // USEAD7794
	}
}

inline void Set_System(){
	// SysTick setup: 1 ms
	SysTick_Config(SystemCoreClock / 1000);
	// Ports configuration
	Ports_Config();
#ifdef USBPA
	USBD_Init(&USB_OTG_devA,USB_OTG_FS_CORE_ID,&USR_desc,&USBD_CDC_cb,&USR_cb);
#endif
#ifdef USBPA
	USBD_Init(&USB_OTG_devB,USB_OTG_HS_CORE_ID,&USR_desc,&USBD_CDC_cb,&USR_cb);
#endif
#ifdef USE_ETH
	ETH_BSP_Config();
#endif
#ifdef USEAD7794
	SPI_Config();
#endif // USEAD7794
}

/*
 * Send char array wrd thru USB
 */
void prnt(uint8_t *wrd){
#ifdef USE_USB
	if(!wrd || !USB_active[0]) return;
	uint8_t *p = wrd;
	uint32_t L = 0;
	while(*p++) L++;
	VCP_DataTx(USB_active[0], wrd, L);
	//VCP_DataTx(&USB_OTG_devA, wrd, L);
#endif // USE_USB
}

size_t strlen(const char *s){
	size_t L = 0;
	while(*s++) L++;
	return L;
}

/**
 * Send to terminal an integer value
 * if "more messages" mode == 1, value will be in hex,
 * else value will be printed as BigEndian binary
 * @param val - value itself
 * @param len - bytelength of val
 */
void printInt(_U_ volatile uint8_t *val, _U_ int8_t len){
	#ifdef USE_USB
	P("0x");
	void putch(uint8_t c){
		if(c < 10)
			USB_Send_Data(USB_active[0], c + '0');
		else
			USB_Send_Data(USB_active[0], c + 'a' - 10);
	}
	while(--len >= 0){
		putch(val[len] >> 4);
		putch(val[len] & 0x0f);
	}
	USB_Send_Data(USB_active[0], ' ');
	#endif
}
