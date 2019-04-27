const byte LED_PINS[] = {0, 1, 2, 3, 4, 5, 6, 7};
const byte NUM_LEDS =  sizeof(LED_PINS) / sizeof(LED_PINS[0]);

void setup()
{
    for(byte i = 0; i < NUM_LEDS; i++)
        pinMode(LED_PINS[i], OUTPUT);
}

void loop()
{
    for(byte i = 0; i < NUM_LEDS; i++)
    {
        digitalWrite(LED_PINS[i], HIGH);
        delay(250);
        digitalWrite(LED_PINS[i], LOW);
    }
}
