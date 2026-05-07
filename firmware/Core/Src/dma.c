#include "adc.h"
#include <stddef.h>
#include "stm32c031xx.h"

void dma_init()
{
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
}

void dma_configure_for_sampling()
{
    // Note: DMAMUX Channel 2 corresponds to DMA Channel 3
    DMAMUX1_Channel2->CCR |= (5 /* 5 = adc1_dma */ & DMAMUX_CxCR_DMAREQ_ID);

    DMA1_Channel3->CPAR = ADC1_BASE + offsetof(ADC_TypeDef, DR);
    DMA1_Channel3->CMAR = (uint32_t)adc_get_buff();
    DMA1_Channel3->CNDTR = ADC_BUFF_SIZE;
    DMA1_Channel3->CCR |=
        DMA_CCR_TCIE // Enable transfer complete interrupt
        | DMA_CCR_HTIE // Enable half transfer interrupt
        | DMA_CCR_TEIE // Enable transfer error interrupt
        | DMA_CCR_CIRC // Enable circular mode
        | DMA_CCR_MINC // Enable memory increment mode
        | DMA_CCR_PSIZE_0
        | DMA_CCR_MSIZE_0;

    NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

    DMA1_Channel3->CCR |= DMA_CCR_EN;
}

void dma_cleanup_after_sampling()
{
    DMA1_Channel3->CCR &= ~DMA_CCR_EN;
}
