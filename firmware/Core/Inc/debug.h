#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#ifdef DEBUG

#include <stdio.h>

// Note: https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html#:~:text=Historically%2C%20GNU,write
#define LOG(format, ...) printf("[LOG]: " format "\n", ##__VA_ARGS__)

void debug_init();

#else // DEBUG

#define LOG(...) (void)0

#endif // DEBUG

#endif /* INC_DEBUG_H_ */
