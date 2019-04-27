#include <fastarduino/flash.h>

// This is the type of data we want to store in flash
struct Dummy
{
    uint16_t a;
    uint8_t b;
    bool c;
    int16_t d;
    char e;
};

// Define 2 variables of that type, which will be stored in flash
// Note the PROGMEM keyword that says the compiler and linker to put this data to flash
const Dummy sample1 PROGMEM = {54321, 123, true, -22222, 'z'};
const Dummy sample2 PROGMEM = {12345, 231, false, -11111, 'A'};

// The following function needs value of sample1 to be read from flash
void read_and_use_sample1()
{
    // value will get copied with sample1 read-only content
    Dummy value;
    // request reading sample1 from flash into local variable value
    flash::read_flash(&sample1, value);
    // Here we can use value which is {54321, 123, true, -22222, 'z'}

}
