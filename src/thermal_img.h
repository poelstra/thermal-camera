#ifndef THERMAL_IMG_H
#define THERMAL_IMG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_thermal.h"
#include "settings.h"

void thermal_img_init();
void thermal_img_update(const float pixels[THERMAL_ROWS * THERMAL_COLS], Settings *settings);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*THERMAL_IMG_H*/
