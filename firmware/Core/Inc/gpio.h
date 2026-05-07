#ifndef INC_GPIO_H_
#define INC_GPIO_H_

void gpio_init();

/**
 * @brief Turns on PA5 (LD4 on Nucleo-C031C6).
 */
void gpio_led_on();

/**
 * @brief Turns off PA5 (LD4 on Nucleo-C031C6).
 */
void gpio_led_off();

#endif /* INC_GPIO_H_ */
