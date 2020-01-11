#include <fastarduino/gpio.h>
#include <fastarduino/time.h>

int main()
{
    board::init();
    sei();

    gpio::FAST_PIN<board::DigitalPin::LED> led{gpio::PinMode::OUTPUT};
    while (true)
    {
        led.toggle();
        time::delay_ms(500);
    }
    return 0;
}
