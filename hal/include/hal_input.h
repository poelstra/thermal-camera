#ifndef HAL_INPUT_H
#define HAL_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>
#include <stdbool.h>

bool hal_keyboard_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*HAL_INPUT_H*/
