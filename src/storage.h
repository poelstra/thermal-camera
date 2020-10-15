#ifndef STORAGE_H
#define STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool storage_read(uint32_t magic, uint32_t version, void *data, size_t len);
bool storage_write(uint32_t magic, uint32_t version, const void *data, size_t len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*STORAGE_H*/
