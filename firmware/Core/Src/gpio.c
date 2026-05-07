#include "gpio.h"
#include "stm32c031xx.h"

void gpio_init()
{
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

#ifdef DEBUG
    /* LD4 */
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE5) | GPIO_MODER_MODE5_0; // Set PA5 to output
#endif

    /* TIM14 */
    GPIOA->AFR[0 /* GPIOA_AFRL */] = (GPIOA->AFR[0] & ~GPIO_AFRL_AFSEL4) | GPIO_AFRL_AFSEL4_2; // Select TIM14_CH1 (AF4) as alternate function of PA4
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE4) | GPIO_MODER_MODE4_1; // Set PA4 to alternate function

    /* USART2 */

    GPIOA->AFR[0 /* GPIOA_AFRL */] =
        (GPIOA->AFR[0] & ~(GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3))
        | GPIO_AFRL_AFSEL2_0 // Select AF1 (USART2_TX) for PA2
        | GPIO_AFRL_AFSEL3_0; // Select AF1 (USART2_RX) for PA3

    GPIOA->MODER =
        (GPIOA->MODER & ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3))
        | GPIO_MODER_MODE2_1 // Set PA2 to alternate function
        | GPIO_MODER_MODE3_1; // Set PA3 to alternate function

}

void gpio_led_on()
{
    GPIOA->BSRR |= GPIO_BSRR_BS5;
}

void gpio_led_off()
{
    GPIOA->BSRR |= GPIO_BSRR_BR5;
}
