#include "settings.h"

#include "focus.h"
#include "hal_print.h"
#include "storage.h"
#include <lvgl.h>

typedef struct
{
    Settings *settings;
    SettingsClosedCallback closed_cb;

    lv_obj_t *win;
    lv_obj_t *emissivity;
    lv_obj_t *auto_ambient;
    lv_obj_t *reflected;
    lv_obj_t *auto_range;
    lv_obj_t *max_temp;
    lv_obj_t *min_temp;
    lv_obj_t *flip_hor;
    lv_obj_t *flip_ver;
    lv_obj_t *close_btn;
    lv_obj_t *save_btn;

    lv_group_t *focus_group;
    lv_task_t *sync_task;
} SettingsWindow;

static SettingsWindow *settings_win;

static void settings_close_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED)
    {
        return;
    }

    focus_pop_group();
    lv_obj_del(settings_win->win);
    lv_task_del(settings_win->sync_task);
    SettingsClosedCallback closed_cb = settings_win->closed_cb;
    lv_mem_free(settings_win);
    settings_win = NULL;

    if (closed_cb)
    {
        closed_cb();
    }
}

static void save_error_close_cb(lv_obj_t *msgbox, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED)
    {
        return;
    }

    focus_pop_group();
    lv_obj_del(msgbox);

    lv_group_focus_obj(settings_win->close_btn);
}

static void save_defaults_cb(lv_obj_t *obj, lv_event_t event)
{
    if (event != LV_EVENT_CLICKED)
    {
        return;
    }

    bool ok = true;
    if (!storage_write(SETTINGS_MAGIC, SETTINGS_VERSION, settings_win->settings, sizeof(Settings)))
    {
        // Recompile with -DSTORAGE_DEBUG=1 for detailed info
        hal_printf("ERROR: Saving settings failed.\n");
        ok = false;
    }

    Settings temp_settings;
    if (!storage_read(SETTINGS_MAGIC, SETTINGS_VERSION, &temp_settings, sizeof(Settings)) ||
        memcmp(settings_win->settings, &temp_settings, sizeof(Settings)) != 0)
    {
        hal_printf("ERROR: Settings verify failed.\n");
        ok = false;
    }

    lv_group_t *group = focus_push_group();
    lv_obj_t *msgbox = lv_msgbox_create(lv_scr_act(), NULL);
    const char *msg =
        ok ? LV_SYMBOL_OK " Settings saved." : LV_SYMBOL_WARNING " There was an error saving your settings.";
    lv_msgbox_set_text(msgbox, msg);
    static const char *btn_str[] = {"Close", ""};
    lv_msgbox_add_btns(msgbox, btn_str);
    lv_obj_set_event_cb(msgbox, save_error_close_cb);
    lv_group_add_obj(group, msgbox);
}

static void activate_textarea(lv_obj_t *area, lv_obj_t *focused, bool editing)
{
    bool edit = focused == area && editing;
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

    // Scroll the whole row ('box') into view
    lv_obj_t *box = focused;
    lv_obj_t *scrollable = lv_page_get_scrollable(lv_win_get_content(settings_win->win));
    while (box)
    {
        lv_obj_t *parent = lv_obj_get_parent(box);
        if (!parent || parent == scrollable)
        {
            break;
        }
        box = parent;
    }
    lv_win_focus(settings_win->win, box, LV_ANIM_ON);

    bool editing = lv_group_get_editing(group);
    activate_textarea(settings_win->emissivity, focused, editing);
    activate_textarea(settings_win->reflected, focused, editing);
    activate_textarea(settings_win->max_temp, focused, editing);
    activate_textarea(settings_win->min_temp, focused, editing);
}

static void configure_focus_group()
{
    lv_group_t *group = settings_win->focus_group;
    lv_obj_t *focused = lv_group_get_focused(group);

    lv_group_remove_all_objs(group);
    lv_group_add_obj(group, settings_win->emissivity);
    lv_group_add_obj(group, settings_win->auto_ambient);
    if (!settings_win->settings->auto_ambient)
    {
        lv_group_add_obj(group, settings_win->reflected);
    }
    lv_group_add_obj(group, settings_win->auto_range);
    if (!settings_win->settings->auto_range)
    {
        lv_group_add_obj(group, settings_win->max_temp);
        lv_group_add_obj(group, settings_win->min_temp);
    }
    lv_group_add_obj(group, settings_win->flip_hor);
    lv_group_add_obj(group, settings_win->flip_ver);
    lv_group_add_obj(group, settings_win->close_btn);
    lv_group_add_obj(group, settings_win->save_btn);

    if (focused)
    {
        lv_group_focus_obj(focused);
    }
}

static void conditionally_enable_obj(lv_obj_t *target, bool enable)
{
    bool is_enabled = lv_obj_get_group(target) != NULL;
    if (is_enabled != enable)
    {
        lv_obj_set_state(target, enable ? LV_STATE_DEFAULT : LV_STATE_DISABLED);
        configure_focus_group();
    }
}

static void sync_settings(void)
{
    // Disable editors and remove from focus or vice-versa
    conditionally_enable_obj(settings_win->reflected, !settings_win->settings->auto_ambient);
    conditionally_enable_obj(settings_win->max_temp, !settings_win->settings->auto_range);
    conditionally_enable_obj(settings_win->min_temp, !settings_win->settings->auto_range);

    // Update values in (disabled) editors if needed
    if (settings_win->settings->auto_ambient)
    {
        lv_spinbox_set_value(settings_win->reflected, settings_win->settings->reflected_temperature * 10 + 0.5);
    }
    if (settings_win->settings->auto_range)
    {
        lv_spinbox_set_value(settings_win->max_temp, settings_win->settings->max_temp + 0.5);
        lv_spinbox_set_value(settings_win->min_temp, settings_win->settings->min_temp + 0.5);
    }
}

static void sync_settings_task(lv_task_t *task)
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
        else if (checkbox == settings_win->auto_range)
        {
            settings_win->settings->auto_range = checked;
        }
        sync_settings();
    }
}

static void spinbox_event_cb(lv_obj_t *spinbox, lv_event_t event)
{
    if (event == LV_EVENT_VALUE_CHANGED)
    {
        int32_t value = lv_spinbox_get_value(spinbox);
        if (spinbox == settings_win->emissivity)
        {
            settings_win->settings->emissivity = value / 100.0;
        }
        else if (spinbox == settings_win->reflected)
        {
            if (!settings_win->settings->auto_ambient)
            {
                settings_win->settings->reflected_temperature = value / 10.0;
            }
        }
        else if (spinbox == settings_win->max_temp)
        {
            if (!settings_win->settings->auto_range)
            {
                settings_win->settings->max_temp = value;
                // 'Move' the other spinbox value if we'd otherwise cross it.
                // Note: don't use setting value directly, because that's a float,
                // and causes rounding issues.
                if (lv_spinbox_get_value(settings_win->min_temp) >= value)
                {
                    lv_spinbox_set_value(settings_win->min_temp, value - 1);
                }
            }
        }
        else if (spinbox == settings_win->min_temp)
        {
            if (!settings_win->settings->auto_range)
            {
                settings_win->settings->min_temp = value;
                if (lv_spinbox_get_value(settings_win->max_temp) <= value)
                {
                    lv_spinbox_set_value(settings_win->max_temp, value + 1);
                }
            }
        }
    }
}

void settings_show(Settings *settings, SettingsClosedCallback closed_cb)
{
    lv_group_t *group = focus_push_group();

    LV_DEBUG_ASSERT(settings_win == NULL, "Settings window already initialized", 0);
    settings_win = lv_mem_alloc(sizeof(SettingsWindow));

    settings_win->settings = settings;
    settings_win->closed_cb = closed_cb;
    settings_win->focus_group = group;

    settings_win->win = lv_win_create(lv_scr_act(), NULL);
    lv_win_set_title(settings_win->win, "Settings");
    lv_win_set_header_height(settings_win->win, LV_DPX(40));
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
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width_margin(label, lv_obj_get_width_fit(row) - lv_obj_get_width_margin(spinbox) - padding);
    lv_label_set_text(label, "Emissivity");

    // Reflected temperature + auto_ambient check box
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
    lv_checkbox_set_text(checkbox, "Auto");
    lv_checkbox_set_checked(checkbox, settings_win->settings->auto_ambient);
    spinbox = lv_spinbox_create(cell, NULL);
    settings_win->reflected = spinbox;
    lv_spinbox_set_range(spinbox, -1000, +6000);
    lv_spinbox_set_digit_format(spinbox, 4, 3);
    lv_spinbox_set_value(spinbox, settings_win->settings->reflected_temperature * 10 + 0.5);
    lv_spinbox_step_prev(spinbox);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width_margin(label, lv_obj_get_width_fit(row) - lv_obj_get_width_margin(cell) - padding - pad_outer);
    lv_label_set_text(label, "Reflected temperature");
    lv_cont_set_layout(cell, LV_LAYOUT_COLUMN_RIGHT);

    // Auto-ranging
    row = lv_cont_create(settings_win->win, row);
    label = lv_label_create(row, label);
    cell = lv_cont_create(row, cell);

    checkbox = lv_checkbox_create(cell, NULL);
    settings_win->auto_range = checkbox;
    lv_checkbox_set_text(checkbox, "Auto");
    lv_checkbox_set_checked(checkbox, settings_win->settings->auto_range);

    lv_obj_t *spinbox_cell = lv_cont_create(cell, cell);
    lv_obj_clean_style_list(spinbox_cell, LV_CONT_PART_MAIN);
    lv_obj_set_style_local_pad_inner(spinbox_cell, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, padding);
    lv_obj_t *spinbox_label = lv_label_create(spinbox_cell, NULL);
    lv_label_set_text(spinbox_label, "high");
    spinbox = lv_spinbox_create(spinbox_cell, NULL);
    settings_win->max_temp = spinbox;
    lv_spinbox_set_range(spinbox, -99, +600);
    lv_spinbox_set_digit_format(spinbox, 3, 0);
    lv_spinbox_set_value(spinbox, settings_win->settings->max_temp + 0.5);
    lv_spinbox_step_prev(spinbox);
    lv_cont_set_layout(spinbox_cell, LV_LAYOUT_ROW_MID);

    spinbox_cell = lv_cont_create(cell, spinbox_cell);
    spinbox_label = lv_label_create(spinbox_cell, NULL);
    lv_label_set_text(spinbox_label, "low");
    spinbox = lv_spinbox_create(spinbox_cell, NULL);
    settings_win->min_temp = spinbox;
    lv_spinbox_set_range(spinbox, -100, +599);
    lv_spinbox_set_digit_format(spinbox, 3, 0);
    lv_spinbox_set_value(spinbox, settings_win->settings->min_temp + 0.5);
    lv_spinbox_step_prev(spinbox);
    lv_cont_set_layout(spinbox_cell, LV_LAYOUT_ROW_MID);

    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width_margin(label, lv_obj_get_width_fit(row) - lv_obj_get_width_margin(cell) - padding - pad_outer);
    lv_label_set_text(label, "Colorscale");
    lv_cont_set_layout(cell, LV_LAYOUT_COLUMN_RIGHT);

    // Flip horizontally
    row = lv_cont_create(settings_win->win, row);
    label = lv_label_create(row, label);
    checkbox = lv_checkbox_create(row, NULL);
    lv_obj_set_style_local_pad_inner(checkbox, LV_CHECKBOX_PART_BG, LV_STATE_DEFAULT, 0);
    settings_win->flip_hor = checkbox;
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
    lv_checkbox_set_text(checkbox, "");
    lv_checkbox_set_checked(checkbox, settings_win->settings->flip_ver);
    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
    lv_obj_set_width_margin(label, lv_obj_get_width_fit(row) - lv_obj_get_width_margin(checkbox) - padding);
    lv_label_set_text(label, "Flip vertically");

    // Close
    lv_obj_t *close_btn = lv_btn_create(settings_win->win, NULL);
    settings_win->close_btn = close_btn;
    lv_obj_set_event_cb(settings_win->close_btn, settings_close_cb);
    lv_obj_t *close_label = lv_label_create(close_btn, NULL);
    lv_label_set_text(close_label, "Close");

    // Save defaults
    lv_obj_t *save_btn = lv_btn_create(settings_win->win, NULL);
    settings_win->save_btn = save_btn;
    lv_obj_set_event_cb(settings_win->save_btn, save_defaults_cb);
    close_label = lv_label_create(save_btn, NULL);
    lv_label_set_text(close_label, "Save as defaults");
    lv_btn_set_fit2(save_btn, LV_FIT_TIGHT, LV_FIT_NONE);

    // Callbacks
    lv_obj_set_event_cb(settings_win->emissivity, spinbox_event_cb);
    lv_obj_set_event_cb(settings_win->reflected, spinbox_event_cb);
    lv_obj_set_event_cb(settings_win->auto_ambient, checkbox_event_cb);
    lv_obj_set_event_cb(settings_win->max_temp, spinbox_event_cb);
    lv_obj_set_event_cb(settings_win->min_temp, spinbox_event_cb);
    lv_obj_set_event_cb(settings_win->auto_range, checkbox_event_cb);
    lv_obj_set_event_cb(settings_win->flip_hor, checkbox_event_cb);
    lv_obj_set_event_cb(settings_win->flip_ver, checkbox_event_cb);

    configure_focus_group();
    lv_group_set_focus_cb(group, settings_focus_cb);
    settings_focus_cb(group);
    sync_settings();

    settings_win->sync_task = lv_task_create(sync_settings_task, 1000, LV_TASK_PRIO_LOW, NULL);
}
