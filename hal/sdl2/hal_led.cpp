#include "hal_led.h"

#include <stdio.h>

void hal_led(bool on)
{
    printf("led=%u\n", on ? 1 : 0);
}