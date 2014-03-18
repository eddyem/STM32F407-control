/*
 * user_protocol.c
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

/**
 * Parce incoming buffer with commands
 *
 * @param Buf (i) - buffer to parce
 * @param Len     - its length
 */
void parce_incoming_buf(uint8_t* Buf, uint32_t Len){
	uint32_t i;
	//loop through buffer
	for (i = 0; i < Len; i++){
		uint8_t ch = Buf[i];
		switch(ch){
			case SYSTEM_RESET: // this is a full system reset, so we can do it right here!
				NVIC_SystemReset();
			break;
			case LED_MINUS:
				if(LED_delay > 50) LED_delay -= 50;
				else LED_delay = 1000;
			break;
			case LED_PLUS:
				if(LED_delay < 1000) LED_delay += 50;
				else LED_delay = 50;
			break;
			case ETH_RESET:
				FLAG |= FLAG_ETH_RESET;
			break;
			case READ_ADC:
				FLAG |= FLAG_READ_ADC;
			break;
			case RESET_ADC:
				FLAG |= FLAG_RESET_ADC;
			break;
			case INIT_PERIPH:
				FLAG |= FLAG_INIT_PERIPH;
			break;
		}
		#ifdef USE_USB
		USB_Send_Data(USB_active[0], ch);
		#endif
	}
}
