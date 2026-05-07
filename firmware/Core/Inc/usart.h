#ifndef INC_USART_H_
#define INC_USART_H_

#include <stdint.h>

void usart_init();

// Note: For now, no circular buffer is used - the receiving
// works by setting up dma transfer from RDR to internal buffer
// until IDLE line is detected - FRAMES MUST BE SENT IN ONE USART TRANSFER.
// This also means that this function has to be called frequently to re-arm DMA
void usart_receive_dma(uint8_t** buff, uint8_t* buff_len);

void usart_transmit(uint8_t byte);

void usart_transmit_dma(const uint8_t *src, uint16_t size);

#endif /* INC_USART_H_ */
