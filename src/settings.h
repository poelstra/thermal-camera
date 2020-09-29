#ifndef SETTINGS_H
#define SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct
{
    /**
     * Emissivity of measured object.
     * Value between [0..1] (inclusive).
     * Use e.g. 0.95 for typical matte objects.
     */
    float emissivity;

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
