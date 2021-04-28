#include "header.h"

#include "focus.h"
#include "materials.h"

#include <lvgl.h>

static lv_obj_t *emissivity_label;
static lv_obj_t *emissivity_dropdown;
static lv_obj_t *reflected_label;
static lv_obj_t *range_label;
static Settings *current_settings;

static void emissivity_dropdown_handler(lv_obj_t *obj, lv_event_t event)
{
    bool close = false;

    if (event == LV_EVENT_VALUE_CHANGED)
    {
        uint16_t selected = lv_dropdown_get_selected(obj);
        if (current_settings)
        {
            current_settings->material_index = selected;
        }
        close = true;
    }
    else if (event == LV_EVENT_CANCEL)
    {
        close = true;
    }

    if (close)
    {
        current_settings = NULL;
        focus_pop_group();
    }
}

static char *build_material_options(float custom_emissivity)
{
    char custom_emissivity_text[] = "0.00: custom";
    lv_snprintf(custom_emissivity_text, sizeof custom_emissivity_text, "%.2f: custom", custom_emissivity);
    size_t custom_len = strlen(custom_emissivity_text);

    size_t len = custom_len + 1; // include the \n
    for (uint8_t i = 1; i < materials_count; i++)
    {
        len += strlen(materials[i].name) + 1; // include the \n (or \0 for the last element)
    }

    char *options_text = lv_mem_alloc(len);
    LV_ASSERT_MEM(options_text);

    char *text_index = options_text;
    strncpy(text_index, custom_emissivity_text, custom_len);
    text_index += custom_len;
    for (uint8_t i = 1; i < materials_count; i++)
    {
        *text_index++ = '\n';
        size_t name_len = strlen(materials[i].name);
        strncpy(text_index, materials[i].name, name_len);
        text_index += name_len;
    }

    return options_text;
}

static void update_material_options(float custom_emissivity)
{
    static char *options = NULL;

    if (options)
    {
        lv_mem_free(options);
    }
    options = build_material_options(custom_emissivity);
    lv_dropdown_set_static_options(emissivity_dropdown, options);
}

#define INITIAL_EMISSIVITY (0.95)

void header_init()
{
    emissivity_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text_fmt(emissivity_label, CUSTOM_SYMBOL_VIAL);
    lv_obj_set_pos(emissivity_label, 4, 2);

    emissivity_dropdown = lv_dropdown_create(lv_scr_act(), NULL);
    lv_obj_clean_style_list(emissivity_dropdown, LV_DROPDOWN_PART_MAIN);
    lv_dropdown_set_symbol(emissivity_dropdown, ""); // don't use NULL as it then centers the text
    lv_obj_align(emissivity_dropdown, emissivity_label, LV_ALIGN_OUT_RIGHT_TOP, LV_DPX(5), 0);
    lv_obj_set_event_cb(emissivity_dropdown, emissivity_dropdown_handler);
    lv_obj_set_width(emissivity_dropdown, 170);
    update_material_options(INITIAL_EMISSIVITY);

    reflected_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text_fmt(reflected_label, CUSTOM_SYMBOL_THERMOMETER_HALF " 000.0");
    lv_obj_align(reflected_label, emissivity_dropdown, LV_ALIGN_OUT_RIGHT_TOP, LV_DPX(10), 0);

    range_label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text_fmt(range_label, CUSTOM_SYMBOL_RULER_VERTICAL " auto");
    lv_obj_align(range_label, reflected_label, LV_ALIGN_OUT_RIGHT_TOP, LV_DPX(10), 0);
}

void header_update(const Settings *settings)
{
    static float custom_emissivity = INITIAL_EMISSIVITY;
    static uint8_t material_index = 255;
    if (material_index != settings->material_index || custom_emissivity != settings->custom_emissivity)
    {
        if (custom_emissivity != settings->custom_emissivity)
        {
            custom_emissivity = settings->custom_emissivity;
            update_material_options(settings->custom_emissivity);
        }

        material_index = settings->material_index;
        lv_dropdown_set_selected(emissivity_dropdown, material_index);
    }

    static bool auto_ambient = true;
    static float reflected_temperature = -1000;
    if (auto_ambient != settings->auto_ambient || reflected_temperature != settings->reflected_temperature)
    {
        auto_ambient = settings->auto_ambient;
        reflected_temperature = settings->reflected_temperature;

        if (settings->auto_ambient)
        {
            auto_ambient = settings->auto_ambient;
            lv_label_set_text_fmt(reflected_label, CUSTOM_SYMBOL_THERMOMETER_HALF " %.1f",
                                  settings->reflected_temperature);
        }
        else
        {
            lv_label_set_text_fmt(reflected_label, CUSTOM_SYMBOL_EYE_DROPPER " %.1f", settings->reflected_temperature);
        }
    }

    static bool auto_range = true;
    if (auto_range != settings->auto_range)
    {
        auto_range = settings->auto_range;
        if (settings->auto_range)
        {
            lv_label_set_text_fmt(range_label, CUSTOM_SYMBOL_RULER_VERTICAL " auto");
        }
        else
        {
            lv_label_set_text_fmt(range_label, CUSTOM_SYMBOL_RULER_VERTICAL " custom");
        }
    }
}

void header_edit_emissivity(Settings *settings)
{
    current_settings = settings;

    lv_group_t *group = focus_push_group();
    lv_group_add_obj(group, emissivity_dropdown);
    lv_dropdown_open(emissivity_dropdown);
    lv_group_set_editing(group, true); // Opened the dropdown, so 'focus' on it
}
