#include "materials.h"

#include <stdint.h>

#define MATERIAL(value, label)                                                                                         \
    {                                                                                                                  \
        .emissivity = value * 1000, .name = #value ": " label                                                          \
    }

// The following list was compiled from multiple sources, and
// contains approximate values only.
//
// WARNING: When updating this list, also increase SETTINGS_VERSION
// clang-format off
const Material materials[] = {
    MATERIAL(0.00, "<custom>"),
    MATERIAL(1.00, "<black body>"),
    MATERIAL(0.03, "Aluminum foil"),
    MATERIAL(0.77, "Aluminum anodized"),
    MATERIAL(0.88, "Asphalt"),
    MATERIAL(0.83, "Brick"),
    MATERIAL(0.54, "Cement"),
    MATERIAL(0.96, "Charcoal powder"),
    MATERIAL(0.10, "Chromium polished"),
    MATERIAL(0.93, "Concrete"),
    MATERIAL(0.04, "Copper polished"),
    MATERIAL(0.65, "Copper oxidized"),
    MATERIAL(0.92, "Glass"),
    MATERIAL(0.98, "Human skin"),
    MATERIAL(0.78, "Human corrected"),
    MATERIAL(0.89, "Plaster"),
    MATERIAL(0.94, "Plastic acrylic"),
    MATERIAL(0.95, "Plastic black"),
    MATERIAL(0.84, "Plastic white"),
    MATERIAL(0.82, "Plywood"),
    MATERIAL(0.95, "Rubber"),
    MATERIAL(0.80, "Snow"),
    MATERIAL(0.59, "Stainless steel"),
    MATERIAL(0.28, "Steel galvanized"),
    MATERIAL(0.97, "Tape black electrical"),
    MATERIAL(0.92, "Tape masking"),
    MATERIAL(0.95, "Water"),
};
// clang-format on

const uint8_t materials_count = sizeof materials / sizeof materials[0];
