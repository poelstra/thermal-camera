#include "storage.h"

#include "hal_flash.h"
#include <string.h>

// Enable printf output while reading/writing to storage.
// Set to 1 to enable, 0 to disable.
#ifndef STORAGE_DEBUG
#define STORAGE_DEBUG 0
#endif

#if STORAGE_DEBUG
#include "hal_print.h"
#define STORAGE_PRINTF(fmt, ...) hal_printf((fmt), ##__VA_ARGS__)
#else
#define STORAGE_PRINTF(fmt, ...)
#endif

/**
 * Sector-aligned flash base address
 */
#define STORAGE_BASE_ADDRESS (0x00000000)

typedef struct StorageHeader
{
    uint32_t magic;
    uint32_t version;
    uint32_t checksum;
    size_t len;
} StorageHeader;

static uint32_t crc32(const uint8_t *data, size_t size);

bool storage_read(uint32_t magic, uint32_t version, void *data, size_t len)
{
    StorageHeader header;

    if (!hal_flash_read(STORAGE_BASE_ADDRESS, &header, sizeof(header)))
    {
        STORAGE_PRINTF("storage_read(): read header failed\n");
        return false;
    }

    STORAGE_PRINTF("storage_read(): magic=0x%08x version=0x%08x chk=0x%08x len=%u\n", header.magic, header.version,
                   header.checksum, header.len);

    if (header.magic != magic || header.version != version || header.len != len)
    {
        STORAGE_PRINTF("storage_read(): header mismatch\n");
        return false;
    }

    if (!hal_flash_read(STORAGE_BASE_ADDRESS + sizeof(header), data, len))
    {
        STORAGE_PRINTF("storage_read(): read data failed\n");
        return false;
    }

    uint32_t crc = crc32(data, len);
    STORAGE_PRINTF("storage_read(): crc=0x%08x\n", crc);
    if (header.checksum != crc)
    {
        STORAGE_PRINTF("storage_read(): crc mismatch\n");
        return false;
    }

    STORAGE_PRINTF("storage_read(): ok\n");
    return true;
}

bool storage_write(uint32_t magic, uint32_t version, const void *data, size_t len)
{
    StorageHeader header;
    uint8_t buffer[sizeof(header) + len];

    header.magic = magic;
    header.version = version;
    header.checksum = crc32(data, len);
    header.len = len;

    STORAGE_PRINTF("storage_write(): magic=0x%08x version=0x%08x chk=0x%08x len=%u\n", header.magic, header.version,
                   header.checksum, header.len);

    memmove(buffer, &header, sizeof(header));
    memmove(buffer + sizeof(header), data, len);

    bool ok = hal_flash_write(STORAGE_BASE_ADDRESS, buffer, sizeof(buffer));

    if (!ok)
    {
        STORAGE_PRINTF("storage_write(): write failed\n");
    }
    else
    {
        STORAGE_PRINTF("storage_write(): ok\n");
    }
    return ok;
}

static uint32_t crc32(const uint8_t *data, size_t size)
{
    uint32_t r = ~0;
    const uint8_t *end = data + size;

    while (data < end)
    {
        r ^= *data++;

        for (int i = 0; i < 8; i++)
        {
            uint32_t t = ~((r & 1) - 1);
            r = (r >> 1) ^ (0xEDB88320 & t);
        }
    }

    return ~r;
}
