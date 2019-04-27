#include <fastarduino/analog_input.h>
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>

const uint16_t THRESHOLD = 500;

int main()
{
    board::init();
    sei();

    gpio::FastPinType<board::DigitalPin::LED>::TYPE led{gpio::PinMode::OUTPUT};
    analog::AnalogInput<board::AnalogPin::A0> sensor;
    while (true)
    {
        if (sensor.sample() > THRESHOLD)
            led.set();
        else
            led.clear();
        time::delay_ms(100);
    }
    return 0;
}
