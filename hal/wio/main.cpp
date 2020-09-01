#include "app_main.h"

#include "hal_led.h"
#include "hal_time.h"

#include "lcd.h"
#include <Arduino.h>
#include <lvgl.h>

void setup()
{
    Serial.begin(115200);

    // Wait for serial monitor to connect when powering up with
    // middle button pressed
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);
    if (digitalRead(WIO_5S_PRESS) == 0)
    {
        while (!Serial)
        {
            hal_led(true);
            hal_sleep(50);
            hal_led(false);
            hal_sleep(100);
        }
        Serial.println("Serial connected.");

        // Wait until key released to prevent accidental other
        // presses at startup
        while (digitalRead(WIO_5S_PRESS) == 0)
        {
            hal_led(true);
            hal_sleep(500);
            hal_led(false);
            hal_sleep(100);
        }
    }

    Serial.println("Thermal Camera");
    Serial.println("Copyright (C) 2020 Martin Poelstra");
    Serial.println();

    lv_init();
    lcd_init();
    app_init();
}

void loop()
{
    lv_task_handler();
    app_tick();
}
