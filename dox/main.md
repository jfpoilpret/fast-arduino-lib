FastArduino Documentation	{#index}
=========================

Welcome to FastArduino documentation!

FastArduino is a C++ object-oriented library for Arduino boards based on AVR MCU and also bare AVR MCU. FastArduino:

- provides smaller and faster code than other libraries for the same functionality
- ensures you pay (size and speed) only for what you use
- uses Object-Oriented Design
- reduces risk of bad code by performing extensive compile-time checks
- supports event-driven programs
- supports both ATmega and ATtiny chips

FastArduino implementation:
- is based on C++11 standard
- favours C++ templates rather than virtual methods whenever possible
- virtual methods are used only when needed (mostly for event handlers)
- never automatically registers any ISR but requires explicit registration through provided macros

The rest of this guide provides indications where to start and proceed with learning how to use FastArduino for your projects.

Building a project with FastArduino
-----------------------------------
TODO

Discovering FastArduino API step by step
----------------------------------------

Using Fast Arduino API can be learnt step by step in the preferred following order:

Basics:
1. [gpio & time](@ref gpiotime)
2. UART & flash
3. timer
4. real-time timer
5. analog input
6. PWM
7. utilities

Advanced:
1. watchdog
2. interrupts
3. events, scheduler
4. power
5. SPI devices
6. I2C devices
7. eeprom

Devices:
1. SPI
2. I2C

### @anchor gpiotime Basics: gpio & time ###

Here is a first example of a FastArduino based program:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>

int main()
{
    board::init();
    sei();

    gpio::FastPinType<board::DigitalPin::LED>::TYPE led{gpio::PinMode::OUTPUT};
    while (true)
    {
        led.toggle();
        time::delay_ms(500);
    }
    return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example can be broken down into several parts:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/gpio.h>
#include <fastarduino/time.h>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This includes the necessary API from FastArduino; in this example, we just use `gpio.h` (all API for digital input/output) and `time.h` (API for busy loop delays).

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
int main()
{
    board::init();
    sei();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The next part defines the standard `main()` function as the entry point of the program; first actions in the `main()` should always be to call `board::init()` (important initialization for some specific boards), then sooner or later call `sei()` to enable interrupts again, as interrupts are disabled when `main()` is initially called.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    gpio::FastPinType<board::DigitalPin::LED>::TYPE led{gpio::PinMode::OUTPUT};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This line declares and initializes a digital pin variable named `led` as output for the board's LED (i.e. `D13` on Arduino boards).

`board::DigitalPin` is a strong enum class that defines all digital pins **for the current target**, that target must be defined in the compiler command-line.

The actual type of `led` is `gpio::FastPin<board::Port:PORT_B, 5>` which means "the pin number 5 within port B"; since this type is not easy to declare when you only know the number of the pin you need, `gpio::FastPinType<board::DigitalPin::LED>::TYPE` is used instead, as it maps directly to the right type.

`led` is initialized as an output pin, its initial level is `false` (i.e. GND) by default.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    while (true)
    {
        led.toggle();
        time::delay_ms(500);
    }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Then the program enters an endless loop in which, at every iteration:

1. It toggles the level of `led` pin (D13 on Arduino) from GND to Vcc or from Vcc to GND
2. It delays execution (i.e. it "waits") for 500 milliseconds

This part of the program simply makes your Arduino LED blink at 1Hz frequency!

The last part below is never executed (because of the endless loop above) but is necessary to make the compiler happy, as `main()` shall return a value:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Congratulation! We have just studied the typical "blink" program.

At this point, it is interesting to compare our first program with the equivalent with standard Arduino API:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Granted that the latter code seems simpler to write! However, it is also simpler to write it wrong, build and upload it to Arduino and only see it not working:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#define LED 53
void setup()
{
    pinMode(LED, OUTPUT);
}

void loop()
{
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The problem here is that Arduino API accept a simple number when they need a pin, hence it is perfectly possible to pass them the number of a pin that does not exist, as in the faulty code above: this code will compile and upload properly to an Arduino UNO, however it will not work, because pin 53 does not exist!

This problem cannot occur with FastArduino as the available pins are stored in a strong enum and it becomes impossible to select a pin that does not exist for the board we target!

Now, what is really interesting in comparing both working code examples is the size of the built program (measured with UNO as a target, TODO IDE/toolchains versions):

|           | Arduino API | FastArduino |
|-----------|-------------|-------------|
| code size | 928 bytes   | 154 bytes   |
| data size | 9 bytes     | 0 byte      |

As you probably know, Atmel AVR MCU (and Arduino boards that embed them) are much constrained in code and data size, hence we could say that "every byte counts". In the table ablove, one easily sees that Arduino API comes cluttered with lots of code and data, even if you don't need it; on the other hand, FastArduino is highly optimized and will produce code only for what you **do** use.

Now `gpio.h` has more API than just `gpio::FastPin` and `gpio::FastPinType`; it also includes `gpio::FastPort` and `gpio::FastMaskedPort` that allow to manipulate several pins at a time, as long as these pis belong to the same Port of the MCU. This allows size and speed optimizations when having to deal with a group of related pins, e.g. if you want to implement a LED chasing project.

With FastArduino, here is a program showing how you could implement a simple 8 LED chaser on UNO:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In this example, we selected all pins of the same port to connect the 8 LEDs of our chaser. Concretely on UNO, this is port D, which pins are D0-D7.

We thus declare and initialize `leds` as a `gpio::FastPort<board::Port::PORT_D>` port, with all pins as output (`0xFF`), with initial level to GND (`0x00`, all LEDs off).

Then, we will keep track of the current lit LED through `pattern` byte which each bit represents actually one LED; `pattern` is initialized with `0x01` i.e. D0 should be the first LED to be ON.

In the endless loop that follows, we perform the following actions:

1. Set all pins values at once to the current value of `pattern`
2. Delay execution for 250ms
3. Shift the only 1 bit of `pattern` left; note that after 8 shifts, `pattern` will become `0`, hence we need to check against this condition to reset `pattern` to its initial state.

This should be rather straightforward to understand if you know C or C++.

Here is an equivalent example with Arduino API:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
TODO LED chasing with Arduino IDE.
https://learn.sparkfun.com/tutorials/sik-experiment-guide-for-arduino---v32/experiment-4-driving-multiple-leds
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We see, with Arduino API, that we have to deal with each pin individually, which makes the program source code longer and not necessarily easier to understand.

Here is a quick comparison of the sizes for both programs:
TODO
|           | Arduino API | FastArduino |
|-----------|-------------|-------------|
| code size | XXX bytes   | XXX bytes   |
| data size | X bytes     | X byte      |




