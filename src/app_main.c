#include "app_main.h"
#include "thermal_img.h"

#include "hal_led.h"
#include "hal_print.h"
#include "hal_thermal.h"
#include "hal_time.h"

#include <lvgl.h>

void app_init()
{
    while (!thermal_init())
    {
        hal_sleep(5000);
    }

    thermal_img_init();
}

void app_tick()
{
    // static bool led = true;
    // led = !led;
    // hal_led(led);

    float pixels[THERMAL_COLS * THERMAL_ROWS];
    if (thermal_tick(pixels, 0.95, NULL))
    {
        thermal_img_update(pixels);
        // hal_printf("{");
        // for (int i = 0; i < 192; i++)
        // {
        //     hal_printf("%.2f,", pixels[i]);
        // }
        // hal_printf("},\n");
    }
}
