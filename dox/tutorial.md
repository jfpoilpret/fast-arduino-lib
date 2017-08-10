FastArduino API Tutorial	{#tutorial}
========================

This is FastArduino API step-by-step tutorial.

Only the API is covered here: creating and building a project is not described here, you are supposed to know how to do it already.

Using FastArduino API can be learnt step by step in the preferred following order:

Basics:
1. [gpio & time](@ref gpiotime)
2. [UART & flash](@ref uartflash)
3. [analog input](@ref analoginput)
4. [timer](@ref timer)
5. real-time timer
6. PWM
7. utilities

Advanced:
1. watchdog
2. interrupts
3. events, scheduler
4. power
5. SPI devices management
6. I2C devices management
7. eeprom
8. Software UART

Devices:
1. SPI
2. I2C
3. Other devices: sonar, servo, SIPO


@anchor gpiotime Basics: gpio & time
------------------------------------

### Blink example ###

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

Now, what is really interesting in comparing both working code examples is the size of the built program (measured with UNO as a target, FastArduino project built with AVR Toolchain 3.5.3, Arduino API project built with Arduino IDE 1.8.2):
|           | Arduino API | FastArduino |
|-----------|-------------|-------------|
| code size | 928 bytes   | 154 bytes   |
| data size | 9 bytes     | 0 byte      |

As you probably know, Atmel AVR MCU (and Arduino boards that embed them) are much constrained in code and data size, hence we could say that "every byte counts". In the table ablove, one easily sees that Arduino API comes cluttered with lots of code and data, even if you don't need it; on the other hand, FastArduino is highly optimized and will produce code only for what you **do** use.

### LED Chaser example ###

Now `gpio.h` has more API than just `gpio::FastPin` and `gpio::FastPinType`; it also includes `gpio::FastPort` and `gpio::FastMaskedPort` that allow to manipulate several pins at a time, as long as these pis belong to the same Port of the MCU. This allows size and speed optimizations when having to deal with a group of related pins, e.g. if you want to implement a LED chaser project.

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
We see, with Arduino API, that we have to deal with each pin individually, which makes the program source code longer and not necessarily easier to understand.

Here is a quick comparison of the sizes for both programs:
|           | Arduino API | FastArduino |
|-----------|-------------|-------------|
| code size | 968 bytes   | 168 bytes   |
| data size | 17 bytes    | 0 byte      |


@anchor uartflash Basics: UART & flash
--------------------------------------

### Simple Serial Output example ###

Although not often necessary in many finished programs, `UART` (for serial communication interface) is often very useful for debugging a program while it is being developed; this is why `UART` is presented now.

Here is a first simple program showing how to display, with FastArduino API, a simple string to the serial output (for UNO, this is connected to USB):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/uart.h>

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

REGISTER_UATX_ISR(0)

int main()
{
    board::init();
    sei();
	
    serial::hard::UATX<board::USART::USART0> uart{output_buffer};
    uart.register_handler();
    uart.begin(115200);

    streams::OutputBuffer out = uart.out();
    out.puts("Hello, World!\n");
    out.flush();
    return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
As usual, at first we need to include the proper header (`uart.h`) to use its API.

Then, we define a buffer that will be used by the `UART` API to transmit characters to your PC through USB. You may find it cumbersome to do it yourself but it brings a huge advantage: you are the one to decide of the buffer size, whereas in Arduino API, you have no choice. Here, we consider 64 bits to be big enough to store characters that will be transmitted to the PC. How `UART` is using this buffer is not important to you though.

Then we *register an ISR* necessary for transmissions to take place; this is done by the `REGISTER_UATX_ISR(0)` macro. Explicit ISR registration is one important design choice of FastArduino: **you** decide which ISR should be registered to do what. This may again seem cumbersome but once again this gives you the benefit to decie what you need, hence build your application the way you want it.

The code that follows instantiates a `uart::hard::UATX` object that is using `board::USART::USART0` (the only one available on UNO) and based on the previously created buffer. Note that `UATX` class is in charge of **only** transmitting characters, not receiving. Other classes exist for only receiving (`UARX`), or for doing both (`UART`).

Once created, `uart` needs to be *linked* to the ISR previously registered, this is done through `uart.register_handler()`. Then we can set `uart` ready for transmission, at serial speed of 115200 bps.

Next step consists in extracting, from `uart`, a `streams::OutputBuffer` that will allow us to send characters or strings to USB:

    out.puts("Hello, World!\n");

The last important instruction waits for all characters to be transmitted before leaving the program.

Here is the equivalent code with Arduino API:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
void setup()
{
  Serial.begin(115200);
  Serial.println("Hello, World!");
}

void loop()
{
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Of course, we can see here that the code looks simpler, although one may wonder why we need to define a `loop()` function that does nothing.

Now let's compare the size of both:
|           | Arduino API | FastArduino |
|-----------|-------------|-------------|
| code size | 1440 bytes  | 768 bytes   |
| data size | 200 bytes   | 93 bytes    |
The data size is big because the buffer used by `Serial` has a hard-coded size (you cannot change it without modifying and recompiling Arduino API). Moreover, when using `Serial`, 2 buffers are created, one for input and one for output, even though you may only need the latter one!

Now let's take a look at the 93 bytes of data used in the FastArduino version of this example, how are they broken down?
| Source            | data size   |
|-------------------|-------------|
| `output_buffer`   | 64 bytes    |
| power API         | 1 byte      |
| `UATX` ISR        | 2 bytes     |
| `UATX` vtable     | 10 bytes    |
| "Hello, World!\n" | 16 bytes    |
| **TOTAL**         | 93 bytes    |

*vtable* is specific data created by C++ compiler for classes with `virtual` methods: every time you use virtual methods in classes, this will add more data size, this is why FastArduino avoids `virtual` as much as possible.

As you can see in the table above, the constant string `"Hello, World!\n"` occupies 16 bytes of data (i.e. AVR SRAM) in addition to 16 bytes of Flash (as it is part of the program and must eb stored permanently). If your program deals with a lot of constant strings like this, you may quickly meet a memory problem with SRAM usage. This is why it is more effective to keep these strings exclusively in Flash (you have no choice) but load them t SRAM only when they are needed, i.e. when they get printed to `UATX` as in the sample code.

How do we change our program so that this string is only stored in Flash? We can use FastArduino `flash` API for that, by changing only one line of code:

    out.puts(F("Hello, World!\n"));

Note the use of `F()` macro here: this makes the string reside in Flash only, and then it is being read from Flash "on the fly" by `out.puts()` method; the latter method is overloaded for usual C-strings (initial example) and for C-strings stored in Flash only.

We can compare the impact on sizes:
|           | without %F() | with %F()   |
|-----------|--------------|-------------|
| code size | 768 bytes    | 776 bytes   |
| data size | 93 bytes     | 77 bytes    |
Although a bit more code has been added (the code to read the string from Flash into SRAM on the fly), we see 16 bytes have been removed from data, this is the size of the string constant.

You may wonder why `"Hello, World!\n"` occupies 16 bytes, although it should use only 15 bytes (if we account for the terminating `'\0'` character); this is because the string is stored in Flash and Flash is word-addressable, not byte-addressable on AVR.

Note that Flash can also be used to store other read-only data that you may want to access at runtime at specific times, i.e. data you do not want to be stored permanently on SRAM during all execution of your program.

The following example shows how to:
- define, in your source code, read-only data that shall be stored in Flash memory
- read that data when you need it
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/flash.h>

// This is the type of data we want to store in flash
struct Dummy
{
    uint16_t a;
    uint8_t b;
    bool c;
    int16_t d;
    char e;
};

// Define 2 variables of that type, which will be stored in flash
// Note the PROGMEM keyword that says the compiler and linker to put this data to flash
const Dummy sample1 PROGMEM = {54321, 123, true, -22222, 'z'};
const Dummy sample2 PROGMEM = {12345, 231, false, -11111, 'A'};

// The following function needs value of sample1 to be read from flash
void read_and_use_sample1()
{
    // value will get copied with sample1 read-only content
    Dummy value;
    // request reading sample1 from flash into local variable value
    flash::read_flash(&sample1, value);
    // Here we can use value which is {54321, 123, true, -22222, 'z'}

}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Formatted Output example ###

Compared to Arduino API, FastArduino brings formatted streams as can be found in standard C++; although more verbose than usual C `printf()` function, formatted streams allow compile-time safety.

Here is an example that prints formatted data to USB:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/uart.h>

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

REGISTER_UATX_ISR(0)

int main()
{
    board::init();
    sei();
	
    serial::hard::UATX<board::USART::USART0> uart{output_buffer};
    uart.register_handler();
    uart.begin(115200);

    streams::FormattedOutput<streams::OutputBuffer> out = uart.fout();
    uint16_t value = 0x8000;
    out << F("value = 0x") << hex << value 
        << F(", ") << dec << value 
        << F(", 0") << oct << value 
        << F(", B") << bin << value << endl;
    return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here, we use `uart.fout()` instead of `uart.out()` to get a `streams::FormattedOutput` on which we can use the "insertion operator" `<<`.

If you are used to programming with C++ for more usual systems (e.g. Linux), then you will immediately recognize [`std::ostream` API](http://www.cplusplus.com/reference/ostream/ostream/operator%3C%3C/) which FastArduino library tries to implement with some level of fidelity.

You can also find more details in `streams` namespace documentation.

Here is the equivalent code with Arduino API:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
void setup()
{
    Serial.begin(115200);
    byte value = 0x8000;
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once again, we can compare the size of both:
|           | Arduino API | FastArduino |
|-----------|-------------|-------------|
| code size | 1808 bytes  | 1412 bytes  |
| data size | 186 bytes   | 77 bytes    |

### Serial Input example ###

FastArduino also implements input streams connected to serial output; here is a simple example:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/uart.h>

static constexpr const uint8_t INPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];

REGISTER_UARX_ISR(0)

int main()
{
    board::init();
    sei();
	
    serial::hard::UARX<board::USART::USART0> uart{input_buffer};
    uart.register_handler();
    uart.begin(115200);

    streams::InputBuffer in = uart.in();

    // Get one character if any
    int input = in.get();
    if (input != InputBuffer:EOF)
    {
        char value = char(input);
    }

    // Wait until a character is ready and get it
    char value = streams::get(in);

    // Wait until a complete string is ready and get it
    char str[64+1];
    int len = streams::gets(in, str, 64);

    return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note the similarities between this example and UATX example above for all the setup parts.
The main differences are:
- use `UARX` type instead of `UATX`
- `REGISTER_UARX_ISR()` instead of `REGISTER_UATX_ISR()` macro for ISR registration
- use `InputBuffer` instead of `OutputBuffer` and `uart.in()` instead of `uart.out()`

Then `UARX` mainly offers one method, `get()`, which returns the next character serially received and buffered; if the input buffer is currently empty, then `get()` returns `InputBuffer::EOF`, which must be tested before dealing with the returned value.

Then the example uses 2 functions defined directly within `streams` namespace:
- `get()`: this is similar to `InputBuffer.get()` except that it **blocks** until one character is available on serial input.
- `gets()`: this blocks until a complete string (terminated by `'\0'`) gets read on serial input and fills the given buffer parameter with that string content.

Note that these 2 functions use `time::yield()` while waiting; this may be linked to `power` management. Please take a look at the documentation for this API for further details.

### Formatted Input example ###

Similar to output, input streams supports formatted input, as can be found in standard C++; once again, formatted input streams allow compile-time safety.

The following example uses formatted input to read values from USB:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/uart.h>

// Define vectors we need in the example
REGISTER_UARX_ISR(0)

// Buffers for UARX
static const uint8_t INPUT_BUFFER_SIZE = 64;
static char input_buffer[INPUT_BUFFER_SIZE];

using INPUT = streams::FormattedInput<streams::InputBuffer>;

int main()
{
    board::init();
    sei();
	
    // Start UART
    serial::hard::UARX<board::USART::USART0> uarx{input_buffer};
    uarx.register_handler();
    uarx.begin(115200);
    INPUT in = uarx.fin();

    // Wait for a char
    char value1;
    in >> streams::skipws >> value1;

    // Wait for an uint16_t
    uint16_t value2;
    in >> streams::skipws >> value2;

    return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here, we use `uart.fin()` instead of `uart.in()` to get a `streams::FormattedInput` on which we can use the "extraction operator" `>>`. All extractions are blocking and will not return until the required type can be read from the buffer.

If you are used to programming with C++ for more usual systems (e.g. Linux), then you will immediately recognize [`std::istream` API](http://www.cplusplus.com/reference/istream/istream/operator%3E%3E/) which FastArduino library tries to implement with some level of fidelity.

You can also find more details in `streams` namespace documentation.

We have already seen `UATX` and `UARX` as classes for sending, resp. receiving, data through serial. There is also `UARX` which combines both.

As you know, the number of physical (hardware) UART available on an MCU target is limited, some targets (ATtiny) don't even have any hardware UART at all. For this reason, if you need extra UART featurs to connect to some devices, you can use software UART API, documented in [namespace `serial::soft`](TODO). As this more complicated to use, it is not part of this basic tutorial, but will be addressed later on.


@anchor analoginput Basics: analog input
----------------------------------------

Here is a simple example using analog input API to read a value from some sensor (thermistor, potentiometer, whatever you like) and lights a LED if the read value is above some threshold:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This example is an adaptation of the [first GPIO example](@ref gpio) of this tutorial.

The first change consists in including the necessary header:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/analog_input.h>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Then we have the definition of the `sensor` variable:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    analog::AnalogInput<board::AnalogPin::A0> sensor;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here we instantiate `AnalogInput` for analog pin `A0` (on Arduino UNO).

In the infinite loop, we then get the current analog value of `sensor` and compare it to `THRESHOLD` constant:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
        if (sensor.sample() > THRESHOLD)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

By default, sample values are on 10 bits (0..1023) represented as `uint16_t`.

If you don't need such precision, you can define `sensor` differently:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    analog::AnalogInput<board::AnalogPin::A0, board::AnalogReference::AVCC, uint8_t> sensor;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note the two additional template arguments provided to `AnalogInput<...>`:
- the first added argument `board::AnalogReference::AVCC`, although seldom changed, may become important when you create your own boards from MCU chips; you can further read API documentation if you need more information about it
- the second added argument is the type of returned samples, either `uint16_t` (default value) or `uint8_t`. The type determines the samples precision:
    - `uint8_t`: 8 bits (samples between 0 and 255)
    - `uint16_t`: 10 bits (samples between 0 and 1023)

Now let's compare the first example with the equivalent Arduino core API program:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As usual, we compare the size of both:
|           | Arduino API | FastArduino |
|-----------|-------------|-------------|
| code size | 204 bytes   | 926 bytes   |
| data size | 0 bytes     | 9 bytes     |

Note that Arduino core API does not allow you any other precision than 10 bits.


@anchor timer Basics: timer
---------------------------

TODO reminder on what timers are and what they can do.

TODO all timers are similar but different (prescalers, size, capabilities).

TODO start simple example (which one?)

