#include "hal_flash.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE (4096)
#define FLASH_MEM_SIZE (4 * 1024 * 1024)

static uint8_t *flash_mem;

bool hal_flash_init()
{
    flash_mem = malloc(FLASH_MEM_SIZE);
    if (flash_mem == NULL)
    {
        return false;
    }
    memset(flash_mem, UCHAR_MAX, FLASH_MEM_SIZE);
    return true;
}

bool hal_flash_write(size_t address, const void *data, size_t len)
{
    if (flash_mem == NULL)
    {
        return false;
    }

    if (address % SECTOR_SIZE != 0)
    {
        // Unaligned write
        return false;
    }

    if (address + len > FLASH_MEM_SIZE)
    {
        return false;
    }

    size_t start_sector = address / SECTOR_SIZE;
    size_t end_sectors = start_sector + (len + SECTOR_SIZE - 1) / SECTOR_SIZE;
    for (size_t sector = start_sector; sector < end_sectors; sector++)
    {
        memset(&flash_mem[sector * SECTOR_SIZE], UCHAR_MAX, SECTOR_SIZE);
    }

    memmove(&flash_mem[address], data, len);
    return true;
}

bool hal_flash_read(size_t address, void *data, size_t len)
{
    if (flash_mem == NULL)
    {
        return false;
    }

    if (address + len > FLASH_MEM_SIZE)
    {
        return false;
    }

    memmove(data, &flash_mem[address], len);
    return true;
}
