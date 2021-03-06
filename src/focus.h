#ifndef FOCUS_H
#define FOCUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>

/**
 * Create new focus group and make it current.
 *
 * @returns the newly created group.
 */
lv_group_t *focus_push_group();

/**
 * Restore previous focus group.
 *
 * @returns the group that will now have focus.
 */
lv_group_t *focus_pop_group();

/**
 * Get currently active focus group.
 */
lv_group_t *focus_get_current_group();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*FOCUS_H*/
