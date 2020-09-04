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
    if (event == LV_EVENT_RELEASED)
    {
        settings_show();
        return;
    }
    if (event == LV_EVENT_KEY)
    {
        uint32_t key = *((uint32_t *)lv_event_get_data());
        switch (key)
        {
        case 'A':
            hal_printf("A\n");
            break;
        case 'B':
            hal_printf("B\n");
            break;
        case 'C':
            hal_printf("C\n");
            break;
        }
    }
}

void app_init()
{
    lv_init();
    hal_init();

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
