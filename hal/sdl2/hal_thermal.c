#include "hal_thermal.h"

#include "hal_thermal_frames.inc.c"
#include "hal_time.h"

bool thermal_init()
{
    return true;
}

const unsigned long FRAME_TIME = 1000 / 8;

bool thermal_tick(float *pixels, float emissivity, float *tr)
{
    static unsigned long lastTime = 0;
    unsigned long now = hal_millis();

    if (now - lastTime < FRAME_TIME)
    {
        return false;
    }

    lastTime = now;
    int frameIndex = (now / FRAME_TIME) % thermal_frames_count;

    for (int i = 0; i < THERMAL_ROWS * THERMAL_COLS; i++)
    {
        pixels[i] = thermal_frames[frameIndex][i];
    }

    return true;
}
