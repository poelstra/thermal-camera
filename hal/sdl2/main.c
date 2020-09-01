#include "app_main.h"

#include "hal_time.h"

#include "sdl.h"
#include <lvgl.h>
#include <stdbool.h>

int main(void)
{
    app_init();

    while (true)
    {
        app_tick();
        hal_sleep(5);
    }

    return 0;
}
