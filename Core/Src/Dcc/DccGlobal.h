/*
 * DccGlobal.h
 *
 *  Created on: Mar 17, 2020
 *      Author: Fil
 */

#ifndef SRC_DCC_DCCGLOBAL_H_
#define SRC_DCC_DCCGLOBAL_H_

#include <string.h>

/* Porting */
/* Timer definitions */
extern		TIM_HandleTypeDef 		htim2;
#define		DCC1_TIMER				htim2
extern		TIM_HandleTypeDef 		htim17;
#define		CUTOUT1_TIMER			htim17
/* Timer definitions end */
/* Bit definitions */
#define		ENABLE1_PORT			PWMB_GPIO_Port
#define		ENABL1E_PIN				PWMB_Pin
#define		DCC_ACTIVE_LED_PORT		LD3_GPIO_Port
#define		DCC_ACTIVE_LED_PIN		LD3_Pin
#define		HEARTBEAT_LED_PORT		LD2_GPIO_Port
#define		HEARTBEAT_LED_PIN		LD2_Pin
#define		DCCPKT_LED_PORT			LD1_GPIO_Port
#define		DCCPKT_LED_PIN			LD1_Pin
#define 	DEBUG_FLAG_PIN 			GPIO_PIN_12
#define 	DEBUG_FLAG_GPIO_PORT 	GPIOC
/* Bit definitions end */
/* Porting end */


#define		BIT_0				128
#define		BIT_1				64
#define		PREAMBLE_LEN		15
#define		BYTE_LEN			8
#define		CLOSER_LEN			2
#define		LPTIM1_PERIOD		10000
#define		LPTIM1_BLINK_PERIOD	50
#define		USB_RXPACKETLEN		28

typedef struct {
	uint16_t 	main_power;
} s_main_track;

typedef struct {
	uint16_t preamble[PREAMBLE_LEN];
	uint16_t byte1_start;
	uint16_t byte1[BYTE_LEN];
	uint16_t byte2_start;
	uint16_t byte2[BYTE_LEN];
	uint16_t errdet_start;
	uint16_t errcheck[BYTE_LEN];
	uint16_t closer[CLOSER_LEN];
} s_len3_packet;

typedef struct {
	uint16_t preamble[PREAMBLE_LEN];
	uint16_t byte1_start;
	uint16_t byte1[BYTE_LEN];
	uint16_t byte2_start;
	uint16_t byte2[BYTE_LEN];
	uint16_t byte3_start;
	uint16_t byte3[BYTE_LEN];
	uint16_t errdet_start;
	uint16_t errcheck[BYTE_LEN];
	uint16_t closer[CLOSER_LEN];
} s_len4_packet;

typedef struct {
	uint16_t preamble[PREAMBLE_LEN];
	uint16_t byte1_start;
	uint16_t byte1[BYTE_LEN];
	uint16_t byte2_start;
	uint16_t byte2[BYTE_LEN];
	uint16_t byte3_start;
	uint16_t byte3[BYTE_LEN];
	uint16_t byte4_start;
	uint16_t byte4[BYTE_LEN];
	uint16_t errdet_start;
	uint16_t errcheck[BYTE_LEN];
	uint16_t closer[CLOSER_LEN];
} s_len5_packet;

typedef struct {
	uint16_t preamble[PREAMBLE_LEN];
	uint16_t byte1_start;
	uint16_t byte1[BYTE_LEN];
	uint16_t byte2_start;
	uint16_t byte2[BYTE_LEN];
	uint16_t byte3_start;
	uint16_t byte3[BYTE_LEN];
	uint16_t byte4_start;
	uint16_t byte4[BYTE_LEN];
	uint16_t byte5_start;
	uint16_t byte5[BYTE_LEN];
	uint16_t errdet_start;
	uint16_t errcheck[BYTE_LEN];
	uint16_t closer[CLOSER_LEN];
} s_len6_packet;


typedef struct {
	uint8_t byte_count;
	uint8_t start_of_packet;
	uint8_t received_chars_flag;
	uint8_t header_found;
	uint8_t packet[USB_RXPACKETLEN];
} s_rxbuf;


extern	s_len3_packet	idle_packet;
extern	s_len3_packet	reset_packet;
extern	s_main_track 	main_track;
extern	s_len3_packet 	len3_packet;
extern	s_len4_packet 	len4_packet;
extern	s_len5_packet 	len5_packet;
extern	s_len6_packet 	len6_packet;
extern	uint32_t 		*dcc_packet;
extern	uint32_t		dcc_packet_size;
extern	uint32_t		*dcc_next_packet;
extern	uint32_t		dcc_next_packet_size;
extern	uint32_t		update_semaphore;

void InitDccPackets(void);
void InsertResetPacket(void);
void InsertLen3Packet(void);
void InsertLen4Packet(void);
uint32_t EncodeThrottle(int cab,int speed,int direction);
uint32_t USB_UART_RxCpltCallback(void);
void CoutoutTimerStart(void);
void PacketCallback(void);
void CoutoutCallback(void);
void DccStart(void);
uint32_t DccParser(void);
void DebugFlagPulseSet(void);
void DebugFlagPulseClear(void);
void LedToggle(void);
uint8_t USBD_CDC_Init(void);
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

uint32_t USB_UART_RxCpltCallback(void);

#endif /* SRC_DCC_DCCGLOBAL_H_ */
