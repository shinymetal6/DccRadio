/*
 * TimerHandler.c
 *
 *  Created on: Mar 17, 2020
 *      Author: Fil
 */

#include "main.h"
#include "DccGlobal.h"

extern	s_rxbuf			usb_rxbuf;

void DebugFlagPulseSet(void)
{
	DEBUG_FLAG_GPIO_PORT->BSRR = DEBUG_FLAG_PIN;	// Set pin
}
void DebugFlagPulseClear(void)
{
	DEBUG_FLAG_GPIO_PORT->BRR  = DEBUG_FLAG_PIN;	// Clear pin
}

uint32_t	led_period = LPTIM1_BLINK_PERIOD;
void do_toggle(void)
{
	if ((HEARTBEAT_LED_PORT->ODR & HEARTBEAT_LED_PIN) != 0x00u)
		HEARTBEAT_LED_PORT->BRR = (uint32_t)HEARTBEAT_LED_PIN;
	else
		HEARTBEAT_LED_PORT->BSRR = (uint32_t)HEARTBEAT_LED_PIN;
}

void LedToggle(void)
{
	led_period--;
	if ( led_period == 0 )
	{
		led_period = LPTIM1_BLINK_PERIOD;
		do_toggle();
	}
}

static void fast_DMA_SetConfig(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
	/* Clear the DMAMUX synchro overrun flag */
	hdma->DMAmuxChannelStatus->CFR = hdma->DMAmuxChannelStatusMask;
	if (hdma->DMAmuxRequestGen != 0U)
	{
		/* Clear the DMAMUX request generator overrun flag */
		hdma->DMAmuxRequestGenStatus->RGCFR = hdma->DMAmuxRequestGenStatusMask;
	}
	/* Clear all flags */
	hdma->DmaBaseAddress->IFCR = (DMA_ISR_GIF1 << (hdma->ChannelIndex & 0x1FU));
	/* Configure DMA Channel data length */
	hdma->Instance->CNDTR = DataLength;
	/* Memory to Peripheral */
	/* Configure DMA Channel destination address */
	hdma->Instance->CPAR = DstAddress;
	/* Configure DMA Channel source address */
	hdma->Instance->CMAR = SrcAddress;
}

HAL_StatusTypeDef fast_HAL_DMA_Start_IT(DMA_HandleTypeDef *hdma, uint32_t SrcAddress, uint32_t DstAddress, uint32_t DataLength)
{
	/* Disable the peripheral */
	__HAL_DMA_DISABLE(hdma);
	/* Configure the source, destination address and the data length & clear flags*/
	fast_DMA_SetConfig(hdma, SrcAddress, DstAddress, DataLength);
	__HAL_DMA_DISABLE_IT(hdma, DMA_IT_HT);
	__HAL_DMA_ENABLE_IT(hdma, (DMA_IT_TC | DMA_IT_TE));
	/* Enable the Peripheral */
	__HAL_DMA_ENABLE(hdma);
	return HAL_OK;
}
//TIM2_CHANNEL_1
void PWM_Start_Packet_DMA(uint32_t *pData, uint16_t Length)
{
	/* Set the DMA error callback */
	DCC1_TIMER.hdma[TIM_DMA_ID_CC1]->XferErrorCallback = TIM_DMAError ;
	/* Enable the DMA channel */
	fast_HAL_DMA_Start_IT( DCC1_TIMER.hdma[TIM_DMA_ID_CC1], (uint32_t)pData, (uint32_t)&DCC1_TIMER.Instance->PSC, Length);
	/* Enable the TIM Capture/Compare 1 DMA request */
	DCC1_TIMER.Instance->DIER |= TIM_DMA_CC1;
	/* Enable the Capture compare channel */
	TIM_CCxChannelCmd(DCC1_TIMER.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);
	DCC1_TIMER.Instance->BDTR|=TIM_BDTR_MOE;
	/* Enable the Peripheral */
	DCC1_TIMER.Instance->CR1|=TIM_CR1_CEN;
}

void PacketCallback(void)
{
	DCC1_TIMER.Instance->CR1 &= ~TIM_CR1_CEN;
	CUTOUT1_TIMER.Instance->CR1 |= TIM_CR1_CEN;
	ENABLE1_PORT->BRR = ENABL1E_PIN;	// Clear pin
	if (( dcc_packet == (uint32_t *)&idle_packet) && (dcc_next_packet != NULL ))
	{
		DCCPKT_LED_PORT->BSRR = DCCPKT_LED_PIN;	// Set pin
		DebugFlagPulseSet();
		dcc_packet_size = dcc_next_packet_size;
		dcc_packet = dcc_next_packet;
		dcc_next_packet = NULL;
		DebugFlagPulseClear();
	}
	else
	{
		dcc_packet = (uint32_t *)&idle_packet ;
		dcc_packet_size = sizeof(idle_packet)/2;
		DCCPKT_LED_PORT->BRR = DCCPKT_LED_PIN;	// Clear pin
	}
}

void CoutoutCallback(void)
{
	if ( main_track.main_power == 1 )
	{
		CUTOUT1_TIMER.Instance->CR1 &= ~TIM_CR1_CEN;
		ENABLE1_PORT->BSRR = ENABL1E_PIN;	// Set pin
		PWM_Start_Packet_DMA( dcc_packet , dcc_packet_size);
	}
}

void DccStart(void)
{
	usb_rxbuf.received_chars_flag = 0;
	dcc_packet = (uint32_t *)&idle_packet ;
	dcc_packet_size = sizeof(idle_packet)/2;
	HAL_TIM_Base_Start_IT(&CUTOUT1_TIMER);
}

