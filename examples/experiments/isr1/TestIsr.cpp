/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

// Utilities to handle PROGMEM storage
template<typename T> class FlashStorage;
#define F_(type, ptr) (__extension__({static const type __fs[] PROGMEM = (ptr); (const FlashStorage< type >*) &__fs[0];}))
//#define F(ptr) (__extension__({static const char __c[] PROGMEM = (ptr); (const FlashStorage*) &__c[0];}))
#define F(ptr) F_(char, ptr)

using CALLBACK = void (*)(char);

static void f(const FlashStorage<char>* flash, CALLBACK cb)
{
	uint16_t address = (uint16_t) flash;
	while (char value = pgm_read_byte(address++)) cb(value);
}

static void callback(char c)
{
	PORTB = c;
}

int main() __attribute__((OS_main));
int main()
{
	f(F("abcdefghijklmnopqrstuvwxyz"), callback);
}
