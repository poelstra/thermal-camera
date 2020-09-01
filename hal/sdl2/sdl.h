#ifndef HAL_SDL_H
#define HAL_SDL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>

void sdl_init(lv_group_t *keyboard_group);
void sdl_tick(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*HAL_SDL_H*/
