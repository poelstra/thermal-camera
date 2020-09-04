#include "hal_init.h"

#include "keyboard.h"
#include "lcd.h"

void hal_init()
{
    lcd_init();
    keyboard_init();
}
