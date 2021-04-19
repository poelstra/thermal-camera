#include "header.h"

#include <lvgl.h>

static lv_obj_t *emissivity_label;
static lv_obj_t *reflected_label;
static lv_obj_t *range_label;

void header_init()
{
    emissivity_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text_fmt(emissivity_label, CUSTOM_SYMBOL_VIAL " 0.00");
    lv_obj_set_pos(emissivity_label, 4, 2);

    reflected_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text_fmt(reflected_label, CUSTOM_SYMBOL_THERMOMETER_HALF " 000.0");
    lv_obj_set_pos(reflected_label, 100, 2);

    range_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text_fmt(range_label, CUSTOM_SYMBOL_VIAL " 000.0");
    lv_obj_set_pos(range_label, 200, 2);
}

void header_update(const Settings *settings)
{
    lv_label_set_text_fmt(emissivity_label, CUSTOM_SYMBOL_VIAL " %.2f", settings->emissivity);
    if (settings->auto_ambient)
    {
        lv_label_set_text_fmt(reflected_label, CUSTOM_SYMBOL_THERMOMETER_HALF " %.1f", settings->reflected_temperature);
    }
    else
    {
        lv_label_set_text_fmt(reflected_label, CUSTOM_SYMBOL_EYE_DROPPER " %.1f", settings->reflected_temperature);
    }
    if (settings->auto_range)
    {
        lv_label_set_text_fmt(range_label, CUSTOM_SYMBOL_RULER_VERTICAL " auto");
    }
    else
    {
        lv_label_set_text_fmt(range_label, CUSTOM_SYMBOL_RULER_VERTICAL " custom");
    }
}
