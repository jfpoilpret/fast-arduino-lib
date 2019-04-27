void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
}

const uint16_t THRESHOLD = 500;

void loop()
{
    if (analogRead(A0) > THRESHOLD)
        digitalWrite(LED_BUILTIN, HIGH);
    else
        digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}
