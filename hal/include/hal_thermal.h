#ifndef HAL_THERMAL_H
#define HAL_THERMAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define THERMAL_COLS 16
#define THERMAL_ROWS 12

/**
 * Initialize IR hardware.
 *
 * @return true when initialization successful, false otherwise
 */
bool thermal_init();

/**
 * Check whether new thermal data is available and if so, read it.
 * @param pixels Array of pixels to fill with temperature in degrees Celsius
 * @param emissivity Emmisivity to use for calculation (0.0 .. 1.0)
 * @param tr Reflected temperature to use for calculation, or NULL to derive ambient
 *      ambient temperature using IR sensor's built-in temperature sensor.
 * @returns true when new sub-page has been read (all pixels will have been updated), false
 *      if no data was available or an error occurred.
 */
bool thermal_tick(float pixels[THERMAL_COLS * THERMAL_ROWS], float emissivity, float *tr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*HAL_THERMAL_H*/
