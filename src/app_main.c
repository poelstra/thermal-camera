#include "app_main.h"
#include "thermal_img.h"

#include "hal_init.h"
#include "hal_led.h"
#include "hal_print.h"
#include "hal_thermal.h"
#include "hal_time.h"

#include <lvgl.h>

typedef struct FocusGroup FocusGroup;
struct FocusGroup
{
    lv_group_t *group;
    FocusGroup *next;
};

static FocusGroup *groups = NULL;

void set_focus_group(lv_group_t *group)
{
    lv_indev_t *indev = lv_indev_get_next(NULL);
    while (indev)
    {
        lv_indev_set_group(indev, group);
        indev = lv_indev_get_next(indev);
    }
}

/**
 * Create new focus group and make it current.
 */
lv_group_t *push_focus_group()
{
    FocusGroup *item = lv_mem_alloc(sizeof(FocusGroup));
    item->group = lv_group_create();
    item->next = groups;
    groups = item;
    set_focus_group(item->group);
    return item->group;
}

/**
 * Restore previous focus group.
 */
lv_group_t *pop_focus_group()
{
    if (groups->next == NULL)
    {
        hal_printf("ERROR: cannot pop focus group, empty\n");
        return NULL;
    }

    FocusGroup *old = groups;
    groups = groups->next;
    lv_group_del(old->group);
    lv_mem_free(old);
    set_focus_group(groups->group);
    return groups->group;
}

void settings_show()
{
}

static void main_event_cb(lv_obj_t *obj, lv_event_t event)
{
    hal_printf("main: obj=%p, e=%d\n", obj, event);
    if (event == LV_EVENT_KEY)
    {
        uint32_t key = *((uint32_t *)lv_event_get_data());
        bool wait_release = false;
        switch (key)
        {
        case LV_KEY_ENTER:
            settings_show();
            wait_release = true;
            break;
        case 'A':
            hal_printf("A\n");
            wait_release = true;
            break;
        case 'B':
            hal_printf("B\n");
            wait_release = true;
            break;
        case 'C':
            hal_printf("C\n");
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

void app_init()
{
    lv_init();
    hal_init();
    lv_log_register_print_cb(print_lv_log_cb);

    hal_printf("Thermal Camera\n");
    hal_printf("Copyright (C) 2020 Martin Poelstra\n\n");

    while (!thermal_init())
    {
        hal_printf("Camera initialization failed, retrying...\n");
        hal_sleep(5000);
    }
    hal_printf("Camera initialized.\n");

    thermal_img_init();

    lv_obj_set_event_cb(lv_scr_act(), main_event_cb);
    lv_group_t *group = push_focus_group();
    lv_group_add_obj(group, lv_scr_act());
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
