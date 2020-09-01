#include "app_main.h"
#include "thermal_img.h"

#include "hal_init.h"
#include "hal_led.h"
#include "hal_print.h"
#include "hal_thermal.h"
#include "hal_time.h"

#include <lvgl.h>

static lv_group_t *focus_group;

void app_init()
{
    lv_init();
    focus_group = lv_group_create();
    hal_init(focus_group);

    hal_printf("Thermal Camera\n");
    hal_printf("Copyright (C) 2020 Martin Poelstra\n\n");

    while (!thermal_init())
    {
        hal_printf("Camera initialization failed, retrying...\n");
        hal_sleep(5000);
    }
    hal_printf("Camera initialized.\n");

    thermal_img_init();
}

void app_tick()
{
    lv_task_handler();

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
