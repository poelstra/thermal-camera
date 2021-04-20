#include "app_main.h"
#include "focus.h"
#include "header.h"
#include "settings.h"
#include "storage.h"
#include "thermal_img.h"

#include "hal_flash.h"
#include "hal_init.h"
#include "hal_input.h"
#include "hal_led.h"
#include "hal_print.h"
#include "hal_thermal.h"
#include "hal_time.h"

#include <lvgl.h>

static Settings settings = {
    .emissivity = 0.95,
    .auto_ambient = true,
    .reflected_temperature = 25.0,
    .auto_range = true,
    .min_temp = 25.0,
    .max_temp = 38.0,
    .flip_hor = false,
    .flip_ver = false,
};

static void settings_closed_cb(void)
{
    /* no-op */
}

static void main_event_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_KEY)
    {
        uint32_t key = *((uint32_t *)lv_event_get_data());
        bool wait_release = false;
        switch (key)
        {
        case LV_KEY_ENTER:
            settings_show(&settings, settings_closed_cb);
            wait_release = true;
            break;
        case 'A':
            wait_release = true;
            break;
        case 'B':
            wait_release = true;
            break;
        case 'C':
            wait_release = true;
            break;
        }

        if (wait_release)
        {
            lv_indev_t *indev = lv_indev_get_act();
            lv_indev_wait_release(indev);
        }
    }
}

void print_lv_log_cb(lv_log_level_t level, const char *file, uint32_t line, const char *func, const char *desc)
{
    const char *level_text = "<UNKNOWN>";
    switch (level)
    {
    case LV_LOG_LEVEL_TRACE:
        level_text = "TRACE";
        break;
    case LV_LOG_LEVEL_INFO:
        level_text = "INFO";
        break;
    case LV_LOG_LEVEL_WARN:
        level_text = "WARNING";
        break;
    case LV_LOG_LEVEL_ERROR:
        level_text = "ERROR";
        break;
    case LV_LOG_LEVEL_USER:
        level_text = "USER";
        break;
    }
    hal_printf("[%s] %s:%u (%s) %s\n", level_text, file, line, func, desc);
}

static bool keyboard_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    bool result = hal_keyboard_read(drv, data);

    static bool enter_pressed = false;
    if (data->key != LV_KEY_ENTER || data->state != LV_INDEV_STATE_PR)
    {
        enter_pressed = false;
    }

    lv_group_t *group = focus_get_current_group();
    bool editing = lv_group_get_editing(group);

    switch (data->key)
    {
    case LV_KEY_ENTER:
        if (!enter_pressed && data->state == LV_INDEV_STATE_PR)
        {
            enter_pressed = true;
            lv_obj_t *focused = lv_group_get_focused(group);
            lv_obj_type_t type;
            lv_obj_get_type(focused, &type);
            if (strcmp(type.type[0], "lv_spinbox") == 0)
            {
                lv_indev_wait_release(lv_indev_get_act());

                lv_group_set_editing(group, !editing);
                lv_group_focus_cb_t focus_cb = lv_group_get_focus_cb(group);
                if (focus_cb)
                {
                    focus_cb(group);
                }
            }
        }
        break;
    case LV_KEY_UP:
        if (!editing)
        {
            data->key = LV_KEY_PREV;
        }
        break;
    case LV_KEY_DOWN:
        if (!editing)
        {
            data->key = LV_KEY_NEXT;
        }
        break;
    case LV_KEY_LEFT:
        if (!editing)
        {
            data->key = LV_KEY_PREV;
        }
        break;
    case LV_KEY_RIGHT:
        if (!editing)
        {
            data->key = LV_KEY_NEXT;
        }
        break;

    case 'A':
    case 'B':
    case 'C':
        if (editing)
        {
            // Prevent entering 'function keys' as literal values
            // into e.g. an edit box.
            data->state = LV_INDEV_STATE_REL;
        }
        break;
    }
    return result;
}

static void keyboard_init()
{
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keyboard_read;
    lv_indev_drv_register(&indev_drv);
}

void app_init()
{
    lv_init();
    hal_init();
    keyboard_init();
    lv_log_register_print_cb(print_lv_log_cb);

    hal_printf("Thermal Camera\n");
    hal_printf("Copyright (C) 2020 Martin Poelstra\n\n");

    if (!hal_flash_init())
    {
        hal_printf("WARNING: Storage init failed: can't load/store settings\n");
    }
    else
    {
        Settings temp_settings;
        if (!storage_read(SETTINGS_MAGIC, SETTINGS_VERSION, &temp_settings, sizeof(temp_settings)))
        {
            hal_printf("No existing settings found, using defaults.\n");
        }
        else
        {
            hal_printf("Settings loaded from flash.\n");
            settings = temp_settings;
        }
    }

    while (!hal_thermal_init())
    {
        hal_printf("Camera initialization failed, retrying...\n");
        hal_sleep(5000);
    }
    hal_printf("Camera initialized.\n");

    header_init();
    thermal_img_init();

    lv_obj_set_event_cb(lv_scr_act(), main_event_cb);
    lv_group_t *group = focus_push_group();
    lv_group_add_obj(group, lv_scr_act());
}

void flip_pixels(float pixels[THERMAL_COLS * THERMAL_ROWS], bool flip_hor, bool flip_ver)
{
    if (flip_hor)
    {
        for (uint16_t y = 0; y < THERMAL_ROWS; y++)
        {
            uint16_t row = y * THERMAL_COLS;
            for (uint16_t x = 0; x < THERMAL_COLS / 2; x++)
            {
                float temp = pixels[row + x];
                pixels[row + x] = pixels[row + THERMAL_COLS - x - 1];
                pixels[row + THERMAL_COLS - x - 1] = temp;
            }
        }
    }
    if (flip_ver)
    {
        for (uint16_t y = 0; y < THERMAL_ROWS / 2; y++)
        {
            float temp[THERMAL_COLS];
            memmove(temp, &pixels[y * THERMAL_COLS], THERMAL_COLS * sizeof(float));
            memmove(&pixels[y * THERMAL_COLS], &pixels[(THERMAL_ROWS - y - 1) * THERMAL_COLS],
                    THERMAL_COLS * sizeof(float));
            memmove(&pixels[(THERMAL_ROWS - y - 1) * THERMAL_COLS], temp, THERMAL_COLS * sizeof(float));
        }
    }
}

void app_tick()
{
    lv_task_handler();

    float pixels[THERMAL_COLS * THERMAL_ROWS];
    if (hal_thermal_tick(pixels, settings.emissivity, settings.auto_ambient, &settings.reflected_temperature))
    {
        flip_pixels(pixels, settings.flip_hor, settings.flip_ver);
        hal_printf("temp=%.1f\n", pixels[THERMAL_COLS / 2 + THERMAL_COLS * (THERMAL_ROWS / 2)]);
        thermal_img_update(pixels, &settings);
        header_update(&settings);
    }
}
