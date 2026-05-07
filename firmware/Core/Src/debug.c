#ifdef DEBUG

#include "debug.h"
#include "gpio.h"
#include "usart.h"
#include "stm32c031xx.h"

// Note: Override default assert handler
void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
    gpio_led_on();

    LOG("ASSERT: file: %s, line: %i, func: %s, failedexpr: %s", file, line, func, failedexpr);

    __ASM("BKPT #0");

    while (1)
        ;
}

int _write(int file, char *ptr, int len)
{
    (void)file;

    for (int i = 0; i < len; i++) {
        usart_transmit(ptr[i]);
    }

    return len;
}

void debug_init()
{
    RCC->APBENR1 |= RCC_APBENR1_DBGEN;

    DBG->APBFZ2 |= DBG_APB_FZ2_DBG_TIM1_STOP | DBG_APB_FZ2_DBG_TIM14_STOP;
}

#endif // DEBUG
