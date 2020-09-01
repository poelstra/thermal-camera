#include "hal_time.h"

#include <SDL2/SDL.h>

void hal_sleep(unsigned long milliseconds)
{
    SDL_Delay(milliseconds);
}

unsigned long hal_millis()
{
    return SDL_GetTicks();
}
