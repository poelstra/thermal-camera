#include "sdl.h"

#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
#include "display/monitor.h"
#include "indev/keyboard.h"
#include <SDL2/SDL.h>

#include "hal_input.h"
#include "hal_time.h"

bool hal_keyboard_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    bool result = keyboard_read(indev_drv, data);

    switch (data->key)
    {
    case LV_KEY_LEFT:
    case LV_KEY_RIGHT:
    case LV_KEY_UP:
    case LV_KEY_DOWN:
    case LV_KEY_ENTER:
    case 'B':
    case 'C':
        break;

    case LV_KEY_ESC:
    case 'A':
    case 'a':
        data->key = LV_KEY_ESC;
        break;

    case 'b':
        data->key = 'B';
        break;
    case 'c':
        data->key = 'C';
        break;

    default:
        // Only allow keys that also exist on the
        // WIO terminal to facilitate realistic
        // navigation during development.
        data->state = LV_INDEV_STATE_REL;
    }

    return result;
}

void sdl_init()
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
}
