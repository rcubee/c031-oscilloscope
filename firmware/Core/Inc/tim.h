#ifndef INC_TIM_H_
#define INC_TIM_H_

#include <stdint.h>

void tim_init();

void tim_enable();

void tim_disable();

void tim_configure_for_sampling(uint32_t ext_trig_interval);

void tim_cleanup_after_sampling();

/**
 * @brief Returns the number of **CK_INT** clock cycles after which Update Event is guaranteed to occur.
 *
 * @return The number of clock cycles.
 */
uint16_t tim_get_cycles_to_uev();

#endif /* INC_TIM_H_ */
