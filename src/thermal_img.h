#ifndef THERMAL_IMG_H
#define THERMAL_IMG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_thermal.h"

void thermal_img_init();
void thermal_img_update(const float pixels[THERMAL_ROWS * THERMAL_COLS]);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*THERMAL_IMG_H*/
