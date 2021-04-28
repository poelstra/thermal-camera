#ifndef HEADER_H
#define HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "settings.h"

void header_init();
void header_update(const Settings *settings);
void header_edit_emissivity(Settings *settings);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*HEADER_H*/
