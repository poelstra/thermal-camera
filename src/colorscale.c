#include "colorscale.h"

typedef struct
{
    lv_color_t color;
    uint8_t position;
} ColorStop;

void colorscale_build(lv_color_t map[256])
{
    /* clang-format off */
    const ColorStop stops[] = {
        {.color = LV_COLOR_BLUE,    .position = 255 * 0.0},        
        {.color = LV_COLOR_CYAN,    .position = 255 * 0.2},
        {.color = LV_COLOR_LIME,    .position = 255 * 0.3},
        {.color = LV_COLOR_YELLOW,  .position = 255 * 0.5},
        {.color = LV_COLOR_RED,     .position = 255 * 0.8},
        {.color = LV_COLOR_MAGENTA, .position = 255 * 1.0},
    };
    /* clang-format on */

    int firstIndex = 0;
    int secondIndex = 1;
    for (uint16_t i = 0; i < 256; i++)
    {
        if (i > stops[secondIndex].position)
        {
            firstIndex++;
            secondIndex++;
        }

        ColorStop first = stops[firstIndex];
        ColorStop second = stops[secondIndex];
        uint8_t mix = 255 * (i - first.position) / (second.position - first.position);
        map[i] = lv_color_mix(second.color, first.color, mix);
    }
}
