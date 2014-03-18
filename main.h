/*
 * main.h
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
#ifndef __MAIN_H__
#define __MAIN_H__
#include "stm32f4xx_conf.h"
#include "hw_config.h"
#include "interrupts.h"
#include <stdio.h>
#include "user_protocol.h"
#ifdef USE_USB
	#include "usb_conf.h"
	#include "usbd_cdc_core.h"
	#include "usbd_usr.h"
	#include "usbd_desc.h"
	#include "usb_dcd_int.h"
	#include "usb_bsp.h"
	#include "usbd_cdc_vcp.h"
	#ifdef USBPA
		extern USB_OTG_CORE_HANDLE USB_OTG_devA;
	#endif
	#ifdef USBPB
		extern USB_OTG_CORE_HANDLE USB_OTG_devB;
	#endif
#endif // USE_USB
#include "tcp_echoserver.h" // we need it to normal compiling even without eth
#ifdef USE_ETH
	#include "stm32f4x7_eth.h"
	#include "lwipopts.h"
	#include "netconf.h"
#endif // USE_ETH
#ifdef USEAD7794
	#include "AD7794.h"
#endif // USEAD7794


#define _U_    __attribute__((__unused__))
#define STR(arg) ((uint8_t*)arg)

#define  U8(x)  ((uint8_t)  x)
#define U16(x)  ((uint16_t) x)
#define U32(x)  ((uint32_t) x)

inline void Set_System();
extern volatile uint16_t LED_delay;
extern volatile uint32_t LocalTime;

void prnt(uint8_t *wrd);
void printInt(volatile uint8_t *val, int8_t len);

#ifdef USE_USB
extern USB_OTG_CORE_HANDLE *USB_active[2];
#endif

size_t strlen(const char *s);
//void memcpy(void *dest, const void *src, uint32_t n);

#define P(arg) prnt((uint8_t*)arg)
#define newline()  do{P("\r\n");}while(0)

#ifdef EBUG
	#define DBG(a) do{P(a);}while(0)
#else
	#define DBG(a)
#endif

#endif // __MAIN_H__
