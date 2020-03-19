/*
 * Dcc.c
 *
 *  Created on: Mar 17, 2020
 *      Author: Fil
 */

#include "main.h"
#include "DccGlobal.h"

s_len3_packet			idle_packet;
s_len3_packet			reset_packet;
s_main_track 			main_track;
s_len3_packet 			len3_packet;
s_len4_packet 			len4_packet;
s_len5_packet 			len5_packet;
s_len6_packet 			len6_packet;
uint32_t 				*dcc_packet;
uint32_t				dcc_packet_size;
uint32_t 				*dcc_next_packet = NULL;
uint32_t				dcc_next_packet_size = 0;

extern	s_rxbuf			usb_rxbuf,available_packet;

void EncodeByte(uint16_t *dest , uint8_t value)
{
	uint8_t i,mask=0x80;
	for ( i=0;i<8;i++)
	{
		dest[i] = BIT_0;
		if (( value & mask) == mask)
			dest[i] = BIT_1;
		mask = mask >> 1;
	}
}

uint32_t EncodeThrottle(int cab,int speed,int direction)
{
int cab1,cab2,command,errcheck, retval=0;

	command = 0x40 | (direction << 5) | (speed & 0x1f);

	if ( cab > 127 )
	{
		retval=1;
		cab1 = (cab >> 8 ) | 0xc0;
		cab2 = cab & 0xff;
		errcheck = cab1 ^ cab2 ^ command;
		EncodeByte(len4_packet.byte1,cab1);
		EncodeByte(len4_packet.byte2,cab2);
		EncodeByte(len4_packet.byte3,command);
		EncodeByte(len4_packet.errcheck,errcheck);
	}
	else
	{
		retval=0;
		errcheck = cab ^ command;
		EncodeByte(len3_packet.byte1,cab);
		EncodeByte(len3_packet.byte2,command);
		EncodeByte(len3_packet.errcheck,errcheck);
	}
	return retval;
}

void InitDccPackets(void)
{
uint32_t	i;

	/* Prepares idle packets */
	for(i=0;i<PREAMBLE_LEN;i++)
		idle_packet.preamble[i] = BIT_1;
	EncodeByte(idle_packet.byte1,0xff);
	EncodeByte(idle_packet.byte2,0x00);
	EncodeByte(idle_packet.errcheck,0xff);
	idle_packet.byte1_start=BIT_0;
	idle_packet.byte2_start=BIT_0;
	idle_packet.errdet_start=BIT_0;
	for(i=0;i<CLOSER_LEN;i++)
		idle_packet.closer[i] = BIT_1;

	/* Prepares reset packets */
	for(i=0;i<PREAMBLE_LEN;i++)
		reset_packet.preamble[i] = BIT_1;
	EncodeByte(reset_packet.byte1,0x00);
	EncodeByte(reset_packet.byte2,0x00);
	EncodeByte(reset_packet.errcheck,0x00);
	reset_packet.byte1_start=BIT_0;
	reset_packet.byte2_start=BIT_0;
	reset_packet.errdet_start=BIT_0;
	for(i=0;i<CLOSER_LEN;i++)
		reset_packet.closer[i] = BIT_1;

	/* Prepare all the others */
	for(i=0;i<PREAMBLE_LEN;i++)
	{
		len3_packet.preamble[i] = BIT_1;
		len4_packet.preamble[i] = BIT_1;
		len5_packet.preamble[i] = BIT_1;
		len6_packet.preamble[i] = BIT_1;
	}
	for(i=0;i<CLOSER_LEN;i++)
	{
		len3_packet.closer[i] = BIT_1;
		len4_packet.closer[i] = BIT_1;
		len5_packet.closer[i] = BIT_1;
		len6_packet.closer[i] = BIT_1;
	}

	EncodeByte(len3_packet.byte1,0xff);
	EncodeByte(len3_packet.byte2,0x00);
	EncodeByte(len3_packet.errcheck,0xff);
	len3_packet.byte1_start=BIT_0;
	len3_packet.byte2_start=BIT_0;
	len3_packet.errdet_start=BIT_0;

	EncodeByte(len4_packet.byte1,0xff);
	EncodeByte(len4_packet.byte2,0x00);
	EncodeByte(len4_packet.byte3,0x00);
	EncodeByte(len4_packet.errcheck,0xff);
	len4_packet.byte1_start=BIT_0;
	len4_packet.byte2_start=BIT_0;
	len4_packet.byte3_start=BIT_0;
	len4_packet.errdet_start=BIT_0;

	EncodeByte(len5_packet.byte1,0xff);
	EncodeByte(len5_packet.byte2,0x00);
	EncodeByte(len5_packet.byte3,0x00);
	EncodeByte(len5_packet.byte4,0x00);
	EncodeByte(len5_packet.errcheck,0xff);
	len5_packet.byte1_start=BIT_0;
	len5_packet.byte2_start=BIT_0;
	len5_packet.byte3_start=BIT_0;
	len5_packet.byte4_start=BIT_0;
	len5_packet.errdet_start=BIT_0;

	EncodeByte(len6_packet.byte1,0xff);
	EncodeByte(len6_packet.byte2,0x00);
	EncodeByte(len6_packet.byte3,0x00);
	EncodeByte(len6_packet.byte4,0x00);
	EncodeByte(len6_packet.byte5,0x00);
	EncodeByte(len6_packet.errcheck,0xff);
	len6_packet.byte1_start=BIT_0;
	len6_packet.byte2_start=BIT_0;
	len6_packet.byte3_start=BIT_0;
	len6_packet.byte4_start=BIT_0;
	len6_packet.byte5_start=BIT_0;
	len6_packet.errdet_start=BIT_0;
}

void InsertResetPacket(void)
{
	if ( main_track.main_power == 0)
		return;
	dcc_next_packet = (uint32_t *)&reset_packet ;
	dcc_next_packet_size = sizeof(s_len3_packet)/2;
}

void InsertLen3Packet(void)
{
	if ( main_track.main_power == 0)
		return;
	dcc_next_packet = (uint32_t *)&len3_packet ;
	dcc_next_packet_size = sizeof(s_len3_packet)/2;
}

void InsertLen4Packet(void)
{
	if ( main_track.main_power == 0)
		return;
	dcc_next_packet = (uint32_t *)&len4_packet ;
	dcc_next_packet_size = sizeof(s_len4_packet)/2;
}


uint32_t USB_UART_RxCpltCallback(void)
{
uint8_t	i;
	if ( usb_rxbuf.received_chars_flag == 0 )
		return 0;

	if ( usb_rxbuf.header_found == 0 )
	{
		for ( i=0;i<USB_RXPACKETLEN;i++)
		{
			if (usb_rxbuf.packet[i]== '<')
			{
				usb_rxbuf.header_found = 1;
				usb_rxbuf.start_of_packet = i;
			}
		}
	}
	else
	{
		if( usb_rxbuf.packet[usb_rxbuf.byte_count-1] == '>')
		{
			for ( i=0;i<USB_RXPACKETLEN;i++)
				available_packet.packet[i] = 0;
			for ( i=0;i<usb_rxbuf.byte_count-usb_rxbuf.start_of_packet;i++)
				available_packet.packet[i] = usb_rxbuf.packet[i+usb_rxbuf.start_of_packet];
			available_packet.header_found = usb_rxbuf.header_found;
			available_packet.byte_count = usb_rxbuf.byte_count-usb_rxbuf.start_of_packet;
			available_packet.header_found = usb_rxbuf.header_found;
			for ( i=0;i<USB_RXPACKETLEN;i++)
				usb_rxbuf.packet[i] = 0;
			usb_rxbuf.header_found = 0;
			usb_rxbuf.byte_count = 0;
			usb_rxbuf.start_of_packet = 0;
			usb_rxbuf.received_chars_flag = 0;
			return 1; /* Return 1 when packet finished */
		}
		else
		{
			if ( usb_rxbuf.byte_count >= USB_RXPACKETLEN)
				usb_rxbuf.header_found = usb_rxbuf.byte_count = 0;
		}
	}
	usb_rxbuf.received_chars_flag = 0;
	DebugFlagPulseClear();

	return 0;
}
