/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "MLX90641_I2C_Driver.h"

#include <Arduino.h>
#include <Wire.h>

int MLX90641_I2CGeneralReset(void)
{
    Wire.beginTransmission(0);
    Wire.write(0x06);
    if (Wire.endTransmission() != 0)
    {
        return -1;
    }
    return 0;
}

int MLX90641_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    while (nMemAddressRead > 0)
    {
        Wire.beginTransmission(slaveAddr);
        Wire.write(highByte(startAddress));
        Wire.write(lowByte(startAddress));
        if (Wire.endTransmission(false) != 0)
        {
            // NACK
            return -1;
        }

        Wire.requestFrom(slaveAddr, nMemAddressRead * 2);
        while (Wire.available() >= 2)
        {
            uint8_t msb = Wire.read();
            uint8_t lsb = Wire.read();
            *data = msb << 8 | lsb;
            data++;
            startAddress++;
            nMemAddressRead--;
        }
    }

    return 0;
}

void MLX90641_I2CFreqSet(int kHz)
{
    Wire.setClock(1000 * kHz);
}

int MLX90641_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    Wire.beginTransmission(slaveAddr);
    Wire.write(highByte(writeAddress));
    Wire.write(lowByte(writeAddress));
    Wire.write(highByte(data));
    Wire.write(lowByte(data));

    if (Wire.endTransmission() != 0)
    {
        // NACK
        return -1;
    }

    uint16_t dataCheck;
    MLX90641_I2CRead(slaveAddr, writeAddress, 1, &dataCheck);

    if (dataCheck != data)
    {
        return -2;
    }

    return 0;
}
