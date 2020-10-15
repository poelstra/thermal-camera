#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Initialize flash driver.
 *
 * @returns true when init successful, false otherwise.
 */
bool hal_flash_init(void);

/**
 * Write `len` bytes of `data` to flash address starting at `address`.
 * Data is always written in multiples of the sector size, and all of
 * the affected sectors will be erased before writing.
 *
 * @param address Starting address of flash memory, must be sector-aligned (4096 bytes).
 * @param data Pointer to data buffer.
 * @param len Size of data buffer to write.
 * @returns true when data was successfully written, false otherwise.
 */
bool hal_flash_write(size_t address, const void *data, size_t len);

/**
 * Read `len` bytes from flash address starting at `address` into `data`.
 * @param address Starting address of flash memory, does not have to be aligned.
 * @param data Pointer to data buffer.
 * @param len Number of bytes to read, data buffer must be at least this big.
 * @returns true when data was successfully read, false otherwise.
 */
bool hal_flash_read(size_t address, void *data, size_t len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*HAL_FLASH_H*/
