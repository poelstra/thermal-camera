#include "hal_time.h"

#include <Arduino.h>

void hal_sleep(unsigned long milliseconds)
{
    delay(milliseconds);
}

unsigned long hal_millis()
{
    return millis();
}
