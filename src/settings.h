#ifndef SETTINGS_H
#define SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * Magic number for recognizing valid settings in flash storage.
 */
#define SETTINGS_MAGIC (0x5e7716a1)

/**
 * Current settings version. Increment for every change to the
 * struct below.
 */
#define SETTINGS_VERSION (3)

typedef struct
{
    /**
     * Index in material array.
     * When 0, a custom emissivity is used (see below),
     * otherwise the value from a pre-defined list of
     * materials is used.
     */
    uint8_t material_index;

    /**
     * Custom emissivity, used when material_index is 0.
     * Value between [0..1] (inclusive).
     * Use e.g. 0.95 for typical matte objects.
     */
    float custom_emissivity;

    /**
     * Use built-in ambient temperature sensor.
     */
    bool auto_ambient;

    /**
     * Reflected temperature to use.
     * Used when `auto_ambient` is `false`, but can be
     * set externally to the currently measured automatic
     * value to give the user an indication of the actual
     * in-use value when `auto_ambient` is `true`.
     */
    float reflected_temperature;

    /**
     * Whether to automatically update min/max temperature
     * of color scale based on live image.
     */
    bool auto_range;

    /**
     * Minimum temperature on color scale.
     */
    float min_temp;

    /**
     * Maximum temperature on color scale.
     */
    float max_temp;

    /**
     * Flip image horizontally.
     */
    bool flip_hor;

    /**
     * Flip image vertically.
     */
    bool flip_ver;
} Settings;

typedef void (*SettingsClosedCallback)(void);

void settings_show(Settings *settings, SettingsClosedCallback closed_cb);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*SETTINGS_H*/
