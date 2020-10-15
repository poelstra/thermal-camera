#include "hal_flash.h"

#include <Adafruit_SPIFlashBase.h>

static Adafruit_FlashTransport_QSPI flashTransport;
static Adafruit_SPIFlashBase flash(&flashTransport);
static SPIFlash_Device_t chip = W25Q32FV;

bool hal_flash_init()
{
    return flash.begin(&chip, 1);
}

bool hal_flash_write(size_t address, const void *data, size_t len)
{
    if (address % SFLASH_SECTOR_SIZE != 0)
    {
        // Invalid address alignment
        return false;
    }
    size_t start_sector = address / SFLASH_SECTOR_SIZE;
    size_t end_sectors = start_sector + (len + SFLASH_SECTOR_SIZE - 1) / SFLASH_SECTOR_SIZE;
    for (size_t sector = start_sector; sector < end_sectors; sector++)
    {
        flash.eraseSector(sector);
    }

    size_t written = flash.writeBuffer(address, (uint8_t *)data, len);
    return len == written;
}

bool hal_flash_read(size_t address, void *data, size_t len)
{
    size_t read = flash.readBuffer(address, (uint8_t *)data, len);
    return len == read;
}
