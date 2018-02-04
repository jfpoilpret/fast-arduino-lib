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
5. [real-time timer](@ref rtt)
6. [PWM](@ref pwm)
7. [utilities](@ref utils)

Advanced:
1. [watchdog](@ref watchdog)
2. [interrupts](@ref interrupts)
3. [events, scheduler](@ref events)
4. [power](@ref power)
5. [EEPROM](@ref eeprom)
6. SPI devices management
7. I2C devices management
8. software UART

Devices (not yet documented):
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
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
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

int main() __attribute__((OS_main));
int main()
{
    board::init();
    sei();
	
    serial::hard::UATX<board::USART::USART0> uart{output_buffer};
    uart.register_handler();
    uart.begin(115200);

    streams::ostream out = uart.out();
    out.write("Hello, World!\n");
    out.flush();
    return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
As usual, at first we need to include the proper header (`uart.h`) to use its API.

Then, we define a buffer that will be used by the `UART` API to transmit characters to your PC through USB. You may find it cumbersome to do it yourself but it brings a huge advantage: you are the one to decide of the buffer size, whereas in Arduino API, you have no choice. Here, we consider 64 bits to be big enough to store characters that will be transmitted to the PC. How `UART` is using this buffer is not important to you though.

Then we *register an ISR* necessary for transmissions to take place; this is done by the `REGISTER_UATX_ISR(0)` macro. Explicit ISR registration is one important design choice of FastArduino: **you** decide which ISR should be registered to do what. This may again seem cumbersome but once again this gives you the benefit to decie what you need, hence build your application the way you want it.

The code that follows instantiates a `uart::hard::UATX` object that is using `board::USART::USART0` (the only one available on UNO) and based on the previously created buffer. Note that `UATX` class is in charge of **only** transmitting characters, not receiving. Other classes exist for only receiving (`UARX`), or for doing both (`UART`).

Once created, `uart` needs to be *linked* to the ISR previously registered, this is done through `uart.register_handler()`. Then we can set `uart` ready for transmission, at serial speed of 115200 bps.

Next step consists in extracting, from `uart`, a `streams::ostream` that will allow us to send characters or strings to USB:

    out.write("Hello, World!\n");

The last important instruction, `out.flush()`, waits for all characters to be transmitted before leaving the program.

Do note the specific `main` declaration line before its definition: `int main() __attribute__((OS_main));`. This helps the compiler perform some optimization on this function, and may avoid generating several dozens code instructions in some circumstances. In some situations though, this may increase code size by a few bytes; for your own programs, you would have to compile with and without this line if you want to find what is the best for you.

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
| code size | 1440 bytes  | 662 bytes   |
| data size | 200 bytes   | 91 bytes    |
The data size is big because the buffer used by `Serial` has a hard-coded size (you cannot change it without modifying and recompiling Arduino API). Moreover, when using `Serial`, 2 buffers are created, one for input and one for output, even though you may only need the latter one!

Now let's take a look at the 91 bytes of data used in the FastArduino version of this example, how are they broken down?
| Source            | data size   |
|-------------------|-------------|
| `output_buffer`   | 64 bytes    |
| power API         | 1 byte      |
| `UATX` ISR        | 2 bytes     |
| `UATX` vtable     | 8 bytes     |
| "Hello, World!\n" | 16 bytes    |
| **TOTAL**         | 91 bytes    |

*vtable* is specific data created by C++ compiler for classes with `virtual` methods: every time you use virtual methods in classes, this will add more data size, this is why FastArduino avoids `virtual` as much as possible.

As you can see in the table above, the constant string `"Hello, World!\n"` occupies 16 bytes of data (i.e. AVR SRAM) in addition to 16 bytes of Flash (as it is part of the program and must eb stored permanently). If your program deals with a lot of constant strings like this, you may quickly meet a memory problem with SRAM usage. This is why it is more effective to keep these strings exclusively in Flash (you have no choice) but load them t SRAM only when they are needed, i.e. when they get printed to `UATX` as in the sample code.

How do we change our program so that this string is only stored in Flash? We can use FastArduino `flash` API for that, by changing only one line of code:

    out.write(F("Hello, World!\n"));

Note the use of `F()` macro here: this makes the string reside in Flash only, and then it is being read from Flash "on the fly" by `out.write()` method; the latter method is overloaded for usual C-strings (initial example) and for C-strings stored in Flash only.

We can compare the impact on sizes:
|           | without %F() | with %F()   |
|-----------|--------------|-------------|
| code size | 662 bytes    | 668 bytes   |
| data size | 91 bytes     | 75 bytes    |
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

Compared to Arduino API, FastArduino brings formatted streams as can be [found in standard C++](https://en.wikipedia.org/wiki/Input/output_(C%2B%2B)); although more verbose than usual C `printf()` function, formatted streams allow compile-time safety.

Here is an example that prints formatted data to USB:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/uart.h>

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

REGISTER_UATX_ISR(0)

using namespace streams;

int main()
{
    board::init();
    sei();
	
    serial::hard::UATX<board::USART::USART0> uart{output_buffer};
    uart.register_handler();
    uart.begin(115200);

    ostream out = uart.out();
    uint16_t value = 0x8000;
    out << F("value = 0x") << hex << value 
        << F(", ") << dec << value 
        << F(", 0") << oct << value 
        << F(", B") << bin << value << endl;
    return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here, we still use `uart.out()`, but here we use its "insertion operator" `<<`.

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
| code size | 1808 bytes  | 1804 bytes  |
| data size | 186 bytes   | 83 bytes    |

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

    streams::istream in = uart.in();

    // Wait until a character is ready and get it
    char value;
    in.get(value);

    // Wait until a complete string is ready and get it
    char str[64+1];
	in.get(str, 64+1);

    return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note the similarities between this example and UATX example above for all the setup parts.
The main differences are:
- use `UARX` type instead of `UATX`
- `REGISTER_UARX_ISR()` instead of `REGISTER_UATX_ISR()` macro for ISR registration
- use `istream` instead of `ostream` and `uart.in()` instead of `uart.out()`

Then `UARX` mainly offers one method, `get()`, which returns the next character serially received and buffered; if the input buffer is currently empty, then `get()` will block.

The example uses 2 flavors of this `istream` method:
- `get(char&)`: this method **blocks** until one character is available on serial input.
- `get(char*, size_t, char)`: this blocks until a complete string (terminated by a specified delimiter character, `'\n'` by default) gets read on serial input and fills the given buffer parameter with that string content.

Note that these 2 methods use `time::yield()` while waiting; this may be linked to `power` management. Please take a look at the documentation for this API for further details.

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

using INPUT = streams::istream;

int main()
{
    board::init();
    sei();
	
    // Start UART
    serial::hard::UARX<board::USART::USART0> uarx{input_buffer};
    uarx.register_handler();
    uarx.begin(115200);
    INPUT in = uarx.in();

    // Wait for a char
    char value1;
    in >> streams::skipws >> value1;

    // Wait for an uint16_t
    uint16_t value2;
    in >> streams::skipws >> value2;

    return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here, we still use `uart.in()` but we use its "extraction operator" `>>`. All extractions are blocking and will not return until the required type can be read from the buffer.

If you are used to programming with C++ for more usual systems (e.g. Linux), then you will immediately recognize [`std::istream` API](http://www.cplusplus.com/reference/istream/istream/operator%3E%3E/) which FastArduino library tries to implement with some level of fidelity.

You can also find more details in `streams` namespace documentation.

We have already seen `UATX` and `UARX` as classes for sending, resp. receiving, data through serial. There is also `UARX` which combines both.

As you know, the number of physical (hardware) UART available on an MCU target is limited, some targets (ATtiny) don't even have any hardware UART at all. For this reason, if you need extra UART featurs to connect to some devices, you can use software UART API, documented in [namespace `serial::soft`](TODO). As this is more complicated to use, it is not part of this basic tutorial, but will be addressed later on.


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
| code size | 926 bytes   | 204 bytes   |
| data size | 9 bytes     | 0 bytes     |

Note that Arduino core API does not allow you any other precision than 10 bits.


@anchor timer Basics: timer
---------------------------
A timer (it should actually be named "timer/counter") is a logic chip or part of an MCU that just "counts" pulses of a clock at a given frequency. It can have several modes. It is used in many occasions such as:
- real time counting
- asynchronous tasks (one-shot or periodic) scheduling
- PWM signal generation (see [PWM](@ref pwm) for further details)

A timer generally counts up, but it may also, on some occasions, count down; it may trigger interrupts on several events (i.e. when counter reaches some specific limits), it may drive some specific digital output pins, and sometimes it may also be driven by digital input pins (to capture counter value).

There are typically several independent timers on an MCU, but they are not all the same. Timers may differ in:
- counter size (8 or 16 bits for AVR timers)
- list of settable frequencies (timer frequencies are derived from the MCU clock by prescaler devices)
- the timer modes supported
- the pins they are connected to
- specific capabilities they may have

Rather than explaining the theory further, we will start studying a simple example that uses a timer for blinking a LED, a little bit like the first example in this tutorial, but totally driven asynchronously:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/gpio.h>
#include <fastarduino/timer.h>

constexpr const board::Timer TIMER = board::Timer::TIMER1;
using CALCULATOR = timer::Calculator<TIMER>;
using TIMER_TYPE = timer::Timer<TIMER>;
constexpr const uint32_t PERIOD_US = 1000000;

constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = CALCULATOR::CTC_prescaler(PERIOD_US);
constexpr const TIMER_TYPE::TIMER_TYPE COUNTER = CALCULATOR::CTC_counter(PRESCALER, PERIOD_US);

class Handler
{
public:
	Handler(): _led{gpio::PinMode::OUTPUT, false} {}
	
	void on_timer()
	{
		_led.toggle();
	}
	
private:
	gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;
};

// Define vectors we need in the example
REGISTER_TIMER_COMPARE_ISR_METHOD(1, Handler, &Handler::on_timer)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	Handler handler;
	interrupt::register_handler(handler);
	TIMER_TYPE timer{timer::TimerMode::CTC, PRESCALER, timer::TimerInterrupt::OUTPUT_COMPARE_A};
	timer.begin(COUNTER);
	
	while (true) ;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This example looks much more complex than all previous examples but it is straightforward to understand once explained part after part.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/gpio.h>
#include <fastarduino/timer.h>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In addition to GPIO, we include the header containing all Timer API.

For this example, we use Arduino UNO, which MCU (ATmega328P) includes 3 timers (named respectively `Timer0`, `Timer1`, `Timer2` in its datasheet), we use Timer1 which is 16-bits in size:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
constexpr const board::Timer TIMER = board::Timer::TIMER1;
using CALCULATOR = timer::Calculator<TIMER>;
using TIMER_TYPE = timer::Timer<TIMER>;
constexpr const uint32_t PERIOD_US = 1000000;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Although not needed, it is a good practice to define a `const`, named `TIMER` in this snippet, valued with the real timer we intend to use.
Then we define 2 new type aliases, `CALCULATOR` and `TIMER` that will help us type less code (this is common recommended practice when using C++ templates heavily in programs):
- `CALCULATOR` is the type of a class which provides `static` utility methods that will help us configure the timer we have selected; do note that, since all timers are different, `CALCULATOR` is specific to one timer only; hence if our program was using 2 distinct timers, we would have to define two distinct calculator type aliases, one for each timer.
- `TIMER_TYPE` is the type of the class that embed all timer API for the specific timer we have selected.

Finally we define `PERIOD_US` the period, in microseconds, at which we want the LED to blink. Please note that this is in fact half the actual period, because this is the time at which we will toggle the LED light.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = CALCULATOR::CTC_prescaler(PERIOD_US);
constexpr const TIMER_TYPE::TIMER_TYPE COUNTER = CALCULATOR::CTC_counter(PRESCALER, PERIOD_US);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The above snippet defines constant settings, computed by `CALCULATOR` utility class, that we will later use to initialize our timer:
- `PRESCALER` is the **optimum** prescaler value that we can use for our timer in order to be able to count up to the requested period, i.e. 1 second; the type of prescaler is an `enum` that depends on each timer (because the list of available prescaler values differ from one timer to another). The prescaler defines the number by which the MCU clock frequency will be divided to provide the pulses used to increment the timer. We don't need to know this value or fix it ourselves because `CALCULATOR::CTC_prescaler` calculates the best choice for us.
- `COUNTER` is the maximum counter value that the timer can reach until 1 second has ellapsed; its type is based on the timer we have selected (i.e. `Timer1` => 16 bits => `uint16_t`), but we don't need to fix this type ourselves because it depends on the timer we have selected.

Note that, although we know in advance which timer we use, we always avoid declaring direct types (such as `uint16_t`) in order to facilate a potential change to another timer in the future, without having to change several code locations. 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class Handler
{
public:
	Handler(): _led{gpio::PinMode::OUTPUT, false} {}
	
	void on_timer()
	{
		_led.toggle();
	}
	
private:
	gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here we define the class which implements the code in charge of blinking the LED every time the timer has reached its maximum value, i.e. every second.
There is nothing special to explain here, except that the method `on_timer()` is a *callback function* which will get called asynchronously (from interrupt handlers) when the timer reaches its max.

Since timers generate interruptions, we need to "attach" our handler code above to the suitable interruption, this is done through the following line of code:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
REGISTER_TIMER_COMPARE_ISR_METHOD(1, Handler, &Handler::on_timer)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
`REGISTER_TIMER_COMPARE_ISR_METHOD` is a macro that will generate extra code (code you do not need, nor want, to see) to declare the Interrupt Service Routine (*ISR*) attached to the proper interruption of our selected timer; it takes 3 arguments:
- `1` is the timer number (`0`, `1` or `2` on UNO)
- `Handler` is the class that contains the code to be called when the interrupt occurs
- `&Handler::on_timer` is the Pointer to Member Function (often abbreviated *PTMF* by usual C++ developers) telling which method from `Handler` shall be called back when the interrupt occurs
In FastArduino, interrupt handling follows some patterns that are further described [here](@ref interrupts) and won't be developed in detail now.

Now we can finally start writing the code of the `main()` function:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	Handler handler;
	interrupt::register_handler(handler);

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Past the usual initialization stuff, this code performs an important task regarding interrupt handling: it creates `handler`, an instance of the `Handler` class that has been defined before as the class to handle interrupts for the selected timer, and then it **registers** this handler instance with FastArduino. Now we are sure that interrupts for our timer will call `handler.on_timer()`.

The last part of the code creates and starts the timer we need in our program:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	TIMER_TYPE timer{timer::TimerMode::CTC, PRESCALER, timer::TimerInterrupt::OUTPUT_COMPARE_A};
	timer.begin(COUNTER);
	
	while (true) ;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
`timer` is the instance of `timer::Timer` API for `board::Timer::TIMER1`; upon instantiation, it is passed the timer mode to use, the previously calculated clock prescaler, and the interrupt we want to enable.

Here we use *CTC* mode (Clear Timer on Compare); in this mode the counter is incremented until it reaches a maximum value, then it triggers an interrupt and it clears the counter value back to zero and starts counting up again.

To ensure that our handler to get called back when the timer reaches 1 second, we set `timer::TimerInterrupt::OUTPUT_COMPARE_A`, which enables the proper interrupt on this timer: when the counter is reached, an interrupt will occur, the properly registered ISR will be called, and in turn it will call our handler.

Then `timer.begin()` activates the timer with the maximum counter value, that was calculated initially in the program. This value, along with `PRESCALER`, has been calculated in order for `timer` to generate an interrupt (i.e. call `handler.on_timer()`) every second.

Note the infinite loop `while (true);` at the end of `main()`: without it the program would terminate immediately, giving no chance to our timer and handler to operate as expected. What is interesting to see here is that the main code does not do anything besides looping forever: all actual stuff happens asynchronously behind the scenes!

I would have liked to perform a size comparison with Arduino API, but unfortunately, the basic Arduino API does not provide an equivalent way to directly access a timer, hence we cannot produce the equivalent code here. Anyway, here is the size for the example above:
|           | FastArduino |
|-----------|-------------|
| code size | 248 bytes   |
| data size | 2 bytes     |


@anchor rtt Basics: real-time timer 
-----------------------------------

A real-time timer is primarily a device that tracks time in standard measurements (ms, us).

It may be used in various situations such as:
- delay program execution for some us or ms
- capture the duration of some event with good accuracy
- implement timeouts in programs waiting for an event to occur
- generate periodic events

The simple example that follows illustrates the first use case:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/gpio.h>
#include <fastarduino/realtime_timer.h>

REGISTER_RTT_ISR(0)

const constexpr uint32_t BLINK_DELAY_MS = 500;

int main()
{
	board::init();
	sei();

	timer::RTT<board::Timer::TIMER0> rtt;
	rtt.register_rtt_handler();
	rtt.begin();

	gpio::FastPinType<board::DigitalPin::LED>::TYPE led{gpio::PinMode::OUTPUT};
	while (true)
	{
		led.toggle();
		rtt.delay(BLINK_DELAY_MS);
	}
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This example looks much like the first blinking example in this tutorial, with a few changes.

First off, as usual the neceaary header file is included:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/realtime_timer.h>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Then we need to register an ISR for the RTT feature to work properly:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
REGISTER_RTT_ISR(0)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Then, in `main()`, after the usual initialization stuff, we create a real-time timer instance, based on AVR UNO Timer0 (8-bits timer), register it with the ISR previously registered with `REGISTER_RTT_ISR(0)` macro, and finally starts it counting time.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	timer::RTT<board::Timer::TIMER0> rtt;
	rtt.register_rtt_handler();
	rtt.begin();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Finally, we have the usual loop, toogling the LED, and then delay for 10s, using the RTT API: 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
		rtt.delay(BLINK_DELAY_MS);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Let's examine the size of this program and compare it with the first example of this tutorial, which used `time::delay_ms()`:
|           | delay_ms   | RTT::delay  |
|-----------|------------|-------------|
| code size | 154 bytes  | 404 bytes   |
| data size | 0 bytes    | 3 bytes     |

As you can see, code and data size is higher here, so what is the point of using `RTT::delay()` instead of `time::delay_ms()`? The answer is **power consumption**:
- `time::delay_ms` is a busy loop which requires the MCU to be running during the whole delay, hence consuming "active supply current" (about 15mA for an ATmega328P at 16MHz)
- `RTT::delay()` will set the MCU to pre-defined sleep mode and will still continue to operate well under most available sleep modes (this depends on which timer gets used, refer to [AVR datasheet](TODO) for further details); this will alow reduction of supply current, hence power consumption. Current supply will be reduced more or less dramatically according to the selected sleep mode.

Another practical use of RTT is to measure the elapsed time between two events. For instance it can be used with an ultrasonic ranging device to measure the duration of an ultrasound wave to do a roundript from the device to an obstacle, then calculate the actual distance in mm. The following snippet shows how it could look like for an HC-SR04 sensor:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
// Declare 2 pins connected to HC-SR04
gpio::FastPinType<board::DigitalPin::D0>::TYPE trigger{gpio::PinMode::OUTPUT};
gpio::FastPinType<board::DigitalPin::D1>::TYPE echo{gpio::PinMode::INPUT};

// Declare RTT (note: don't forget to call REGISTER_RTT_ISR(1) macro in your program)
timer::RTT<board::Timer::TIMER1>& rtt;

// Send a 10us pulse to the trigger pin
trigger.set();
time::delay_us(10);
trigger.clear();

// Wait for echo signal start
while (!echo.value()) ;
// Reset RTT time
rtt.millis(0);
// Wait for echo signal end
while (echo.value()) ;
// Read current time
time::RTTTime end = rtt.time();
// Calculate the echo duration in microseconds
uint16_t echo_us = uint16_t(end.millis * 1000UL + end.micros);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note that this snippet is just an example and is not usable as is: it does not include a timeout mechanism to avoid waiting the echo signal forever (which can happen if the ultrasonic wave does not encounter an obstacle within its possible range, i.e. 4 meters). Also, this approach could be improved by making it interrupt-driven (i.e. having interrupts generated when the `echo_pin` changes state).

Another interesting use of RTT is to perform some periodic actions. FastArduino implements an events handling mechanism that can be connected to an RTT in order to deliver periodic events. This mechanism is [described later](@ref events) in this tutorial.


@anchor pwm Basics: PWM 
-----------------------

PWM (*Pulse Width modulation*) is a technique that can be used to simulate generation of an analog voltage level through a purely digital output. This is done by varying the *duty cycle* of a rectangular pulse wave, i.e. the ratio of "on" time over the wave period.

PWM is implemented by MCU through timers.

FastArduino includes special support for PWM. The following example demonstrates PWM to increase then decrease the light emitted by a LED:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/time.h>
#include <fastarduino/pwm.h>

static constexpr const board::Timer TIMER = board::Timer::TIMER0;
using TIMER_TYPE = timer::Timer<TIMER>;
using CALC = timer::Calculator<TIMER>;
static constexpr const uint16_t PWM_FREQUENCY = 450;
static constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = CALC::FastPWM_prescaler(PWM_FREQUENCY);

static constexpr const board::DigitalPin LED = board::PWMPin::D6_PD6_OC0A;
using LED_PWM = analog::PWMOutput<LED>;

int main()
{
	board::init();
	sei();

	// Initialize timer
	TIMER_TYPE timer{timer::TimerMode::FAST_PWM};
	timer.begin(PRESCALER);
	
	LED_PWM led{timer};
	// Loop of samplings
	while (true)
	{
		for (LED_PWM::TYPE duty = 0; duty < LED_PWM::MAX; ++duty)
		{
			led.set_duty(duty);
			time::delay_ms(50);
		}
		for (LED_PWM::TYPE duty = LED_PWM::MAX; duty > 0; --duty)
		{
			led.set_duty(duty);
			time::delay_ms(50);
		}
	}
	return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The program starts by including the header for PWM API; this will automatically include the timer API header too.

Then a timer is selected for PWM (note that the choice of a timer imposes the choice of possible pins) and a prescaler value computed for it, based on the PWM frequency we want to use, 450Hz, which is generally good enough for most use cases (dimming a LED, rotating a DC motor...):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
static constexpr const board::Timer TIMER = board::Timer::TIMER0;
using TIMER_TYPE = timer::Timer<TIMER>;
using CALC = timer::Calculator<TIMER>;
static constexpr const uint16_t PWM_FREQUENCY = 450;
static constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = CALC::FastPWM_prescaler(PWM_FREQUENCY);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Then we define the pin that will be connected to the LED and the PWMOutput type for this pin:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
static constexpr const board::DigitalPin LED = board::PWMPin::D6_PD6_OC0A;
using LED_PWM = analog::PWMOutput<LED>;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note that `board::PWMPin` namespace limits the pins to PWM-enabled pins; also note the pin name `D6_PD6_OC0A` includes useful information:
- this is pin `D6` on Arduino UNO
- this pin is on `PD6` i.e. Port D bit #6
- this pin is connectable to `OC0A` i.e. Timer 0 COM A

Then, in `main()`, after the usual initialization code, we initialize and start the timer:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	// Initialize timer
	TIMER_TYPE timer{timer::TimerMode::FAST_PWM};
	timer.begin(PRESCALER);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Notice we use the *Fast PWM* mode here, but we might as well use *Phase Correct PWM* mode.

Next we connect the LED pin to the timer:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	LED_PWM led{timer};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In the main loop, we have 2 consecutive loops, the first increases the light, the second decreases it. Both loops vary the duty cycle between its limits:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
		for (LED_PWM::TYPE duty = 0; duty < LED_PWM::MAX; ++duty)
		{
			led.set_duty(duty);
			time::delay_ms(50);
		}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note the use of `LED_PWM::TYPE`, which depends on the timer selected (8 or 16 bits), and `LED_PWM::MAX` which provides the maximum value usable for the duty cycle, i.e. the value mapping to 100% duty cycle. Pay attention to the fact that `LED_PWM::TYPE` is unsigned, this explains the 2nd loop:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
		for (LED_PWM::TYPE duty = LED_PWM::MAX; duty > 0; --duty)
		{
			led.set_duty(duty);
			time::delay_ms(50);
		}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here, we shall not use `duty >= 0` as the `for` condition, because that condition would be always `true`, hence the loop would be infinite.

Now let's compare this example with the Arduino API equivalent:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Nothing special to comment here, except:
- `duty` values are always limited to 255 even though some PWM pins are attached to a 16-bits timer
- you cannot choose the PWM mode you want to use, it is fixed by Arduino API and varies depending on which timer is used (e.g. for Arduino UNO, Timer0 uses Fast PWM, whereas Time1 and Timer2 use Phase Correct PWM mode)
- you cannot choose the PWM frequency, this is imposed to you by Arduino API and varies depending on which timer is used
- you may pass any pin value to `analogWrite()` and the sketch will still compile and upload but the sketch will not work

Comparing sizes once again shows big differences:
|           | Arduino API | FastArduino |
|-----------|-------------|-------------|
| code size | 1302 bytes  | 288 bytes   |
| data size | 9 bytes     | 0 byte      |


@anchor utils Basics: utilities
-------------------------------

FastArduino provides several general utilities, gathered inside one namespace `utils`.

We will not demonstrate each of these utilities here but just show a few of them in action. In your own programs, if you find yourself in need of some helper stuff that you think deserves to be provided as a general utility, then first take a look at FastArduino [utilities API documentation](namespaceutils.html) and check if you don't find it there, or something similar that you could use in your situation.

FastArduino utilities are made of different kinds:
- low-level utilities: mostly functions to handle bytes and bits
- value conversion utilities: functions to help convert a value from one referential to another, very useful when dealing with sensors of all sorts

### Low-level utilities examples

The few examples in this section will introduce you to a few functions that may prove useful if you need to handle devices that are not natively supported by FastArduino.

1. `utils::swap_bytes`: this function is useful whenever you use a sensor device that provides you with integer values, coded on 2 bytes, with high byte first and low byte second; since AVR MCU are "little-endian" processors, they expect words in the opposite order: low byte first, high byte second, hence in order to interpret values provided by that device, you need to first swap their bytes. Bytes swap is performed "in-place", i.e. the original value is replace with the converted value. The following example is an excerpt of `hmc5883l.h` provided by FastArduino, where magnetic fields in 3 axes have to be converted from big endian (as provided by the HMC5883L) to little endian (as expected by the AVR MCU):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    bool magnetic_fields(MagneticFields& fields)
    {
        if (	this->write(DEVICE_ADDRESS, OUTPUT_REG_1, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
            &&	this->read(DEVICE_ADDRESS, fields, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK)
        {
            utils::swap_bytes(fields.x);
            utils::swap_bytes(fields.y);
            utils::swap_bytes(fields.z);
            return true;
        }
        else
            return false;
    }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2. `utils::bcd_to_binary`: this function is useful when you use a sensor device that provides values coded as *BCD* (binary-coded decimal), i.e. where each half-byte (*nibble*) contains the value of one digit (i.e. `0` to `9`), thus holding a range of values from `0` to `99`. Many RTC devices use BCD representation for time. In order to performa calculation on BCD values, you need to first convert them to binary. The opposite function is also provided as `utils::binary_to_bcd`. The following example is an excerpt of `ds1307.h` provided by FastArduino, where each datetime field (seconds, minutes, hours...) have to be covnerted from BCD to binary:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
    bool getDateTime(tm& datetime)
    {
        // send register address to read from (0)
        // read datetime at address 0
        if (	write(DEVICE_ADDRESS, TIME_ADDRESS, i2c::BusConditions::START_NO_STOP) == i2c::Status::OK
            &&	read(DEVICE_ADDRESS, datetime, i2c::BusConditions::REPEAT_START_STOP) == i2c::Status::OK)
        {
            // convert DS1307 output (BCD) to integer type
            datetime.tm_sec = utils::bcd_to_binary(datetime.tm_sec);
            datetime.tm_min = utils::bcd_to_binary(datetime.tm_min);
            datetime.tm_hour = utils::bcd_to_binary(datetime.tm_hour);
            datetime.tm_mday = utils::bcd_to_binary(datetime.tm_mday);
            datetime.tm_mon = utils::bcd_to_binary(datetime.tm_mon);
            datetime.tm_year = utils::bcd_to_binary(datetime.tm_year);
        }
    }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

### Conversion utilities examples

Device sensors measure some physical quantity and generally provide you with some integer value that somehow maps to the physical value. hence to make use of the raw value provided by a sensor, you need to convert it to some more meaningful value that you can understand and operate upon.

Or conversely, you may just need to compare the physical value agains some thresholds (e.g. check the gyroscopic speed according to some axis is less than 10°/s), and perform some action when this is not true. In this situation, you don't really need to convert the raw sensor value into a physical quantity to compare to the physical threshold, but rather convert (once only) the physical threshold into the corresponding raw value (a constant in your program) and then only compare raw values, which is:
- more performant (no conversion needed before comparison)
- more size efficient (conversion of threshold can be done at compile time, hence no code is generated for it)

FastArduino utilities provide several conversion methods between raw and physical quantities, according to raw and physical ranges (known for each sensor), and unit prefix (e.g. kilo, mega, giga, centi, milli...). These methods are `constexpr`, which means that, when provided with constant arguments, they will be evaluated at compile-time and return a value that is itself stored as a constant.

1. `utils::map_physical_to_raw`: although it may seem complicated by its list of arguments, this function is actually pretty simple, as demonstrated in the snippet hereafter:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	static constexpr const int16_t ACCEL_1 = map_physical_to_raw(500, UnitPrefix::MILLI, 2, 15);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In this example, we convert the acceleration value 500mg (g is *9.81 m/s/s*) to the equivalent raw value as produced by an MPU-6050 accelerometer, using *+/-2g* range (`2` is the max physical value we can get with this device using this range) where this raw value is stored on `15 bits` (+1 bit for the sign), i.e. `32767` is the raw value returned by the device when the measured acceleration is `+2g`.

2. `utils::map_raw_to_physical`: this method does the exact opposite of `utils::map_physical_to_raw` with the same parameters, reversed:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	int16_t rotation = map_raw_to_physical(raw, UnitPrefix::CENTI, 250, 15);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In this example, we convert `raw` which is returned by the MPU-6050 gyroscope, using range *+/-250°/s* with `15 bits` precision (+1 bit for the sign), i.e. `32767` is the raw value returned by the device when the measured rotation speed is `+250°/s`. The calculated value is returned in *c°/s* (centi-degrees per second).

In addition to these functions, FastArduino utilities also include the more common `utils::map` and `utils::constrain` which work like their Arduino API equivalent `map()` and `constrain()`.


@anchor watchdog Advanced: Watchdog
-----------------------------------

In general, a watchdog is a device (or part of a device) that is used to frequently check that a system is not hanging. AVR MCU include such a device and this can be programmed to other purposes than checking the system is alive, e.g. as a simple timer with low-power consumption. This is in that purpose that FastArduino defines a specific Watchdog API.

FastArduino defines 2 watchdog classes. The first one, `WatchdogSignal` allows you to generate watchdog timer interrupts at a given period.
The following example is yet another way to blink a LED:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/gpio.h>
#include <fastarduino/power.h>
#include <fastarduino/watchdog.h>

// Define vectors we need in the example
REGISTER_WATCHDOG_ISR_EMPTY()

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	gpio::FastPinType<board::DigitalPin::LED>::TYPE led{gpio::PinMode::OUTPUT};

	watchdog::WatchdogSignal watchdog;
	watchdog.begin(watchdog::WatchdogSignal::TimeOut::TO_500ms);
	
	while (true)
	{
		led.toggle();
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In this example, we use `watchdog` API but also `power` API in order to reduce power-consumption.

As we use `WatchdogSignal`, we need to register an ISR, however we do not need any callback, hence we just register an empty ISR.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
REGISTER_WATCHDOG_ISR_EMPTY()
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Then we have the usual `main()` function which, after defining `led` output pin, starts the watchdog timer:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	watchdog::WatchdogSignal watchdog;
	watchdog.begin(watchdog::WatchdogSignal::TimeOut::TO_500ms);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here we use a 500ms timeout period, which means our sleeping code will be awakened every 500ms.

The infinite loop just toggles `led` pin level and goes to sleep:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	while (true)
	{
		led.toggle();
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
As explained [later](@ref power), `power::Power::sleep(board::SleepMode::POWER_DOWN)` just puts the MCU to sleep until some external interrupts wake it up; in our example, that interrupt is the watchdog timeout interrupt. 

The size of this example is not much bigger than the first example of this tutorial:
|           | FastArduino |
|-----------|-------------|
| code size | 262 bytes   |
| data size | 0 byte      |

FastArduino defines another class, `Watchdog`. This class allows to use the AVR watchdog timer as a clock for events generation. This will be demonstrated when [events scheduling](@ref events) is described.


@anchor interrupts Advanced: Interrupts
---------------------------------------

### Introduction to AVR interrupts

Many AVR MCU features are based upon interrupts, so that the MCU can efficiently react to events such as timer overflow, pin level change, without having to poll for these conditions. Using AVR interrupts makes your program responsive to events relevant to you. This alos allows taking advantage of power sleep modes thus reducing energy consumption while the MCU has nothing to do other than wait for events to occur.

Each AVR MCU has a predefined list of interrupts sources, each individually activable, and each with an associated *vector* which is the address of an Interrupt Service Routine (**ISR**), executed whenver the matching interrupt occurs. Each ISR is imposed a specific name.

Among common interrupt vectors, you will find for example:
- `TIMERn_OVF_vect`: triggered when Timer *n* overflows
- `INTn_vect`: triggered when input pin *INTn* changes level
- `UARTn_RX_vect`: triggered when a new character has been received on serial receiver *UARTn`
- ...

### AVR interrupts handling in FastArduino

In FastArduino, ISR are created upon your explicit request, FastArduino will never add an ISR without your consent. FastArduino calls this ISR creation a **registration**.

ISR registration is performed through macros provided by FastArduino. There are essentially 4 flavours of registration macros:
1. API-specific registration: in this flavour, a FastArduino feature requires to directly be linked to an ISR, through a dedicated macro and a specific registration method (either implicitly called by constructor or explicitly through a specific method). In the previous examples of this tutorial, you have already encountered `REGISTER_UATX_ISR()`, `REGISTER_UARX_ISR()`, `REGISTER_UART_ISR()` and `REGISTER_RTT_ISR()`.
All macros in this flavour follow the same naming scheme: `REGISTER_XXX_ISR`, where *XXX* is the feature handled.
2. Empty ISR registration: in this flavour, you activate an interrupt but do not want any callback for it, but then you have to define an empty ISR for it; this empty ISR will not increase code size of your program. You may wonder why you would want to enable an interrupt but do nothing when it occurs, in fact this is often used to make the MCU sleep (low power consumption) and wake it once an interrupt occurs. You have already seen such usage in a previous example, where `REGISTER_WATCHDOG_ISR_EMPTY()` was used.
3. Method callback registration: with this flavour, you activate an interrupt and want a sepcific method of a given class to be called back when the interrupt occurs; in this flavour, a second step is required inside your code: you need to register an instance of the class that was registered. In this tutorial, previous examples used this approach with `REGISTER_TIMER_COMPARE_ISR_METHOD()` macro and `interrupt::register_handler(handler);` instance registration in `main()`. This is probably the most useful approach as it allows to pass an implicit context (`this` class instance) to the callback.
4. Function callback registration: with this flavour, you can register one of your functions (global or static) as a callback of an ISR. This approach does not require an extra registration step. This is not used as often as the Method callback registration flavour above.

Whenever method callback is needed (typically for flavours 1 and 3 above), then a second registration step is needed.

All FastArduino API respects some guidelines for naming ISR registration macros. All macros are in one of the following formats:
- `REGISTER_XXX_ISR()` for API-specific registration
- `REGISTER_XXX_ISR_EMPTY()` for empty ISR
- `REGISTER_XXX_ISR_CALLBACK()` for method callback
- `REGISTER_XXX_ISR_FUNCTION()` for function callback

Here is a table showing all FastArduino macros to register ISR (*Name* is to be replaced in macro name `REGISTER_NAME_ISR`, `REGISTER_NAME_ISR_EMPTY`, `REGISTER_NAME_ISR_CALLBACK` or `REGISTER_NAME_ISR_FUNCTION`):
| Header             | Name              | Flavours | Comments                                                           |
|--------------------|-------------------|----------|--------------------------------------------------------------------|
| `eeprom.h`         | `EEPROM`          | 1,3,4    | Called when asynchronous EEPROM write is finished.                 |
| `int.h`            | `INT`             | 2,3,4    | Called when an INT pin changes level.                              |
| `pci.h`            | `PCI`             | 2,3,4    | Called when a PCINT pin changes level.                             |
| `pulse_timer.h`    | `PULSE_TIMER8_A`  | 1        | Called when a PulseTimer8 overflows or equals OCRA.                |
| `pulse_timer.h`    | `PULSE_TIMER8_B`  | 1        | Called when a PulseTimer8 overflows or equals OCRB.                |
| `pulse_timer.h`    | `PULSE_TIMER8_AB` | 1        | Called when a PulseTimer8 overflows or equals OCRA or OCRB.        |
| `realtime_timer.h` | `RTT`             | 1,3,4    | Called when RTT timer has one more millisecond elapsed.            |
| `soft_uart.h`      | `UART_PCI`        | 1        | Called when a start bit is received on a PCINT pin linked to UATX. |
| `soft_uart.h`      | `UART_INT`        | 1        | Called when a start bit is received on an INT pin linked to UATX.  |
| `timer.h`          | `COMPARE`         | 2,3,4    | Called when a Timer counter reaches OCRA.                          |
| `timer.h`          | `OVERFLOW`        | 2,3,4    | Called when a Timer counter overflows.                             |
| `timer.h`          | `CAPTURE`         | 2,3,4    | Called when a Timer counter gets captured (when ICP level changes).|
| `uart.h`           | `UATX`            | 1        | Called when one character is finished transmitted on UATX.         |
| `uart.h`           | `UARX`            | 1        | Called when one character is finished received on UARX.            |
| `uart.h`           | `UART`            | 1        | Called when one character is finished transmitted/received on UART.|
| `watchdog.h`       | `WATCHDOG_CLOCK`  | 1        | Called when Watchdog timeout occurs, and clock must be updated.    |
| `watchdog.h`       | `WATCHDOG`        | 2,3,4    | Called when WatchdogSignal timeout occurs.                         |

For further details on ISR registration in FastArduino, you can check [`interrutps.h` API](interrupts_8h.html) for the general approach, and each individual API documentation for specific interrupts.

### Pin Interrupts

One very common usage of AVR interrupts is to handle level changes of specific digital input pins. AVR MCU have two kinds of pin interrupts:
- External Interrupts: an interrupt can be triggered for a specific pin when its level changes or is equal to some value (high or low); the number of such pins is quite limited (e.g. only 2 on Arduino UNO)
- Pin Change Interrupts: an interrupt is triggered when one in a set of pins has a changing level; the same interrupt (hence the same ISR) is used for all pins (typically 8, but not necessarily), thus the ISR must determine, by its own means, which pin has triggered the interrupt; although more pins support this interrupt, it is less convenient to use than external interrupts.

#### External Interrupts

External Interrupts are handled with the API provided by `int.h`. This API is demonstrated in the example below:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/gpio.h>
#include <fastarduino/int.h>
#include <fastarduino/power.h>

constexpr const board::DigitalPin SWITCH = board::ExternalInterruptPin::D2_PD2_EXT0;

class PinChangeHandler
{
public:
	PinChangeHandler():_switch{gpio::PinMode::INPUT_PULLUP}, _led{gpio::PinMode::OUTPUT} {}
	
	void on_pin_change()
	{
		if (_switch.value())
			_led.clear();
		else
			_led.set();
	}
	
private:
	gpio::FastPinType<SWITCH>::TYPE _switch;
	gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;	
};

// Define vectors we need in the example
REGISTER_INT_ISR_METHOD(0, SWITCH, PinChangeHandler, &PinChangeHandler::on_pin_change)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	PinChangeHandler handler;
	interrupt::register_handler(handler);
	interrupt::INTSignal<SWITCH> int0{interrupt::InterruptTrigger::ANY_CHANGE};
	int0.enable();

	// Event Loop
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In this example, most of the MCU time is spent sleeping in power down mode. The pin `INT0` (D2 on UNO) is connected to a push button and used to switch on or off the UNO LED (on D13).

Note how the button pin is defined:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
constexpr const board::DigitalPin SWITCH = board::ExternalInterruptPin::D2_PD2_EXT0;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here we use `board::ExternalInterruptPin` namespace to find out the possible pins that can be used as External Interrupt pins.

Then we define the class that will handle interrupts occurring when the button pin changes level:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class PinChangeHandler
{
public:
	PinChangeHandler():_switch{gpio::PinMode::INPUT_PULLUP}, _led{gpio::PinMode::OUTPUT} {}
	
	void on_pin_change()
	{
		if (_switch.value())
			_led.clear();
		else
			_led.set();
	}
	
private:
	gpio::FastPinType<SWITCH>::TYPE _switch;
	gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;	
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The interesting code here is the `on_pin_change()` method which be called back when the button is pushed or relaxed, i.e. on any level change.
Since this method is called whatever the new pin level, it must explicitly check the button status (i.e. the pin level) to set the right output for the LED pin. Note that, since we use `gpio::PinMode::INPUT_PULLUP`, the pin value will be `false` when the button is pushed, and `true` when it is not.

The next line of code registers `PinChangeHandler` class and `on_pin_change()` method as callback for `INT0` interrupt.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
REGISTER_INT_ISR_METHOD(0, SWITCH, PinChangeHandler, &PinChangeHandler::on_pin_change)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In addition to the class and method, the macro takes the number of the `INT` interrupt (`0`) and the pin connected to this interrupt.
Note that the pin reference is redundant; it is passed as a way to ensure that the provided `INT` number matches the pin that we use, if it doesn't, then your code will not compile.

Then the `main()` function oincludes the following initialization code:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	PinChangeHandler handler;
	interrupt::register_handler(handler);
	interrupt::INTSignal<SWITCH> int0{interrupt::InterruptTrigger::ANY_CHANGE};
	int0.enable();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Once the interrupt handler has been instantiated, it must be registered (2nd step of method callback registration).
Then an `INTSignal` is created for `INT0` and it is set for any level change of the pin.
Finally, the interrupt is activated.

Note the infinite loop in `main()`:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
With this loop, we set the MCU to sleep in lowest energy consumption mode. Only some interrupts (`INT0` is one of them) will awaken the MCU from its sleep, immediately after the matching ISR has been called.

Here is the equivalent example with Arduino API:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
First note that Arduino API has no API for sleep mode management, hence you have to include `<avr/sleep.h>` and handle all this by yourself.
Also note that you can only attach a function to an interrupt. Arduino API offers no way to attach a class member instead.

Now let's just compare examples sizes, as usual:
|           | Arduino API | FastArduino |
|-----------|-------------|-------------|
| code size | 1238 bytes  | 266 bytes   |
| data size | 13 bytes    | 2 bytes     |


#### Pin Change Interrupts

Pin Change Interrupts are handled with the API provided by `pci.h`. This API is demonstrated in the example below:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/gpio.h>
#include <fastarduino/pci.h>
#include <fastarduino/power.h>

constexpr const board::DigitalPin SWITCH = board::InterruptPin::D14_PC0_PCI1;
#define PCI_NUM 1

class PinChangeHandler
{
public:
	PinChangeHandler():_switch{gpio::PinMode::INPUT_PULLUP}, _led{gpio::PinMode::OUTPUT} {}
	
	void on_pin_change()
	{
		if (_switch.value())
			_led.clear();
		else
			_led.set();
	}
	
private:
	gpio::FastPinType<SWITCH>::TYPE _switch;
	gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;	
};

// Define vectors we need in the example
REGISTER_PCI_ISR_METHOD(PCI_NUM, PinChangeHandler, &PinChangeHandler::on_pin_change, SWITCH)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	PinChangeHandler handler;
	interrupt::register_handler(handler);
	interrupt::PCIType<SWITCH>::TYPE pci;
	
	pci.enable_pin<SWITCH>();
	pci.enable();

	// Event Loop
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This example performs the same as the previous example except it uses another pin for the button.
Most of the code is similar, hence we will focus only on differences.

The first difference is in the way we define the pin to use as interrupt source:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
constexpr const board::DigitalPin SWITCH = board::InterruptPin::D14_PC0_PCI1;
#define PCI_NUM 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here we use `board::InterruptPin` namespace to find out the possible pins that can be used as Pin Change Interrupt pins. We select pin D14 and see from its name that is belonging to `PCINT1` interrupt vector. We also define the `PCINT` number as a constant, `PCI_NUM`, for later use.

Then we register `PinChangeHandler` class and `on_pin_change()` method as callback for `PCINT1` interrupt.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
REGISTER_PCI_ISR_METHOD(PCI_NUM, PinChangeHandler, &PinChangeHandler::on_pin_change, SWITCH)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Once we have, as before,registered our handler class, we then create a `PCISignal` instance that we will use to activate the proper interrupt:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	interrupt::PCIType<SWITCH>::TYPE pci;
	
	pci.enable_pin<SWITCH>();
	pci.enable();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Before enabling the `PCINT1` interrupt, we need to first indicate that we are only interested in changes of the button pin.

For this example, we cannot compare sizes with the Arduino API equivalent because Pin Change Interrupts are not supported by the API.


@anchor events Advanced: Events, Scheduler
------------------------------------------

FastArduino supports (and even encourages) events-driven programming, where events are generally generated and queued by interrupts (timer, pin changes, watchdog...) but not only, and events are then dequeued in an infinite loop (named the "event loop") in `main()`. 

This way of programming allows short handling of interrupts in ISR (important because interrupts are disabled during ISR execution), and defer long tasks execution to the event loop.

### Events API

We will use the following example to explain the general Event API:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/gpio.h>
#include <fastarduino/pci.h>
#include <fastarduino/events.h>
#include <fastarduino/time.h>

#define PCI_NUM 2
static constexpr const board::Port BUTTONS_PORT = board::Port::PORT_D;
static constexpr const board::DigitalPin LED = board::DigitalPin::LED;

using namespace events;

using EVENT = Event<uint8_t>;
static constexpr const uint8_t BUTTON_EVENT = Type::USER_EVENT;

// Class handling PCI interrupts and transforming them into events
class EventGenerator
{
public:
	EventGenerator(containers::Queue<EVENT>& event_queue):event_queue_{event_queue}, buttons_{0x00, 0xFF} {}

	void on_pin_change()
	{
		event_queue_.push_(EVENT{BUTTON_EVENT, buttons_.get_PIN()});
	}

private:
	containers::Queue<EVENT>& event_queue_;
	gpio::FastPort<BUTTONS_PORT> buttons_;
};

REGISTER_PCI_ISR_METHOD(PCI_NUM, EventGenerator, &EventGenerator::on_pin_change, board::DigitalPin::D0_PD0)

void blink(uint8_t buttons)
{
	// If no button is pressed, do nothing
	if (!buttons) return;

	gpio::FastPinType<LED>::TYPE led;
	// Buttons are plit in 2 groups of four:
	// - 1st group sets 5 iterations
	// - 2nd group sets 10 iterations
	// Note: we multiply by 2 because one blink iteration means toggling the LED twice
	uint8_t iterations = (buttons & 0x0F ? 5 : 10) * 2;
	// In each group, each buttons define the delay between LED toggles
	// - 1st/5th button: 200ms
	// - 2nd/6th button: 400ms
	// - 3rd/7th button: 800ms
	// - 4th/8th button: 1600ms
	uint16_t delay = (buttons & 0x11 ? 200 : buttons & 0x22 ? 400 : buttons & 0x44 ? 800 : 1600);
	while (iterations--)
	{
		led.toggle();
		time::delay_ms(delay);
	}
}

static const uint8_t EVENT_QUEUE_SIZE = 32;

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Prepare event queue
	EVENT buffer[EVENT_QUEUE_SIZE];
	containers::Queue<EVENT> event_queue{buffer};

	// Create and register event generator
	EventGenerator generator{event_queue};
	interrupt::register_handler(generator);

	// Setup PCI interrupts
	interrupt::PCISignal<BUTTONS_PORT> signal;
	signal.enable_pins_(0xFF);
	signal.enable_();

	// Setup LED pin as output
	gpio::FastPinType<LED>::TYPE led{gpio::PinMode::OUTPUT};

	// Enable interrupts at startup time
	sei();

	// Event Loop
	while (true)
	{
		EVENT event = containers::pull(event_queue);
		if (event.type() == BUTTON_EVENT)
			// Invert levels as 0 means button pushed (and we want 1 instead)
			blink(event.value() ^ 0xFF);
	}
	return 0;
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In this example, 8 buttons are connected to one port and generate Pin Change Interrupts when pressed. The registered 
ISR generates an event for each pin change; the main event loop pulls one event from the events queue and starts a 
blinking sequence of the LED connected to D13 (Arduino UNO); the selected sequence (number of blinks and delay between 
blinks) depends on the pressed button.

First off, you need to include the heeader with FastArduino Events API:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/events.h>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Then we will be using `namepsace events` in our example:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
using namespace events;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Events API defines `template<typename T> class Event<T>` which basically contains 2 properties:
- `type()` which is an `uint8_t` value used to discriminate different event types. FastArduino predefines a few event
types and enables you to define your own types as you need,
- `value()` is a `T` instance which you can use any way you need; each event will transport such a value. If you do not need any additional information on your own events, then you can use `void` for `T`.

In the example, we use events to transport the state of all 8 buttons (as a `uint8_t`):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
using EVENT = Event<uint8_t>;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We also define a specific event type when buttons state changes:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
static constexpr const uint8_t BUTTON_EVENT = Type::USER_EVENT;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note that `Type` is a namespace inside `events` namepsace, which lists FastArduino pre-defined Event types as constants:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
		const uint8_t NO_EVENT = 0;
		const uint8_t WDT_TIMER = 1;
		const uint8_t RTT_TIMER = 2;

		// User-defined events start here (in range [128-255]))
		const uint8_t USER_EVENT = 128;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Numbers from 3 to 127 are reserved for FastArduino future enhancements.

The we define the class that will handle Pin Change Interrupts and generate events:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class EventGenerator
{
public:
	EventGenerator(containers::Queue<EVENT>& event_queue):event_queue_{event_queue}, buttons_{0x00, 0xFF} {}

	void on_pin_change()
	{
		event_queue_.push_(EVENT{BUTTON_EVENT, buttons_.get_PIN()});
	}

private:
	containers::Queue<EVENT>& event_queue_;
	gpio::FastPort<BUTTONS_PORT> buttons_;
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note the use of `containers::Queue<EVENT>` that represents a [ring-buffer](https://en.wikipedia.org/wiki/Circular_buffer)
queue to which events are pushed when the state of buttons are changing, and from which events are pulled from the main 
event loop (as shown later).

The most interesting code is the single line:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
		event_queue_.push_(EVENT{BUTTON_EVENT, buttons_.get_PIN()});
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
where a new `EVENT` is instantiated, its value is the current level of all 8 buttons pins; then the new event is pushed 
to the events queue.

As [usual](@ref interrupts), we have to register this handler to the right PCI ISR:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
REGISTER_PCI_ISR_METHOD(PCI_NUM, EventGenerator, &EventGenerator::on_pin_change, board::DigitalPin::D0_PD0)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Next we define the size of the events queue, which must be a power of 2 (this allows code size and speed optimizations):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
static const uint8_t EVENT_QUEUE_SIZE = 32;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This must be known at compile-time as we do not want dynamic memory allocation in an embedded program.

The actual events queue is instantiated inside `main()`:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	EVENT buffer[EVENT_QUEUE_SIZE];
	containers::Queue<EVENT> event_queue{buffer};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note that this is done in 2 steps:
1. A buffer of the required size is allocated (in the stack for this example); the actual size, in this example, is `64` 
bytes, i.e. 32 times 1 byte (event type) + 1 byte (event value).
2. A `Queue` is instantiated for this buffer; after this step, you should never directly access `buffer` from your code.

Finally, the `main()` function has the infinite event loop:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	while (true)
	{
		EVENT event = containers::pull(event_queue);
		if (event.type() == BUTTON_EVENT)
			// Invert levels as 0 means button pushed (and we want 1 instead)
			blink(event.value() ^ 0xFF);
	}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The call to `containers::pull(event_queue)` is blocking until an event is available in the queue, i.e. has been pushed
by `EventGenerator::on_pin_change()`. While waiting, this call uses `time::yield()` which may put the MCU in sleep mode (see [Power management tutorial later](@ref power)).

Once an event is pulled from the event queue, we check its type and call `blink()` function with the event value, i.e. 
the state of buttons when Pin Change Interrupt has occurred.

The `blink()` function simply loops for the requested number of iterations and blinking delay, and returns only when the 
full blinking sequence is finished. This function is very simple and there is no need to explain it further.

### Event Dispatcher

In the previous example, only one type of event is expected inside the event loop. In more complex applications though,
many more types of events are foreseeable and we may end up with an event loop like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	while (true)
	{
		EVENT event = containers::pull(event_queue);
		switch (event.type())
        {
            case EVENT_TYPE_1:
            // Do something
            break;

            case EVENT_TYPE_2:
            // Do something else
            break;

            case EVENT_TYPE_3:
            // Do yet something else
            break;

            ...
        }
	}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This generally makes code less readable and more difficult to maintain.

FastArduino API supports an *event dispatching* approach, where you define `EventHandler` classes and register 
instances for specific event types:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class MyHandler: public EventHandler<EVENT>
{
public:
	MyHandler() : EventHandler<EVENT>{Type::USER_EVENT} {}
	virtual void on_event(const EVENT& event) override
	{
        // Do something
	}
};

int main()
{
    ...
	// Prepare Handlers
	MyHandler handler;
    ...
	
	// Prepare Dispatcher and register Handlers
	Dispatcher<EVENT> dispatcher;
	dispatcher.insert(handler);
    ...
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With `events::Dispatcher`, you can register one handler for each event type, and then the main event loop
is reduced to the simple code below:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	while (true)
	{
		EVENT event = pull(event_queue);
		dispatcher.dispatch(event);
	}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The right event handler gets automatically called for each pulled event.

Note that `events::EventHandler` uses a `virtual` method, hence this may have an impact on memory size consumed in
Flash (vtables) and also on method call time.

### Scheduler & Jobs

Some types of events are generated by FastArduino features. In particular, watchdog and realtime timer features can be
setup to trigger some time events at given periods. From these events, FastArduino offers a `events::Scheduler` API, 
that allows you to define periodic `events::Job`s.

The next code demonstrates a new way of blinking a LED by using watchdog generated events and associated jobs:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/gpio.h>
#include <fastarduino/events.h>
#include <fastarduino/watchdog.h>
#include <fastarduino/scheduler.h>

using namespace events;
using EVENT = Event<void>;

// Define vectors we need in the example
REGISTER_WATCHDOG_CLOCK_ISR(EVENT)

static const uint32_t PERIOD = 1000;

class LedBlinkerJob: public Job
{
public:
	LedBlinkerJob() : Job{0, PERIOD}, led_{gpio::PinMode::OUTPUT} {}
	virtual void on_schedule(UNUSED uint32_t millis) override
	{
		led_.toggle();
	}
	
private:
	gpio::FastPinType<board::DigitalPin::LED>::TYPE led_;
};

// Define event queue
static const uint8_t EVENT_QUEUE_SIZE = 32;
static EVENT buffer[EVENT_QUEUE_SIZE];
static containers::Queue<EVENT> event_queue{buffer};

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	// Prepare Dispatcher and Handlers
	Dispatcher<EVENT> dispatcher;
	watchdog::Watchdog<EVENT> watchdog{event_queue};
	watchdog.register_watchdog_handler();
	Scheduler<watchdog::Watchdog<EVENT>, EVENT> scheduler{watchdog, Type::WDT_TIMER};
	dispatcher.insert(scheduler);

	LedBlinkerJob job;
	scheduler.schedule(job);
	
	// Start watchdog
	watchdog.begin(watchdog::WatchdogSignal::TimeOut::TO_64ms);
	
	// Event Loop
	while (true)
	{
		EVENT event = pull(event_queue);
		dispatcher.dispatch(event);
	}
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FastArduino `Scheduler` API is defined in its own header, but under the same `events` namespace:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/scheduler.h>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This API is based on FastArduino events management, hence we need to define the `Event` type we need:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
using EVENT = Event<void>;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FastArduino `Scheduler` does not use `Event`'s`value`, hence we can simply use `void`, so that one `EVENT` will be
exactly one byte in size.

Next step consists in registering Watchdog clock ISR:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
REGISTER_WATCHDOG_CLOCK_ISR(EVENT)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We then define a `Job` subclass that will be called back at the required period:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
class LedBlinkerJob: public Job
{
public:
	LedBlinkerJob() : Job{0, PERIOD}, led_{gpio::PinMode::OUTPUT} {}
	virtual void on_schedule(UNUSED uint32_t millis) override
	{
		led_.toggle();
	}
	
private:
	gpio::FastPinType<board::DigitalPin::LED>::TYPE led_;
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
`Job` constructor takes 2 arguments:
- `next`: the delay in ms after which this job will execute for the first time; `0` means this job should execute
immediately
- `period`: the period in ms at which the job shall be re-executed; `0` means this job will execute only once.

In this example, we set the job period to 1000ms, i.e. the actual blink period will be 2s.

The `virtual` method `Job::on_schedule()` will get called when the time to executed has elapsed, not necessarily at the 
exact expected time (the exact time depends on several factors exposed hereafter).

In the example, we then declare the event queue, this time as static memory (not in the stack):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
static const uint8_t EVENT_QUEUE_SIZE = 32;
static EVENT buffer[EVENT_QUEUE_SIZE];
static containers::Queue<EVENT> event_queue{buffer};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In `main()`, we create a `Dispatcher` that will be used in the event loop, a `Watchdog` that
will generate and queue `Type::WDT_TIMER` events on each watchdog timeout, and a `Scheduler` that will
get notified of  `Type::WDT_TIMER` events and will use the created `Watchdog` as a clock, in order to
determine which `Job`s to call.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	Dispatcher<EVENT> dispatcher;
	watchdog::Watchdog<EVENT> watchdog{event_queue};
	watchdog.register_watchdog_handler();
	Scheduler<watchdog::Watchdog<EVENT>, EVENT> scheduler{watchdog, Type::WDT_TIMER};
	dispatcher.insert(scheduler);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note the last code line, which links the `Scheduler` to the `Dispatcher` so it can handle proper events.

Next, we instantiate the job that will make the LED blink, and schedule with `Scheduler`:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	LedBlinkerJob job;
	scheduler.schedule(job);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We can then start the watchdog with any suitable timeout value (the selected value impacts the accuracy of the 
watchdog clock):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	watchdog.begin(watchdog::WatchdogSignal::TimeOut::TO_64ms);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Finally, then event loop is quite usual:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	while (true)
	{
		EVENT event = pull(event_queue);
		dispatcher.dispatch(event);
	}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Jobs scheduling accuracy depends on several factors:
- the accuracy of the `CLOCK` used for `Scheduler`; FastArduino provides two possible clocks (but you may also 
provide your own): `watchdog::Watchdog` and `timer::RTT`.
- the CPU occupation of your main event loop, which may trigger a `Job` long after its expected time.

As mentioned above, `Scheduler` can also work with `timer::RTT`, that would mean only a few changes in the previous 
example, as shown in the next snippet, only showing new lines of code:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/realtime_timer.h>
...
static const uint16_t RTT_EVENT_PERIOD = 1024;
REGISTER_RTT_EVENT_ISR(0, EVENT, RTT_EVENT_PERIOD)
...
void main()
{
    ...
	timer::RTT<board::Timer::TIMER0> rtt;
	rtt.register_rtt_handler();
	timer::RTTEventCallback<EVENT, RTT_EVENT_PERIOD> callback{event_queue};
	interrupt::register_handler(callback);
    ...
	Scheduler<timer::RTT<board::Timer::TIMER0>, EVENT> scheduler{rtt, Type::RTT_TIMER};
    ...
	rtt.begin();
    ...
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Since this code uses a realtime timer, two lines of code instantiate, then start one such timer:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	timer::RTT<board::Timer::TIMER0> rtt;
	rtt.register_rtt_handler();
    ...
	rtt.begin();
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In addition, we use `timer::RTTEventCallback` utility class, which purpose is to generate events from
RTT ticks:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	timer::RTTEventCallback<EVENT, RTT_EVENT_PERIOD> callback{event_queue};
	interrupt::register_handler(callback);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Here we create a callback that will generate an event every `RTT_EVENT_PERIOD` milliseconds.

We also need to register the right ISR for the RTT and the callback:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
REGISTER_RTT_EVENT_ISR(0, EVENT, RTT_EVENT_PERIOD)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Now the scheduler can be instantiated with the RTT instance as its clock and `Type::RTT_TIMER` as
the type of events to listen to:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	Scheduler<timer::RTT<board::Timer::TIMER0>, EVENT> scheduler{rtt, Type::RTT_TIMER};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


@anchor power Advanced: Power Management
----------------------------------------

AVR MCU support various sleep modes that you can use to reduce power consumption of your circuits.

Each sleep mode has distinct characteristics regarding:
- maximum current consumed
- list of wake-up signals (interrupts)
- time to wake up from sleep

FastArduino support for AVR power modes is quite simple, gathered in `namespace power` and consists in
a single class `power::Power` with 3 static methods.

In addition, the available sleep modes for a given MCU are defined in `board::SleepMode` enumerated type.

Here is a simple example:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/gpio.h>
#include <fastarduino/power.h>
#include <fastarduino/watchdog.h>

// Define vectors we need in the example
REGISTER_WATCHDOG_ISR_EMPTY()

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	gpio::FastPinType<board::DigitalPin::LED>::TYPE led{gpio::PinMode::OUTPUT};

	watchdog::WatchdogSignal watchdog;
	watchdog.begin(watchdog::WatchdogSignal::TimeOut::TO_500ms);
	
	while (true)
	{
		led.toggle();
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
You may have recognized it: this is the example presented in the [Watchdog section](@ref watchdog) of this tutorial.

In the main loop, we just toggle the LED pin and go to `board::SleepMode::POWER_DOWN` sleep mode (the mode that 
consumes the least current of all).

The `Power::sleep()` method will return only when the MCU is awakened by an external signal, in this example, the
signal used is the watchdog timeout interrupt (every 500ms). 

Note that the `time::yield()` method just calls `Power::sleep()` (with current default sleep mode) and this method 
is used by other FastArduino API on several occasions whenever these API need to wait for something:
- `watchdog::Watchdog::delay()` while waiting for time to elapse for the given delay
- `timer::RTT::delay()` while waiting for time to elapse for the given delay
- `streams::ostreambuf::pubsync()` while waiting until all stream content has been read by a consumer (e.g. UART)
- `containers::pull()` and `containers::peek()` while waiting for an item to be pushed to a queue
- `devices::rf::NRF24L01` when sending or receving payload

Hence when using any of these API, it is important to select the proper sleep mode, based on your power 
consumption requirements.


@anchor eeprom Advanced: EEPROM
-------------------------------

All AVR MCU include an EEPROM (Electrically Erasable Programmable Read-Only Memory) which can be useful for storing
data that:
- is quite stable but may change from time to time
- is fixed but different for every MCU (for a large series of similar circuits)

Typical examples of use of EEPROM cells include (but are not limited to):
- Circuit unique ID (e.g. used in a network of devices)
- WIFI credentials
- Calibration value (for internal RC clock)
- Sequence of tones frequencies & periods for an alarm melody

In these examples, writing EEPROM cells would happen only once (or a few times), while reading would occur at startup.

### Reading EEPROM content

Often, writing EEPROM can be done directly through an ISP programmer (UNO USB programming does not support EEPROM writing) and only reading is needed in a program. The next example illustrates this situation.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <fastarduino/eeprom.h>
#include <fastarduino/tones.h>
#include <fastarduino/time.h>

// Board-dependent settings
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin OUTPUT = board::PWMPin::D9_PB1_OC1A;

using devices::audio::Tone;
using namespace eeprom;
using GENERATOR = devices::audio::ToneGenerator<NTIMER, OUTPUT>;

struct TonePlay
{
	Tone tone;
	uint16_t ms;
};

// Melody to be played
TonePlay music[] EEMEM =
{
	// Intro
	{Tone::A1, 500},
	{Tone::A1, 500},
	{Tone::A1, 500},
	{Tone::F1, 350},
	{Tone::C2, 150},
	{Tone::A1, 500},
	{Tone::F1, 350},
	{Tone::C2, 150},
	{Tone::A1, 650},

	// Marker for end of melody
	{Tone::END, 0}
};

int main()
{
	sei();
	GENERATOR generator;
	TonePlay* play = music;
	while (true)
	{
		TonePlay tone;
		EEPROM::read(play, tone);
		if (tone.tone == Tone::END)
			break;
		generator.tone(tone.tone, tone.ms);
		++play;
	}
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This example plays a short melody, stored in EEPROM, to a buzzer wired on Arduino UNO D9 pin.
It uses FastArduino `devices::audio::ToneGenerator` support to generate square waves at the required
frequencies, in order to generate a melody intro that Star Wars fan will recognize!

Past the headers inclusion, UNO-specific definitions (timer and output pin) and various `using` directives,
the example defines `struct TonePlay` which embeds a `Tone` (frequency) and a duration, which is then used 
in an array of notes to be played:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
struct TonePlay
{
	Tone tone;
	uint16_t ms;
};

// Melody to be played
TonePlay music[] EEMEM =
{
	// Intro
	{Tone::A1, 500},
	{Tone::A1, 500},
	{Tone::A1, 500},
	{Tone::F1, 350},
	{Tone::C2, 150},
	{Tone::A1, 500},
	{Tone::F1, 350},
	{Tone::C2, 150},
	{Tone::A1, 650},

	// Marker for end of melody
	{Tone::END, 0}
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note that `music` array is defined with `EEMEM` attribute, which tells the compiler that
this array shall be stored in EEPROM and not in Flash or SRAM.

The advantages of storing the melody in EEPROM are:
- it can easily be changed to another melody without any change to the program (flash)
- it does not use Flash memory for storage
- it does not use SRAM memory in runtime (although it might, if entirely read from EEPROM
to SRAM, but why would you do that?)

Note, however, that for some Arduino boards (e.g. UNO), storing a melody to the EEPROM may require using a
specific device, called an ISP programmer. This is beacuse some Arduino bootloaders do not support EEPROM upload.

The important part of the program is the loop where each note is read for EEPROM before being played:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
	TonePlay* play = music;
	while (true)
	{
		TonePlay tone;
		EEPROM::read(play, tone);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In this snippet, `play` is used just as an "index" (an address actually) to the current note
in `music` array. Beware that `play` cannot be used directly in your program because it points nowhere 
actually, really.

The interesting bit here is `EEPROM::read(play, tone);` which uses `play` address as a reference to the next note
in EEPROM, reads the EEPROM content at this location and copies it into `tone`.

At the end of the loop, `play` address gets incremented in order to point to the next note of `music` in EEPROM:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
		++play;
	}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

At this point, you may wonder when the `while (true)` loop exits. Since the program does not in advance what melody
it is going to play, it it not possible to use a `for` loop with an upper boundary as we could do if `music` was
directly in SRAM.

Thus, we use another way to determine the end of the meoldy to play: we use a special `Tone`, `Tone::END`, which is
just a marker of the end of the melody; we check it for every tone read from EEPROM:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
		EEPROM::read(play, tone);
		if (tone.tone == Tone::END)
			break;
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

All this is all good but how do we upload the melody to the EEPROM the first time?

FastArduino make system takes care of this:

1. `make build` not only generates a `.hex` file for code upload to the flash, it also generates a `.eep` file
with all content to be uploaded to EEPROM: this is based on all variables defined with attribute `EEMEM` in your 
program.
2. `make eeprom` uses the above `.eep` file an upload it to the MCU to program EEPROM content.

IMPORTANT: note that, as relevant as this example can be for this tutorial, you do not need to write 
all this code, with FastArduino, to play a sequence of tones: you may use `TonePlayer` class
that odes it all for you, and can handle arrays stored in SRAM, EEPROM or Flash memory.


### Writing content to EEPROM

There are also programs that may need to store content to EEPROM during their execution. This is possible
with FastArduino EEPROM support.

Please do note, however, that the number of writes an EEPROM can support is rather limited (100'000 as per 
AVR MCU datasheet), hence you should refrain from writing too many times to the EEPROM.

Also, do note that writing a byte to the EEPROM is not the same as writing a byte to SRAM, this is much slower 
(typically between 1.8ms and 3.4ms).

The following example demonstrates... TODO

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
TODO 2nd example write config to EEPROM
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}

