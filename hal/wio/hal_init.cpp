#include "hal_init.h"

#include "keyboard.h"
#include "lcd.h"

void hal_init(lv_group_t *keyboard_group)
{
    lcd_init();
    keyboard_init(keyboard_group);
}
