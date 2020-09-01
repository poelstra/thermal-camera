#include "app_main.h"

#include "hal_led.h"
#include "hal_time.h"

#include <lvgl.h>

void app_init()
{
    lv_obj_t *button = lv_btn_create(lv_scr_act(), NULL);
}

void app_tick()
{
    hal_led(true);
    hal_sleep(200);
    hal_led(false);
    hal_sleep(100);
}