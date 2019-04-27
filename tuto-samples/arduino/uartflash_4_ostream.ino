void setup()
{
    Serial.begin(115200);
    unsigned int value = 0x8000;
    Serial.print(F("value = 0x"));
    Serial.print(value, 16);
    Serial.print(F(", "));
    Serial.print(value);
    Serial.print(F(", 0"));
    Serial.print(value, 8);
    Serial.print(F(", B"));
    Serial.println(value, 2);
}

void loop()
{
}
