#include <fastarduino/gpio.h>
#include <fastarduino/time.h>

int main()
{
    board::init();
    sei();

    gpio::FastPort<board::Port::PORT_D> leds{0xFF, 0x00};
    uint8_t pattern = 0x01;
    while (true)
    {
        leds.set_PORT(pattern);
        time::delay_ms(250);
        pattern <<= 1;
        if (!pattern) pattern = 0x01;
    }
    return 0;
}
