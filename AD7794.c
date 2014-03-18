/*
 * AD7794.c - routines to work with ADC AD7794 by SPI
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
#ifdef USEAD7794

volatile uint8_t data_error = 0;

/**
 * Some functions to perform 1, 2 and 3-byte transmition over SPI
 * @param data - data to transmit
 * @return received data
 */
#define ERR_SPI() do{data_error = 1; return 0;}while(0)
uint8_t sendByte(uint8_t data){
	volatile uint8_t *ptr;
	if(!write_SPI(&data, 1))
		ERR_SPI();
	ptr = read_SPI();
	if(ptr) return *ptr;
	else
		ERR_SPI();
}
uint16_t sendWord(uint16_t data){
	volatile uint16_t *ptr;
	if(!write_SPI((uint8_t*)&data, 2))
		ERR_SPI();
	ptr = (uint16_t *)(read_SPI());
	if(ptr) return *ptr;
	else
		ERR_SPI();
}
uint32_t sendDWord(uint32_t data){
	volatile uint32_t x = data, *ptr;
	if(!write_SPI((uint8_t*)&x, 4))
		ERR_SPI();
	ptr = (uint32_t *)(read_SPI());
	if(ptr) return *ptr;
	else
		ERR_SPI();
}

#define send3bytes(data) sendDWord(U32(0x80000000) | data)

/**
 * Checks ending of ADC and/or errors
 * @return 1 in case of data ready, set data_error to 1 in case of error
 */
uint8_t check_data_ready(){
	uint8_t x = sendWord(U16(STAT_REGISTER | READ_FROM_REG) << 8);
	if(data_error) return 1;
	if(x & DATA_ERROR){
		data_error = 1;
		return 1;
	}
	if(x & DATA_NOTRDY) return 0;
	else return 1;
}

#define check_err()  if(data_error){data_error = 0; return 0;}

static uint32_t ADC_gain = U32(0x0200); // gain = 4, Vrange = Vref+-0.625V

/**
 * Changes ADC gain coefficient
 * @param gain - log2(gain)
 * gain	voltage range (Vref+-)
 * 0	2.5 V
 * 1	1.25 V
 * 2	625 mV
 * 3	312.5 mV
 * 4	156.2 mV
 * 5	78.125 mV
 * 6	39.06 mV
 * 7	19.53 mV
 * @return 0 in case of wrong gain value
 */
int change_AD7794_gain(uint8_t gain){
	if(gain > 7) return 0;
	ADC_gain = U32(gain) << 8;
	return 1;
}

/**
 * Setup ADC to single transform
 * @param channel - channel to activate
 * @return 0 on error
 */
uint8_t setup_AD7794(uint8_t channel){
	send3bytes(U32(CONF_REGISTER) << 16| // write to CONF_REGISTER
				ADC_gain | U32(EXTREFIN_1 | CHANNEL_MASK & U16(channel)));
	check_err();
	sendWord(U16(IO_REGISTER) << 8 |
				U16(IEXC_DIRECT | IEXC_1MA));
	check_err();
	send3bytes(U32(MODE_REGISTER) << 16|
				U32(INT_FS_CAL) | // make a full-scale calibration
				U32(FILTER_MASK & 5) );  // 50Hz update rate
	while(!check_data_ready());
	check_err();
	return 1;
}

uint32_t read_AD7794(uint8_t channel){
	uint32_t x;
	send3bytes(U32(CONF_REGISTER) << 16| // write to CONF_REGISTER
				ADC_gain | U32(EXTREFIN_1 | CHANNEL_MASK & U16(channel)));
	check_err();
	send3bytes(U32(MODE_REGISTER) << 16| // write to MODE_REGISTER
			U32(SINGLE_MODE)| // put ADC to a single conversion mode
			U32(FILTER_MASK & 5));
	check_err();
	while(!check_data_ready());
	x = sendDWord(U32(DATA_REGISTER | READ_FROM_REG) << 24 | // read from data register
			U32(NOT_WEN) << 16|
			U32(NOT_WEN) << 8 |
			U32(NOT_WEN) ); //
	check_err();
	return (x & U32(0xffffff)) | 1; // 0 is error! so we make 0->1
}

void reset_AD7794(){
	uint32_t r = 0xffffffff, i;
	for(i = 0; i < 8; i++) write_SPI((uint8_t*)&r, 4);
}

#endif // USEAD7794
