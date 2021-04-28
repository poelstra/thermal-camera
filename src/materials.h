#ifndef MATERIALS_H
#define MATERIALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * Special material index, denoting 'custom'.
 */
#define MATERIALS_INDEX_CUSTOM (0)

typedef struct
{
    /**
     * Emissivity value of material, scaled by 1000 (i.e. values are 0..1000 inclusive).
     */
    uint16_t emissivity;

    /**
     * Name of material.
     */
    const char *name;
} Material;

/**
 * List of emissivities of common materials.
 * The first element (index 0) is a placeholder for a custom emissivity.
 */
extern const Material materials[];

/**
 * Number of materials in materials list (including the '<custom>' option).
 */
extern const uint8_t materials_count;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*MATERIALS_H*/
