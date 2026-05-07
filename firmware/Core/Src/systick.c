#include "systick.h"
#include "stm32c031xx.h"

// Note: ARMv6-M Architecture Reference Manual, B3.3.1 SysTick operation

static uint32_t millis;

void SysTick_Handler()
{
    ++millis;
}

void systick_init()
{
    // Note: Reaload each 1 ms (assuming processor clock is running at 24MHz)
    SysTick->LOAD = 24000 - 1;

    SysTick->CTRL |=
        SysTick_CTRL_ENABLE_Msk
        | SysTick_CTRL_TICKINT_Msk // Enable SysTick interrupt
        | SysTick_CTRL_CLKSOURCE_Msk; // Use the processor clock
}

uint32_t systick_get_millis()
{
    return millis;
}
