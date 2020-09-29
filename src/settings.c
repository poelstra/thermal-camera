#include "settings.h"

#include "focus.h"
#include <lvgl.h>

typedef struct
{
    Settings *settings;
    SettingsClosedCallback closed_cb;

    lv_obj_t *win;
    lv_obj_t *emissivity;
    lv_obj_t *auto_ambient;
    lv_obj_t *reflected;
    lv_obj_t *flip_hor;
    lv_obj_t *flip_ver;
    lv_obj_t *close;

    lv_task_t *sync_task;
} SettingsWindow;

static SettingsWindow *settings_win;

LV_EVENT_CB_DECLARE(settings_close_cb)
{
    if (e == LV_EVENT_CLICKED)
    {
        focus_pop_group();
        lv_obj_del(settings_win->win);
        lv_task_del(settings_win->sync_task);
        if (settings_win->closed_cb)
        {
            settings_win->closed_cb();
        }
        lv_mem_free(settings_win);
        settings_win = NULL;
    }
}

static void activate_textarea(lv_obj_t *area, bool edit)
{
    lv_textarea_set_cursor_hidden(area, !edit);
    if (edit)
    {
        lv_textarea_set_cursor_pos(area, lv_textarea_get_cursor_pos(area));
    }
}

static void settings_focus_cb(lv_group_t *group)
{
    lv_obj_t *focused = lv_group_get_focused(group);
    if (!focused)
    {
        return;
    }

    lv_win_focus(settings_win->win, focused, LV_ANIM_ON);

    bool editing = lv_group_get_editing(group);
    activate_textarea(settings_win->emissivity, focused == settings_win->emissivity && editing);
    activate_textarea(settings_win->reflected, focused == settings_win->reflected && editing);
}

static void emissivity_event_cb(lv_obj_t *spinbox, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        settings_win->settings->emissivity = lv_spinbox_get_value(spinbox) / 100.0;
    }
}

static void sync_settings(void)
{
    if (settings_win->settings->auto_ambient)
    {
        lv_obj_set_state(settings_win->reflected, LV_STATE_DISABLED);
        lv_group_remove_obj(settings_win->reflected);

        lv_spinbox_set_value(settings_win->reflected, settings_win->settings->reflected_temperature * 10 + 0.5);
    }
    else
    {
        if (lv_obj_get_group(settings_win->reflected) == NULL)
        {
            lv_obj_set_state(settings_win->reflected, LV_STATE_DEFAULT);
            lv_group_t *group = lv_obj_get_group(settings_win->close);
            lv_group_remove_obj(settings_win->close);
            lv_group_add_obj(group, settings_win->reflected);
            lv_group_add_obj(group, settings_win->close);
        }
    }
}

void sync_settings_task(lv_task_t *task)
{
    sync_settings();
}

static void checkbox_event_cb(lv_obj_t *checkbox, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        const bool checked = lv_checkbox_is_checked(checkbox);
        if (checkbox == settings_win->auto_ambient)
        {
            settings_win->settings->auto_ambient = checked;
        }
        else if (checkbox == settings_win->flip_hor)
        {
            settings_win->settings->flip_hor = checked;
        }
        else if (checkbox == settings_win->flip_ver)
        {
            settings_win->settings->flip_ver = checked;
        }
        sync_settings();
    }
}

static void reflected_event_cb(lv_obj_t *spinbox, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED && !settings_win->settings->auto_ambient)
    {
        settings_win->settings->reflected_temperature = lv_spinbox_get_value(spinbox) / 10.0;
    }
}

void settings_show(Settings *settings, SettingsClosedCallback closed_cb)
{
    lv_group_t *group = focus_push_group();

    LV_DEBUG_ASSERT(settings_win == NULL, "Settings window already initialized", 0);
    settings_win = lv_mem_alloc(sizeof(SettingsWindow));

    settings_win->settings = settings;
    settings_win->closed_cb = closed_cb;

    settings_win->win = lv_win_create(lv_scr_act(), NULL);
    lv_win_set_title(settings_win->win, "Settings");
    lv_win_set_layout(settings_win->win, LV_LAYOUT_COLUMN_MID);
    lv_win_set_scrollbar_mode(settings_win->win, LV_SCROLLBAR_MODE_OFF);

    // Emissivity
    lv_obj_t *row = lv_cont_create(settings_win->win, NULL);
    lv_coord_t padding = lv_obj_get_style_pad_inner(row, LV_CONT_PART_MAIN);
    lv_cont_set_fit2(row, LV_FIT_PARENT, LV_FIT_TIGHT);
    lv_cont_set_layout(row, LV_LAYOUT_ROW_MID);
    lv_obj_t *label = lv_label_create(row, NULL);
    lv_obj_t *spinbox = lv_spinbox_create(row, NULL);
    settings_win->emissivity = spinbox;
    lv_spinbox_set_range(spinbox, 0, 100);
    lv_spinbox_set_digit_format(spinbox, 3, 1);
    lv_spinbox_set_value(spinbox, settings_win->settings->emissivity * 100 + 0.5);
    lv_group_add_obj(group, spinbox);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width_margin(label, lv_obj_get_width_fit(row) - lv_obj_get_width_margin(spinbox) - padding);
    lv_label_set_text(label, "Emissivity");

    // Reflected temperature
    row = lv_cont_create(settings_win->win, row);
    label = lv_label_create(row, label);

    // Create a cell that fits both elements, but make sure that the container itself
    // has enough space to allow e.g. outline to be drawn on elements when necessary, i.e.
    // 'move' the row's padding into this container.
    lv_obj_t *cell = lv_cont_create(row, NULL);
    lv_cont_set_fit(cell, LV_FIT_TIGHT);
    lv_obj_clean_style_list(cell, LV_CONT_PART_MAIN);
    const lv_coord_t pad_outer = lv_obj_get_style_pad_right(row, LV_CONT_PART_MAIN);
    lv_obj_set_style_local_margin_all(cell, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, -pad_outer);
    lv_obj_set_style_local_pad_all(cell, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, pad_outer);
    lv_obj_set_style_local_pad_inner(cell, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, padding);

    lv_obj_t *checkbox = lv_checkbox_create(cell, NULL);
    settings_win->auto_ambient = checkbox;
    lv_group_add_obj(group, checkbox);
    lv_checkbox_set_text(checkbox, "Auto");
    lv_checkbox_set_checked(checkbox, settings_win->settings->auto_ambient);
    spinbox = lv_spinbox_create(cell, NULL);
    settings_win->reflected = spinbox;
    lv_spinbox_set_range(spinbox, -1000, +6000);
    lv_spinbox_set_digit_format(spinbox, 4, 3);
    lv_spinbox_set_value(spinbox, settings_win->settings->reflected_temperature * 10 + 0.5);
    lv_spinbox_step_prev(spinbox);
    lv_group_add_obj(group, spinbox);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width_margin(label, lv_obj_get_width_fit(row) - lv_obj_get_width_margin(cell) - padding - pad_outer);
    lv_label_set_text(label, "Reflected temperature");
    lv_cont_set_layout(cell, LV_LAYOUT_COLUMN_RIGHT);

    // Flip horizontally
    row = lv_cont_create(settings_win->win, row);
    label = lv_label_create(row, label);
    checkbox = lv_checkbox_create(row, NULL);
    lv_obj_set_style_local_pad_inner(checkbox, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, 0);
    settings_win->flip_hor = checkbox;
    lv_group_add_obj(group, checkbox);
    lv_checkbox_set_text(checkbox, "");
    lv_checkbox_set_checked(checkbox, settings_win->settings->flip_hor);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width_margin(label, lv_obj_get_width_fit(row) - lv_obj_get_width_margin(checkbox) - padding);
    lv_label_set_text(label, "Flip horizontally");

    // Flip vertically
    row = lv_cont_create(settings_win->win, row);
    label = lv_label_create(row, label);
    checkbox = lv_checkbox_create(row, NULL);
    lv_obj_set_style_local_pad_inner(checkbox, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, 0);
    settings_win->flip_ver = checkbox;
    lv_group_add_obj(group, checkbox);
    lv_checkbox_set_text(checkbox, "");
    lv_checkbox_set_checked(checkbox, settings_win->settings->flip_ver);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width_margin(label, lv_obj_get_width_fit(row) - lv_obj_get_width_margin(checkbox) - padding);
    lv_label_set_text(label, "Flip vertically");

    // Close
    lv_obj_t *close_btn = lv_btn_create(settings_win->win, NULL);
    settings_win->close = close_btn;
    lv_obj_set_event_cb(close_btn, settings_close_cb);
    lv_group_add_obj(group, close_btn);
    lv_obj_t *close_label = lv_label_create(close_btn, NULL);
    lv_label_set_text(close_label, "Close");

    // Callbacks
    lv_obj_set_event_cb(settings_win->emissivity, emissivity_event_cb);
    lv_obj_set_event_cb(settings_win->reflected, reflected_event_cb);
    lv_obj_set_event_cb(settings_win->auto_ambient, checkbox_event_cb);
    lv_obj_set_event_cb(settings_win->flip_hor, checkbox_event_cb);
    lv_obj_set_event_cb(settings_win->flip_ver, checkbox_event_cb);

    lv_group_set_focus_cb(group, settings_focus_cb);
    settings_focus_cb(group);
    sync_settings();

    settings_win->sync_task = lv_task_create(sync_settings_task, 1000, LV_TASK_PRIO_LOW, NULL);
}
