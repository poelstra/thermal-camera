#include "hal_print.h"

#include <stdarg.h>
#include <stdio.h>

int hal_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int result = vprintf(fmt, args);
    va_end(args);
    return result;
}