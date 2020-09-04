#include "keyboard.h"

#include <Arduino.h>
#include <lvgl.h>

static const uint8_t key_pins[] = {
    WIO_5S_UP, WIO_5S_DOWN, WIO_5S_LEFT, WIO_5S_RIGHT, WIO_5S_PRESS, WIO_KEY_A, WIO_KEY_B, WIO_KEY_C,
};

#define NUM_KEYS (sizeof(key_pins) / sizeof(key_pins[0]))

// WIO numbered their three top keys in reverse order...
static const uint32_t key_codes[NUM_KEYS] = {
    LV_KEY_PREV, LV_KEY_NEXT, LV_KEY_LEFT, LV_KEY_RIGHT, LV_KEY_ENTER, 'C', 'B', 'A',
};

static bool keyboard_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    // Theoretically, we can send multiple _PR states in one 'tick'
    // by returning true and it will keep calling us. In practice,
    // we don't queue keys, and we probably actually want to filter out
    // accidental middle-click while also pressing any of the arrow keys.
    // So, first update the state of all keys, then check if one of them
    // is pressed. If it is, we thereby forget the state of any of the
    // other keys, which is considered a good thing for now.

    for (int i = 0; i < NUM_KEYS; i++)
    {
        bool pressed = (digitalRead(key_pins[i]) == 0);
        if (pressed)
        {
            data->key = key_codes[i];
            data->state = LV_INDEV_STATE_PR;
            return false;
        }
    }

    data->state = LV_INDEV_STATE_REL;
    return false;
}

void keyboard_init()
{
    for (int i = 0; i < NUM_KEYS; i++)
    {
        pinMode(key_pins[i], INPUT_PULLUP);
    }

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keyboard_read;
    lv_indev_drv_register(&indev_drv);
}
