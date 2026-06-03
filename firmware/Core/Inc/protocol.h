#ifndef INC_PROTOCOL_H_
#define INC_PROTOCOL_H_

#include "osc.h"

void osc_handle_protocol(osc* oscilloscope);

void osc_transmit_message(const char* message);

void osc_transmit_samples(osc* oscilloscope, uint16_t samples_collected);

#endif /* INC_PROTOCOL_H_ */
