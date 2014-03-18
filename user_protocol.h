/*
 * user_protocol.h
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


#pragma once
#ifndef __USER_PROTOCOL_H__
#define __USER_PROTOCOL_H__

/*
 * User commands definition
 */
#define LED_MINUS		'-'
#define LED_PLUS		'+'
#define INIT_PERIPH		'p'
#define ETH_RESET		'e'
#define READ_ADC		'a'
#define RESET_ADC		'A'
#define SYSTEM_RESET	'R'

/*
 * state flag
 */
extern volatile uint32_t FLAG;
// its parameters
#define FLAG_ETH_RESET		U32(0x0001)
#define FLAG_READ_ADC		U32(0x0002)
#define FLAG_RESET_ADC		U32(0x0004)
#define FLAG_INIT_PERIPH	U32(0x0008)

void parce_incoming_buf(uint8_t* Buf, uint32_t Len);

#endif // __USER_PROTOCOL_H__
