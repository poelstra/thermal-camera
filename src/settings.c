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
    const lv_obj_t *focused = lv_group_get_focused(group);
    if (!focused)
    {
        return;
    }

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
        lv_obj_set_state(settings_win->reflected, LV_STATE_DEFAULT);
        if (lv_obj_get_group(settings_win->reflected) == NULL)
        {
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

static void auto_ambient_event_cb(lv_obj_t *checkbox, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        settings_win->settings->auto_ambient = !lv_checkbox_is_checked(checkbox);
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

    row = lv_cont_create(settings_win->win, row);
    label = lv_label_create(row, label);
    lv_obj_t *checkbox = lv_checkbox_create(row, NULL);
    settings_win->auto_ambient = checkbox;
    lv_group_add_obj(group, checkbox);
    lv_checkbox_set_text(checkbox, "");
    lv_checkbox_set_checked(checkbox, !settings_win->settings->auto_ambient);
    spinbox = lv_spinbox_create(row, NULL);
    settings_win->reflected = spinbox;
    lv_spinbox_set_range(spinbox, -1000, +6000);
    lv_spinbox_set_digit_format(spinbox, 4, 3);
    lv_spinbox_set_value(spinbox, settings_win->settings->reflected_temperature * 10 + 0.5);
    lv_spinbox_step_prev(spinbox);
    lv_group_add_obj(group, spinbox);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width_margin(label, lv_obj_get_width_fit(row) - lv_obj_get_width_margin(checkbox) -
                                       lv_obj_get_width_margin(spinbox) - 2 * padding);
    lv_label_set_text(label, "Custom reflected temperature");

    lv_obj_t *close_btn = lv_btn_create(settings_win->win, NULL);
    settings_win->close = close_btn;
    lv_obj_set_event_cb(close_btn, settings_close_cb);
    lv_group_add_obj(group, close_btn);
    lv_obj_t *close_label = lv_label_create(close_btn, NULL);
    lv_label_set_text(close_label, "Close");

    lv_obj_set_event_cb(settings_win->emissivity, emissivity_event_cb);
    lv_obj_set_event_cb(settings_win->reflected, reflected_event_cb);
    lv_obj_set_event_cb(settings_win->auto_ambient, auto_ambient_event_cb);

    lv_group_set_focus_cb(group, settings_focus_cb);
    settings_focus_cb(group);
    sync_settings();

    settings_win->sync_task = lv_task_create(sync_settings_task, 1000, LV_TASK_PRIO_LOW, NULL);
}
