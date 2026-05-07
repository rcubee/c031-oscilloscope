#ifndef INC_SYSTICK_H_
#define INC_SYSTICK_H_

#include <stdint.h>

void systick_init();

uint32_t systick_get_millis();

#endif /* INC_SYSTICK_H_ */
