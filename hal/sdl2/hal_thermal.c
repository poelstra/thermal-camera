#include "hal_thermal.h"

#include "hal_thermal_frames.inc.c"
#include "hal_time.h"

#include "hal_print.h"

#include <stddef.h>

bool thermal_init()
{
    return true;
}

const unsigned long FRAME_TIME = 1000 / 8;

bool thermal_tick(float *pixels, float emissivity, bool auto_tr, float *tr)
{
    static unsigned long lastTime = 0;
    unsigned long now = hal_millis();

    if (now - lastTime < FRAME_TIME)
    {
        return false;
    }

    if (auto_tr && tr != NULL)
    {
        // Act like measured ambient temperature is fluctuating
        // between 25.0 and 25.9, just to demonstrate the live
        // display of the value in the settings dialog etc.
        *tr = 25.0 + ((now / 1000) % 10) / 10.0;
    }

    lastTime = now;
    int frameIndex = (now / FRAME_TIME) % thermal_frames_count;

    for (int i = 0; i < THERMAL_ROWS * THERMAL_COLS; i++)
    {
        pixels[i] = thermal_frames[frameIndex][i];
    }

    return true;
}
