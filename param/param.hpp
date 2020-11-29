/*
 * libparam, a generic template class to store, load and verify von volatile 
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
#include <generic/crc8.hpp>
#include <stdint.h>
#include <string.h>

/**
 * @brief A template to use with a struct to store parameter into non volatile
 * memory by using the Arduino EEPROM support.
 * 
 * Pseudo code example to demostrate how to use the class:
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
 *     param.read();
 *     param.data.b = 42;
 *     param.save();
 * 
 * @tparam T The definition of the users data structure.
 */
template <typename T> class Param
{
    public:

        /**
         * @brief Construct a new Param object
         * 
         * @param _addr The Start address in the EEPROM. Defaults to zero.
         */
        Param(uint8_t _addr = 0) :
          addr(_addr)
        {
            clear();
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
         * Performs also a CRC check. 
         * 
         * @return true     If the data has been read sucessfully. 
         * @return false    In case of a CRC error.
         */
        bool read(void)
        {
            crc8 crc;

            EEPROM.get(addr, data);
            
            if (crc.calc(&data, sizeof(data)) ==
                EEPROM.read(addr + sizeof(data)))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        /**
         * @brief Writes the user data from RAM to the EEPROM.
         * 
         * Postpones the user datta in the EEPROM with a CRC byte.
         */
        void write(void)
        {
            crc8 crc;

            EEPROM.put(addr, data);
            EEPROM.write(addr + sizeof(data), crc.calc(&data, sizeof(data)));
        }

        /**
         * @brief Discards the data in the EEPROM.
         * 
         * This is done by inverting the stored CRC. The next read attempt will
         * fail because of a invalid CRC.
         */
        void discard(void)
        {
            EEPROM.write(addr + sizeof(data),
                    ~EEPROM.read(addr + sizeof(data)));
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
         * @brief The user data, can be used by accassing this member.
         */
        T data;

    private:

        /**
         * @brief The Address in the EEPROM.
         */
        uint8_t addr;
};
