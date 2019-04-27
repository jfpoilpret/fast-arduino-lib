#include <avr/sleep.h>

void onPinChange()
{
    digitalWrite(LED_BUILTIN, !digitalRead(2));
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(2), onPinChange, CHANGE);
}

void loop()
{
    sleep_enable();
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_cpu();
}
