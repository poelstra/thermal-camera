#include "sdl.h"

#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
#include "display/monitor.h"
#include "indev/keyboard.h"
#include <SDL2/SDL.h>

#include "hal_time.h"

static bool joystick_keyboard_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    bool result = keyboard_read(indev_drv, data);

    switch (data->key)
    {
    case LV_KEY_UP:
        data->key = LV_KEY_PREV;
        break;
    case LV_KEY_DOWN:
        data->key = LV_KEY_NEXT;
        break;
    }

    return result;
}

void sdl_init(lv_group_t *keyboard_group)
{
    /* Add a display
     * Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    monitor_init();

    static lv_disp_buf_t disp_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * 10];                  /*Declare a buffer for 10 lines*/
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10); /*Initialize the display buffer*/

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);       /*Basic initialization*/
    disp_drv.flush_cb = monitor_flush; /*Used when `LV_VDB_SIZE != 0` in lv_conf.h (buffered drawing)*/
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    // Initialize keys
    keyboard_init();
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = joystick_keyboard_read;
    lv_indev_t *keypad_indev = lv_indev_drv_register(&indev_drv);
    lv_indev_set_group(keypad_indev, keyboard_group);
}
