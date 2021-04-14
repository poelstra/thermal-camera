#include "thermal_img.h"

#include "colorscale.h"
#include "hal_thermal.h"

#include <lvgl.h>
#include <math.h>

#define ZOOM 16
#define THERMAL_IMG_WIDTH ((lv_coord_t)(THERMAL_COLS * ZOOM))
#define THERMAL_IMG_HEIGHT ((lv_coord_t)(THERMAL_ROWS * ZOOM))

#define CROSSHAIR_WIDTH ((lv_coord_t)(2 * ZOOM))
#define CROSSHAIR_HEIGHT ((lv_coord_t)(2 * ZOOM))

#define LEGEND_WIDTH ((lv_coord_t)(16))
#define LEGEND_HEIGHT ((lv_coord_t)(128))

/**
 * Speed of updating auto-range values [0.0 .. 1.0].
 * 0.0 = no update
 * 1.0 = immediate update
 */
#define AUTO_RANGE_SPEED (0.05)

/**
 * Minimum distance between minimum and maximum temperature
 * (in degrees Celsius) to prevent just noise from being
 * displayed.
 */
#define AUTO_RANGE_MIN_DIFF (10)

static lv_design_cb_t thermal_img_ancestor_design;
static lv_obj_t *thermal_img_obj;
static uint8_t thermal_pixels[THERMAL_ROWS * THERMAL_COLS];

static lv_color_t colorMap[256];

/**
 * Cache of min/max temperature ranges for color scale.
 * Automatically updated from settings if needed.
 */
static float thermal_min_temp = 25.0;
static float thermal_max_temp = 38.0;

static lv_obj_t *legend_min_label;
static lv_obj_t *legend_max_label;
static lv_obj_t *measurement_min_label;
static lv_obj_t *measurement_max_label;
static lv_obj_t *measurement_center_label;

static lv_design_res_t thermal_img_design(lv_obj_t *img, const lv_area_t *clip_area, lv_design_mode_t mode)
{
    if (mode == LV_DESIGN_COVER_CHK)
    {
        return thermal_img_ancestor_design(img, clip_area, mode);
    }
    else if (mode == LV_DESIGN_DRAW_MAIN)
    {
        lv_design_res_t res = thermal_img_ancestor_design(img, clip_area, mode);
        if (res != LV_DESIGN_RES_OK)
        {
            return res;
        }

        lv_draw_rect_dsc_t draw_dsc;
        lv_draw_rect_dsc_init(&draw_dsc);

        lv_area_t bounds;
        lv_area_copy(&bounds, clip_area);
        bounds.x1 -= img->coords.x1;
        bounds.x2 -= img->coords.x1;
        bounds.y1 -= img->coords.y1;
        bounds.y2 -= img->coords.y1;
        bounds.x1 /= ZOOM;
        bounds.y1 /= ZOOM;
        bounds.x2 += ZOOM - 1;
        bounds.y2 += ZOOM - 1;
        bounds.x2 /= ZOOM;
        bounds.y2 /= ZOOM;
        if (bounds.x1 < 0)
        {
            bounds.x1 = 0;
        }
        if (bounds.y1 < 0)
        {
            bounds.y1 = 0;
        }
        if (bounds.x2 > THERMAL_COLS)
        {
            bounds.x2 = THERMAL_COLS;
        }
        if (bounds.y2 > THERMAL_ROWS)
        {
            bounds.y2 = THERMAL_ROWS;
        }

        for (uint16_t y = bounds.y1; y < bounds.y2; y++)
        {
            lv_coord_t y1 = y * ZOOM + img->coords.y1;
            uint16_t y_index = y * THERMAL_COLS;
            for (uint16_t x = bounds.x1; x < bounds.x2; x++)
            {
                lv_coord_t x1 = x * ZOOM + img->coords.x1;
                lv_area_t coords;
                lv_area_set(&coords, x1, y1, x1 + ZOOM - 1, y1 + ZOOM - 1);
                draw_dsc.bg_color = colorMap[thermal_pixels[y_index + x]];
                lv_draw_rect(&coords, clip_area, &draw_dsc);
            }
        }
    }
    else if (mode == LV_DESIGN_DRAW_POST)
    {
        return thermal_img_ancestor_design(img, clip_area, mode);
    }

    return LV_DESIGN_RES_OK;
}

void thermal_img_init()
{
    colorscale_build(colorMap);

    // IR camera image
    thermal_img_obj = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_clean_style_list(thermal_img_obj, LV_OBJ_PART_MAIN);
    lv_obj_set_size(thermal_img_obj, THERMAL_IMG_WIDTH, THERMAL_IMG_HEIGHT);
    lv_obj_align(thermal_img_obj, NULL, LV_ALIGN_IN_LEFT_MID, 4, 0);
    thermal_img_ancestor_design = lv_obj_get_design_cb(thermal_img_obj);
    lv_obj_set_design_cb(thermal_img_obj, thermal_img_design);
    lv_obj_set_click(thermal_img_obj, false);
    lv_obj_set_adv_hittest(thermal_img_obj, true);

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
    legend_max_label = lv_label_create(lv_scr_act(), NULL);
    lv_obj_set_auto_realign(legend_max_label, true);
    lv_obj_align(legend_max_label, legend, LV_ALIGN_OUT_TOP_MID, 0, 0);
    lv_label_set_align(legend_max_label, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text_fmt(legend_max_label, "%.0f", thermal_max_temp);

    legend_min_label = lv_label_create(lv_scr_act(), NULL);
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

static uint8_t temp_to_intensity(float temp)
{
    if (temp < thermal_min_temp)
    {
        temp = thermal_min_temp;
    }
    if (temp > thermal_max_temp)
    {
        temp = thermal_max_temp;
    }
    return 255 * (temp - thermal_min_temp) / (thermal_max_temp - thermal_min_temp);
}

static lv_color_t temp_to_color(float temp)
{
    return colorMap[temp_to_intensity(temp)];
}

static uint32_t temp_to_rgb(float temp)
{
    lv_color_t color = temp_to_color(temp);
    return lv_color_to32(color) & 0xffffff;
}

void thermal_img_update(const float pixels[THERMAL_ROWS * THERMAL_COLS], Settings *settings)
{
    float center_temp = get_temp(pixels, THERMAL_COLS / 2, THERMAL_ROWS / 2);
    float min_temp = center_temp;
    float max_temp = center_temp;

    // Determine min/max temperature
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
        }
    }

    // Apply auto-ranging if needed
    if (settings->auto_range)
    {
        float new_min_temp = settings->min_temp;
        new_min_temp *= (1.0 - AUTO_RANGE_SPEED);
        new_min_temp += min_temp * AUTO_RANGE_SPEED;
        new_min_temp = new_min_temp;

        float new_max_temp = settings->max_temp;
        new_max_temp *= (1.0 - AUTO_RANGE_SPEED);
        new_max_temp += max_temp * AUTO_RANGE_SPEED;
        new_max_temp = new_max_temp;

        // Ensure min and max are at least a certain amount of degrees separated,
        // to prevent showing just an enormous amount of noise
        if (new_max_temp - new_min_temp < 10)
        {
            float avg = (new_max_temp + new_min_temp) / 2.0;
            new_min_temp = avg - AUTO_RANGE_MIN_DIFF / 2;
            new_max_temp = avg + AUTO_RANGE_MIN_DIFF / 2;
        }
        settings->min_temp = new_min_temp;
        settings->max_temp = new_max_temp;
    }

    // Apply latest range settings
    if (settings->min_temp != thermal_min_temp)
    {
        thermal_min_temp = settings->min_temp;
        lv_label_set_text_fmt(legend_min_label, "%.0f", thermal_min_temp);
    }
    if (settings->max_temp != thermal_max_temp)
    {
        thermal_max_temp = settings->max_temp;
        lv_label_set_text_fmt(legend_max_label, "%.0f", thermal_max_temp);
    }

    // Convert temperature to color and update image
    for (uint16_t y = 0; y < THERMAL_ROWS; y++)
    {
        for (uint16_t x = 0; x < THERMAL_COLS; x++)
        {
            float pixel = get_temp(pixels, x, y);
            thermal_pixels[y * THERMAL_COLS + x] = temp_to_intensity(pixel);
        }
    }
    lv_obj_invalidate(thermal_img_obj);

    lv_label_set_text_fmt(measurement_min_label, "#%06x " LV_SYMBOL_DOWN "# %.1f ", temp_to_rgb(min_temp), min_temp);
    lv_label_set_text_fmt(measurement_center_label, "#%06x " CUSTOM_SYMBOL_EXPAND "# %.1f ", temp_to_rgb(center_temp),
                          center_temp);
    lv_label_set_text_fmt(measurement_max_label, "#%06x " LV_SYMBOL_UP "# %#.1f ", temp_to_rgb(max_temp), max_temp);
}
