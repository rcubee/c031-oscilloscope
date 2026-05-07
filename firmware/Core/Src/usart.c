#include <stddef.h>
#include "stm32c031xx.h"
#include "usart.h"

#define USART_RX_BUFF_SIZE 64U

static uint8_t usart_rx_buff[USART_RX_BUFF_SIZE];

static void usart_set_baud_rate()
{
    // 19200
	// USART2->BRR = 1250; // (24 * 10^6) / 19200 = 1250;

    // 115200
	// USART2->BRR = 208; // (24 * 10^6) / 115200 ~= 208

	// 1000000
	USART2->BRR = 24; // (24 * 10^6) / 1000000 = 24

	// 2000000
	// USART2->BRR = 12; // (24 * 10^6) / 2000000 = 12
}

static void usart_enable()
{
	USART2->CR1 |= USART_CR1_UE | USART_CR1_RE | USART_CR1_TE;
}

static void usart_receive_dma_arm()
{
    // Idle line detected clear flag
    USART2->ICR |= USART_ICR_IDLECF;

    // IDLE interrupt enable
    USART2->CR1 |= USART_CR1_IDLEIE;

    // DMA enable receiver
    USART2->CR3 |= USART_CR3_DMAR;

    DMAMUX1_Channel0->CCR |= 52 /* usart2_rx_dma */ << DMAMUX_CxCR_DMAREQ_ID_Pos;

    DMA1_Channel1->CPAR = (uint32_t)&(USART2->RDR);
    DMA1_Channel1->CMAR = (uint32_t)usart_rx_buff;
    DMA1_Channel1->CNDTR = USART_RX_BUFF_SIZE; // Note: Receive up to USART_RX_BUFF_SIZE bytes
    DMA1_Channel1->CCR |= DMA_CCR_EN | DMA_CCR_MINC; // Note: Since Channel0 and Channel1 both have low priority, Channel0 will take precedence
}

void usart_init()
{
	// Note: Enable APB clock for USART2
	RCC->APBENR1 |= RCC_APBENR1_USART2EN;

	usart_set_baud_rate();

	usart_enable();

	usart_receive_dma_arm();
}

void usart_receive_dma(uint8_t** buff, uint8_t* buff_len)
{
    if (!(USART2->ISR & USART_ISR_IDLE)) {
        *buff = NULL;
        *buff_len = 0;

        return; // Note: Either no data was received or reception is still in progress
    }

    DMA1_Channel1->CCR &= ~DMA_CCR_EN; // Note: Disable DMA Channel immediately

    uint8_t bytes_received = USART_RX_BUFF_SIZE - DMA1_Channel1->CNDTR;
    if (bytes_received) {
        *buff = usart_rx_buff;
        *buff_len = bytes_received;
    } else {
        // Note: IDLE flag could be set even if no data was received (on error)
        *buff = NULL;
        *buff_len = 0;
    }

    usart_receive_dma_arm();
}

void usart_transmit(uint8_t byte)
{
	while (!(USART2->ISR & USART_ISR_TXE_TXFNF))
		;

	USART2->TDR = byte;

	while (!(USART2->ISR & USART_ISR_TC))
		;
}

void usart_transmit_dma(const uint8_t *src, uint16_t size)
{
    /*
     * RM0490
     * 12.3 DMAMUX implementation
     * 11.4.5 DMA channels Channel configuration procedure,
     * 26.5.19 Continuous communication using USART and DMA
     */

    // Note: DMAMUX Channel 1 corresponds to DMA Channel 2
    DMAMUX1_Channel1->CCR |= (53 /* 53 = usart2_tx_dma */ & DMAMUX_CxCR_DMAREQ_ID);

    DMA1_Channel2->CPAR = USART2_BASE + offsetof(USART_TypeDef, TDR);
    DMA1_Channel2->CMAR = (uint32_t)src;
    DMA1_Channel2->CNDTR = size;
    DMA1_Channel2->CCR |= DMA_CCR_DIR | DMA_CCR_MINC;

    USART2->ICR |= USART_ICR_TCCF;

    USART2->CR3 |= USART_CR3_DMAT;

    DMA1_Channel2->CCR |= DMA_CCR_EN;

    while (!(USART2->ISR & USART_ISR_TC)) {
        __ASM("NOP");
    }

    // Clear DMA Channel ISR transfer-related flags
    DMA1->IFCR |= DMA_IFCR_CGIF1;

    // Disable DMA channel
    DMA1_Channel2->CCR &= ~DMA_CCR_EN;
}
