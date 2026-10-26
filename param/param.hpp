/*
 * libparam, a generic template class to store, load and verify non volatile
 * parameters used in arduino projects.
 *
 * Copyright (C) 2020 Julian Friedrich
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>. 
 *
 * You can file issues at https://github.com/fjulian79/libparam/issues
 */
#include <Arduino.h>
#include <EEPROM.h>
#include <stdint.h>
#include <string.h>

/**
 * Uncomment this line to enable debug output.
 */
// #define PARAM_DEBUG

#if defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
/**
 * The EEPROM emulation of these platforms depends on calling begin(size)
 * before it becomes functional and commit() to write to the flash. Other
 * platforms don't even provide this api at all.
 */
#define PARAM_EEPROM_EMULATED
#endif

/**
 * @brief A template to use with a struct to store parameter into non volatile
 * memory by using the Arduino EEPROM support.
 * 
 * Pseudo code example to demonstrate how to use the class:
 *
 *     typedef struct
 *     {
 *         uint32_t a[4];
 *         uint8_t b;
 *
 *     }UserData_t;
 *
 *     Param<UserData_t> param;
 *
 *     if (!param.begin())
 *         param.clear();
 *     param.data.b = 42;
 *     param.write();
 *
 * The data is stored at the given start address followed by a CRC byte, see
 * size(). On ESP32 and Raspberry Pi Pico each call of begin() sets the size of
 * the emulated EEPROM to the end of its own data. When using more than one
 * instance, call begin() of the instance with the highest end address last,
 * the ESP32 truncates the stored data to a smaller size otherwise.
 *
 * @tparam T The definition of the users data structure.
 */
template <typename T> class Param
{
    public:

        /**
         * @brief Construct a new Param object
         * 
         * @param _addr The start address in the EEPROM. Defaults to zero.
         */
        Param(size_t _addr = 0) :
          addr(_addr)
        {
            clear();
        }

        /**
         * @brief Initializes the EEPROM if needed and reads the data.
         *
         * @return true     If the data has been read successfully.
         * @return false    In case of a CRC error, see read().
         */
        bool begin(void)
        {
            #ifdef PARAM_EEPROM_EMULATED
            EEPROM.begin(addr + size());
            #endif

            clear();
            return read();
        }

        /**
         * @brief Sets all the user data in RAM to zero. 
         * 
         * Does not have any effect on EEPROM data and the CRC.
         */
        void clear(void)
        {
            memset(&data, 0, sizeof(T));
        }

        /**
         * @brief Reads the data from the EEPROM starting at the configured 
         * EEPROM address.
         * 
         * Performs also a CRC check. In case of a CRC error the data contains
         * whatever has been in the EEPROM, call clear() or set defaults then.
         *
         * @return true     If the data has been read successfully.
         * @return false    In case of a CRC error.
         */
        bool read(void)
        {
            uint8_t crc = 0;
            uint8_t val = 0;

            EEPROM.get(addr, data);
            val = EEPROM.read(addr + sizeof(data));
            crc = calcCrc(&data, sizeof(data));

            #ifdef PARAM_DEBUG
            Serial.printf("eepom data: ");
            for (size_t i = 0; i < sizeof(T); i++)
            {
                Serial.printf("%02x", ((uint8_t*) &data)[i]);
            }
            Serial.printf("\n");

            Serial.printf("eeprom crc: 0x%02x\n", val);
            Serial.printf("expect crc: 0x%02x\n", crc);
            #endif

            return crc == val;
        }

        /**
         * @brief Writes the user data from RAM to the EEPROM.
         * 
         * Appends a CRC byte to the user data in the EEPROM.
         *
         * @return true     If the data has been written successfully.
         * @return false    If writing to the flash failed.
         */
        bool write(void)
        {
            EEPROM.put(addr, data);
            EEPROM.write(addr + sizeof(data), calcCrc(&data, sizeof(data)));

            #ifdef PARAM_EEPROM_EMULATED
            return EEPROM.commit();
            #else
            return true;
            #endif
        }

        /**
         * @brief Discards the data in the EEPROM.
         * 
         * This is done by inverting the stored CRC. The next read attempt will
         * fail because of an invalid CRC.
         *
         * @return true     If the data has been discarded successfully.
         * @return false    If writing to the flash failed.
         */
        bool discard(void)
        {
            EEPROM.write(addr + sizeof(data),
                    ~EEPROM.read(addr + sizeof(data)));

            #ifdef PARAM_EEPROM_EMULATED
            return EEPROM.commit();
            #else
            return true;
            #endif
        }

        /**
         * @brief Returns the real size of the data in the EEPROM including CRC.
         * 
         * @return size_t Number of bytes used by the instance in the EEPROM.
         */
        size_t size(void)
        {
            return sizeof(T) + sizeof(uint8_t);
        }

        /**
         * @brief The user data, can be used by accessing this member.
         */
        T data;

    private:

        /**
         * @brief Calculates the CRC of the given data.
         *
         * CRC-8/SMBUS, polynom 0x07, init 0x00, not reflected. Must not be
         * changed, otherwise data stored by previous versions becomes invalid.
         *
         * @param pData     Pointer to the data.
         * @param siz       Number of bytes.
         * @return uint8_t  The CRC.
         */
        static uint8_t calcCrc(const void *pData, size_t siz)
        {
            const uint8_t *p = (const uint8_t *) pData;
            uint8_t crc = 0;

            while (siz--)
            {
                crc ^= *p++;
                for (uint8_t i = 0; i < 8; i++)
                {
                    crc = (crc & 0x80) ? (uint8_t) ((crc << 1) ^ 0x07) : (uint8_t) (crc << 1);
                }
            }

            return crc;
        }

        /**
         * @brief The start address in the EEPROM.
         */
        size_t addr;
};
