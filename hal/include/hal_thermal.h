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
bool hal_thermal_init();

/**
 * Check whether new thermal data is available and if so, read it.
 * @param pixels Array of pixels to fill with temperature in degrees Celsius
 * @param emissivity Emmisivity to use for calculation (0.0 .. 1.0)
 * @param auto_tr When true, derive ambient temperature from IR sensor's built-in sensor,
 *      and update `tr` with that value. When false, use user-supplied value of `tr`.
 * @param tr Reflected temperature to use for calculation. Must be provided by user when
 *      `auto_tr` is false, will be filled in with in-use value when non-NULL and `auto_tr`
 *      is true and data is available.
 * @returns true when new sub-page has been read (all pixels will have been updated), false
 *      if no data was available or an error occurred.
 */
bool hal_thermal_tick(float pixels[THERMAL_COLS * THERMAL_ROWS], float emissivity, bool auto_tr, float *tr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*HAL_THERMAL_H*/
