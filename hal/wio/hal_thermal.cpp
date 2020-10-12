#include <Arduino.h>
#include <MLX90641_API.h>
#include <MLX90641_I2C_Driver.h>
#include <Wire.h>

#include "hal_thermal.h"

const uint8_t MLX90641_address = 0x33; // Default 7-bit unshifted address of the MLX90641
#define TA_SHIFT 5                     // Default shift for MLX90641 in open air

typedef enum
{
    MLX90641_Refresh_0_5Hz = 0,
    MLX90641_Refresh_1Hz = 1,
    MLX90641_Refresh_2Hz = 2,
    MLX90641_Refresh_4Hz = 3,
    MLX90641_Refresh_8Hz = 4,
    MLX90641_Refresh_16Hz = 5,
    MLX90641_Refresh_32Hz = 6,
    MLX90641_Refresh_64Hz = 7,
} MLX90641Refresh;

uint16_t MLX90641Frame[242];
paramsMLX90641 MLX90641;

static bool is_connected()
{
    Wire.beginTransmission(MLX90641_address);
    return (Wire.endTransmission() == 0); // Sensor ACK'ed
}

static bool data_ready()
{
    uint16_t statusRegister;
    int error = MLX90641_I2CRead(MLX90641_address, 0x8000, 1, &statusRegister);
    if (error != 0)
    {
        return false;
    }
    return (statusRegister & 0x0008) > 0;
}

bool hal_thermal_init()
{
    Wire.begin();
    Wire.setClock(1000000); // 1 MHz
    MLX90641_I2CGeneralReset();

    delay(80); // according to MLX datasheet
    if (!is_connected())
    {
        Serial.print("Cannot find MLX90641 at address 0x");
        Serial.println(MLX90641_address, HEX);
        return false;
    }

    int status;

    uint16_t eeMLX90641[832];
    status = MLX90641_DumpEE(MLX90641_address, eeMLX90641);
    if (status != 0)
    {
        Serial.println("Failed to load EEPROM parameters");
        return false;
    }

    status = MLX90641_ExtractParameters(eeMLX90641, &MLX90641);
    if (status != 0)
    {
        Serial.println("Parameter extraction failed");
        return false;
    }

    MLX90641_SetRefreshRate(MLX90641_address, MLX90641_Refresh_8Hz);
    return true;
}

bool hal_thermal_tick(float *pixels, float emissivity, bool auto_tr, float *tr)
{
    static bool havePage[] = {false, false};
    static float Ta = 30.0;

    if (!data_ready())
    {
        return false;
    }

    int subPage = MLX90641_GetFrameData(MLX90641_address, MLX90641Frame);
    if (subPage < 0)
    {
        // Error
        return false;
    }

    havePage[subPage] = true;
    if (!havePage[0] || !havePage[1])
    {
        // Need to have seen both subpages before we can calculate anything
        return false;
    }

    // Determine reflected temperature to use: use built-in ambient
    // temperature sensor or user-defined temperature.
    float trToUse;
    if (auto_tr || tr == NULL)
    {
        Ta = MLX90641_GetTa(MLX90641Frame, &MLX90641);
        trToUse = Ta - TA_SHIFT;
        if (tr != NULL)
        {
            *tr = trToUse;
        }
    }
    else
    {
        trToUse = *tr;
    }

    MLX90641_CalculateTo(MLX90641Frame, &MLX90641, emissivity, trToUse, pixels);
    return true;
}
