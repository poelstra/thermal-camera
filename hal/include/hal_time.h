#ifndef HAL_TIME_H
#define HAL_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void hal_sleep(unsigned long milliseconds);
unsigned long hal_millis();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*HAL_TIME_H*/
