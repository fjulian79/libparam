# libparam

A template class to store user data in nonvolatile memory by using the Arduino EEPROM support.

The data is a plain struct of your own. `Param` stores it at a start address in the EEPROM and appends a CRC byte to detect invalid or missing data. The library is made for the Arduino framework on ESP32 and Raspberry Pi Pico (Earle Philhower's core), where the EEPROM is emulated in flash.

## Installation

Add it to the `lib_deps` of your `platformio.ini`:

```ini
lib_deps =
    https://github.com/fjulian79/libparam.git
```

## Example

```C++
#include <Arduino.h>
#include <param/param.hpp>

typedef struct
{
    uint32_t a[4];
    uint8_t b;

}UserData_t;

Param<UserData_t> param;

void setup()
{
    Serial.begin(115200);
    while (!Serial);   
    Serial.println();
    
    if(param.begin() != true)
    {
        param.clear();
        param.write();
        Serial.printf("Parameter reset.\n");
    }
    else
    {
        Serial.printf("Parameter loaded.\n");
    }
    
    Serial.printf("Value of b is %d\n", param.data.b);
    param.data.b++;
    param.write();
}

void loop()
{

}
```

## API

| Function | Description |
|---|---|
| `Param<T>(addr = 0)` | Creates the instance, the data is stored at `addr`. |
| `begin()` | Initializes the EEPROM and reads the data, returns `false` in case of a CRC error. |
| `read()` | Reads the data, returns `false` in case of a CRC error. |
| `write()` | Writes the data and the CRC, returns `false` if writing to the flash failed. |
| `discard()` | Invalidates the stored data by inverting the CRC. |
| `clear()` | Sets the data in RAM to zero. |
| `size()` | Number of bytes used in the EEPROM, the data plus one CRC byte. |
| `data` | The user data. |

## Notes

- After a failed `begin()` or `read()` the data contains whatever has been in the EEPROM. Call `clear()` or set your defaults then, as in the example.
- Changing the struct usually invalidates the stored data, as the CRC does not match anymore.
- Each `begin()` sets the size of the emulated EEPROM to the end of its own data. When using more than one instance, call `begin()` of the instance with the highest end address last, the ESP32 truncates the stored data to a smaller size otherwise.
- Each `write()` writes to the flash, avoid calling it periodically without a change of the data.
- Define `PARAM_DEBUG` to print the data and the CRC when reading.
