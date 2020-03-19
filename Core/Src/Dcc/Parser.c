/*
 * Parser.c
 *
 *  Created on: Dec 9, 2019
 *      Author: fil
 */

#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "DccGlobal.h"

s_rxbuf			usb_rxbuf, available_packet;

extern	s_main_track 	main_track;
char outbuf[64];

void usb_tx_buffer(char *buffer)
{
	CDC_Transmit_FS((uint8_t*)buffer, strlen(buffer));
}


/* <f 1234 5678 9012> */
/* <T 1234 55 1> */
void one_byte_commands(char cmd)
{
	switch ( cmd)
	{
	case '0' 	: 	sprintf(outbuf,"P0 Main Off\n\r");
					main_track.main_power = 0;
					break;
	case '1' 	: 	sprintf(outbuf,"P1 Main On\n\r");
					main_track.main_power = 1;
					break;
	case '8' 	: 	sprintf(outbuf,"All Off\n\r");
					main_track.main_power = 0;
					break;
	case '9' 	: 	sprintf(outbuf,"All On\n\r");
					main_track.main_power = 1;
					break;
	case 'R' 	: 	sprintf(outbuf,"Sent RESET\n\r");
					InsertResetPacket();
					break;
	case 'S' 	: 	sprintf(outbuf,"Status\n\r");
					break;
	default		:	sprintf(outbuf,"Command error\n\r");
	}
}

void three_bytes_commands(char cmd,int p0,int p1)
{
	switch ( cmd)
	{
	case 'f' 	: 	sprintf(outbuf,"f %d %d\n\r",p0,p1);
					break;
	case 'T' 	: 	sprintf(outbuf,"T %d %d\n\r",p0,p1);
					break;
	default:	sprintf(outbuf,"Command error\n\r");
	}
}

void four_bytes_commands(char cmd,int p0,int p1,int p2)
{
	switch ( cmd)
	{
	case 'f' 	: 	sprintf(outbuf,"f %d %d %d\n\r",p0,p1,p2);
					break;
	case 'a' 	: 	sprintf(outbuf,"a %d %d %d\n\r",p0,p1,p2);
					break;
	case 'T' 	: 	sprintf(outbuf,"T : Address %d , Speed %d , Direction %d\n\r",p0,p1,p2);
					if ( EncodeThrottle(p0,p1,p2) == 0 )
						InsertLen3Packet();
					else
						InsertLen4Packet();
					break;
	default:	sprintf(outbuf,"Command error\n\r");
	}
}

void five_bytes_commands(char cmd,int p0,int p1,int p2,int p3)
{
	switch ( cmd)
	{
	case 'T' 	: 	sprintf(outbuf,"Throttle %d %d %d %d\n\r",p0,p1,p2,p3);
					break;
	default:	sprintf(outbuf,"Command error\n\r");
	}
}

uint32_t DccParser(void)
{
int	p0,p1,p2,p3,p4,p5,p6,p7,pnum;
char cmd;
uint32_t	retval = 0;

	pnum = sscanf((char *)&available_packet.packet[0],"<%c %d %d %d %d %d %d %d %d",&cmd,&p0,&p1,&p2,&p3,&p4,&p5,&p6,&p7);
	switch (pnum)
	{
	case 1 :	one_byte_commands(cmd);
				break;
	case 3 :	three_bytes_commands(cmd,p0,p1);
				break;
	case 4 :	four_bytes_commands(cmd,p0,p1,p2);
				break;
	case 5 :	five_bytes_commands(cmd,p0,p1,p2,p3);
				break;
	default:	sprintf(outbuf,"Command error %d\n\r",pnum); retval = 1;
	}

	usb_tx_buffer(outbuf);
	return retval;
}


