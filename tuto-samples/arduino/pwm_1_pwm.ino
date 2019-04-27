#define LED 6

void setup() {
}

void loop() {
    for (int duty = 0; duty < 255; ++duty)
    {
        analogWrite(LED, duty);
        delay(50);
    }
    for (int duty = 255; duty > 0; --duty)
    {
        analogWrite(LED, duty);
        delay(50);
    }
}
