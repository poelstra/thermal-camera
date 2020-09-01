#include "app_main.h"

#include "lcd.h"
#include <Arduino.h>
#include <lvgl.h>

void setup()
{
    Serial.begin(115200);

    lv_init();
    lcd_init();
    app_init();
}

void loop()
{
    lv_task_handler();
    app_tick();
}
