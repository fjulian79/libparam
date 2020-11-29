# libparam

A template class to store user data in nonvolatile memory by using the Arduino EEPROM support.

**A small demo to demostrate how to use the class:**

```C++
#include <Arduino.h>
#include <param/param.h>

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
    
    if(param.read() != true)
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