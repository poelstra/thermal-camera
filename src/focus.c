#include "focus.h"

#include "hal_print.h"

typedef struct FocusGroup FocusGroup;
struct FocusGroup
{
    lv_group_t *group;
    FocusGroup *next;
};

static FocusGroup *groups = NULL;

void set_focus_group(lv_group_t *group)
{
    lv_indev_t *indev = lv_indev_get_next(NULL);
    while (indev)
    {
        lv_indev_set_group(indev, group);
        indev = lv_indev_get_next(indev);
    }
}

lv_group_t *focus_push_group()
{
    FocusGroup *item = lv_mem_alloc(sizeof(FocusGroup));
    item->group = lv_group_create();
    item->next = groups;
    groups = item;
    set_focus_group(item->group);
    return item->group;
}

lv_group_t *focus_pop_group()
{
    if (groups->next == NULL)
    {
        hal_printf("ERROR: cannot pop focus group, empty\n");
        return NULL;
    }

    FocusGroup *old = groups;
    groups = groups->next;
    lv_group_del(old->group);
    lv_mem_free(old);
    set_focus_group(groups->group);
    return groups->group;
}

lv_group_t *focus_get_current_group()
{
    return groups->group;
}