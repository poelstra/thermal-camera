#include "thermal_img.h"

#include "colorscale.h"
#include "hal_thermal.h"

#include <lvgl.h>

#define ZOOM 16
#define THERMAL_IMG_WIDTH ((lv_coord_t)(THERMAL_COLS * ZOOM))
#define THERMAL_IMG_HEIGHT ((lv_coord_t)(THERMAL_ROWS * ZOOM))

#define CROSSHAIR_WIDTH ((lv_coord_t)(2 * ZOOM))
#define CROSSHAIR_HEIGHT ((lv_coord_t)(2 * ZOOM))

#define LEGEND_WIDTH ((lv_coord_t)(16))
#define LEGEND_HEIGHT ((lv_coord_t)(128))

static lv_color_t thermal_pixels[THERMAL_IMG_WIDTH * THERMAL_IMG_HEIGHT];
static lv_img_dsc_t thermal_img_desc = {
    .header =
        {
            .cf = LV_IMG_CF_TRUE_COLOR,
            .always_zero = 0,
            .reserved = 0,
            .w = THERMAL_IMG_WIDTH,
            .h = THERMAL_IMG_HEIGHT,
        },
    .data_size = sizeof(thermal_pixels),
    .data = (const uint8_t *)thermal_pixels,
};
static lv_obj_t *thermal_img_obj;

static lv_color_t colorMap[256];

static float thermal_min_temp = 25.0;
static float thermal_max_temp = 38.0;
static lv_obj_t *measurement_min_label;
static lv_obj_t *measurement_max_label;
static lv_obj_t *measurement_center_label;

void thermal_img_init()
{
    colorscale_build(colorMap);

    // IR camera image
    thermal_img_obj = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(thermal_img_obj, &thermal_img_desc);
    lv_obj_align(thermal_img_obj, NULL, LV_ALIGN_IN_LEFT_MID, 4, 0);

    // Central pixel marker ('crosshair')
    const uint8_t crosshair_x = THERMAL_COLS / 2;
    const uint8_t crosshair_y = THERMAL_ROWS / 2;
    static lv_color_t crosshair_buffer[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CROSSHAIR_WIDTH, CROSSHAIR_HEIGHT)];
    lv_obj_t *crosshair = lv_canvas_create(thermal_img_obj, NULL);
    lv_canvas_set_buffer(crosshair, crosshair_buffer, CROSSHAIR_WIDTH, CROSSHAIR_HEIGHT, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_obj_set_size(crosshair, CROSSHAIR_WIDTH, CROSSHAIR_HEIGHT);
    lv_obj_set_pos(crosshair, (lv_coord_t)crosshair_x * ZOOM + ZOOM / 2 - CROSSHAIR_WIDTH / 2,
                   (lv_coord_t)crosshair_y * ZOOM + ZOOM / 2 - CROSSHAIR_HEIGHT / 2);
    lv_draw_rect_dsc_t crosshair_rect_dsc;
    lv_draw_rect_dsc_init(&crosshair_rect_dsc);
    crosshair_rect_dsc.bg_opa = LV_OPA_TRANSP;
    crosshair_rect_dsc.border_color = LV_COLOR_BLACK;
    crosshair_rect_dsc.border_width = 3;
    crosshair_rect_dsc.border_opa = LV_OPA_COVER * 0.25;
    lv_canvas_draw_rect(crosshair, 0.5 * ZOOM - 1, 0.5 * ZOOM - 1, ZOOM + 2, ZOOM + 2, &crosshair_rect_dsc);
    crosshair_rect_dsc.border_color = LV_COLOR_WHITE;
    crosshair_rect_dsc.border_width = 1;
    crosshair_rect_dsc.border_opa = LV_OPA_COVER;
    lv_canvas_draw_rect(crosshair, 0.5 * ZOOM, 0.5 * ZOOM, ZOOM, ZOOM, &crosshair_rect_dsc);

    // Color scale legend ('gradient')
    static lv_color_t legend_buffer[LV_CANVAS_BUF_SIZE_TRUE_COLOR(LEGEND_WIDTH, LEGEND_HEIGHT)];
    lv_obj_t *legend = lv_canvas_create(lv_scr_act(), NULL);
    lv_canvas_set_buffer(legend, legend_buffer, LEGEND_WIDTH, LEGEND_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_size(legend, LEGEND_WIDTH, LEGEND_HEIGHT);
    lv_obj_align(legend, NULL, LV_ALIGN_IN_RIGHT_MID, -16, 0);
    lv_draw_rect_dsc_t legend_rect_dsc;
    lv_draw_rect_dsc_init(&legend_rect_dsc);
    for (int y = 0; y < LEGEND_HEIGHT; y++)
    {
        legend_rect_dsc.bg_color = colorMap[y * 256 / LEGEND_HEIGHT];
        lv_canvas_draw_rect(legend, 0, LEGEND_HEIGHT - y - 1, 16, 1, &legend_rect_dsc);
    }

    // Color scale min/max labels
    lv_obj_t *legend_max_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(legend_max_label, true);
    lv_obj_align(legend_max_label, legend, LV_ALIGN_OUT_TOP_MID, 0, 0);
    lv_label_set_align(legend_max_label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text_fmt(legend_max_label, "%.0f", thermal_max_temp);

    lv_obj_t *legend_min_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(legend_min_label, true);
    lv_obj_align(legend_min_label, legend, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_label_set_align(legend_min_label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text_fmt(legend_min_label, "%.0f", thermal_min_temp);

    // Measurements
    measurement_min_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_recolor(measurement_min_label, true);
    lv_label_set_text_fmt(measurement_min_label, LV_SYMBOL_DOWN " 000.0");
    lv_obj_align(measurement_min_label, thermal_img_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 2, 2);

    measurement_center_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_recolor(measurement_center_label, true);
    lv_label_set_text_fmt(measurement_center_label, LV_SYMBOL_PLUS " 000.0");
    lv_obj_align(measurement_center_label, thermal_img_obj, LV_ALIGN_OUT_BOTTOM_MID, 2, 2);

    measurement_max_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_recolor(measurement_max_label, true);
    lv_label_set_text_fmt(measurement_max_label, LV_SYMBOL_UP " 000.0");
    lv_obj_align(measurement_max_label, thermal_img_obj, LV_ALIGN_OUT_BOTTOM_RIGHT, 2, 2);
}

static float get_temp(const float *pixel, uint16_t x, uint16_t y)
{
    return pixel[x + y * THERMAL_COLS];
}

static void set_color(uint16_t x, uint16_t y, lv_color_t color)
{
    x *= ZOOM;
    y *= ZOOM;
    for (uint16_t dx = 0; dx < ZOOM; dx++)
    {
        for (uint16_t dy = 0; dy < ZOOM; dy++)
        {
            thermal_pixels[x + dx + (y + dy) * THERMAL_COLS * ZOOM] = color;
        }
    }
}

static lv_color_t temp_to_color(float temp)
{
    if (temp < thermal_min_temp)
    {
        temp = thermal_min_temp;
    }
    if (temp > thermal_max_temp)
    {
        temp = thermal_max_temp;
    }
    uint8_t intensity = 255 * (temp - thermal_min_temp) / (thermal_max_temp - thermal_min_temp);
    return colorMap[intensity];
}

static uint32_t temp_to_rgb(float temp)
{
    lv_color_t color = temp_to_color(temp);
    return lv_color_to32(color) & 0xffffff;
}

void thermal_img_update(const float pixels[THERMAL_ROWS * THERMAL_COLS])
{
    float center_temp = get_temp(pixels, THERMAL_COLS / 2, THERMAL_ROWS / 2);
    float min_temp = center_temp;
    float max_temp = center_temp;

    for (uint16_t y = 0; y < THERMAL_ROWS; y++)
    {
        for (uint16_t x = 0; x < THERMAL_COLS; x++)
        {
            float pixel = get_temp(pixels, x, y);
            if (pixel < min_temp)
            {
                min_temp = pixel;
            }
            if (pixel > max_temp)
            {
                max_temp = pixel;
            }
            set_color(x, y, temp_to_color(pixel));
        }
    }
    lv_obj_invalidate(thermal_img_obj);

    lv_label_set_text_fmt(measurement_min_label, "#%06x " LV_SYMBOL_DOWN "# %.1f ", temp_to_rgb(min_temp), min_temp);
    lv_label_set_text_fmt(measurement_center_label, "#%06x " LV_SYMBOL_PLUS "# %.1f ", temp_to_rgb(center_temp),
                          center_temp);
    lv_label_set_text_fmt(measurement_max_label, "#%06x " LV_SYMBOL_UP "# %#.1f ", temp_to_rgb(max_temp), max_temp);
}
