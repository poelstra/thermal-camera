#include "hal_print.h"

#include <Arduino.h>
#include <stdarg.h>

int hal_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[256];
    int result = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    Serial.write(buf);
    return result;
}