#include "hal_led.h"

#include <Arduino.h>

void hal_led(bool on)
{
    digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
}