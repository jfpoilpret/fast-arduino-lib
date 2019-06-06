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
6. [SPI devices example](@ref spi)
7. [I2C devices example](@ref i2c)
8. [software UART](@ref softuart)

Supported devices (not yet documented):
1. SPI
2. I2C
3. Other devices: sonar, servo, SIPO


@anchor gpiotime Basics: gpio & time
------------------------------------

### Blink example ###

Here is a first example of a FastArduino based program:

@includelineno fastarduino/gpiotime_1_blink.cpp

This example can be broken down into several parts:
@dontinclude fastarduino/gpiotime_1_blink.cpp
@line include
@line include
This includes the necessary API from FastArduino; in this example, we just use `gpio.h` (all API for digital input/output) and `time.h` (API for busy loop delays).

@skip main
@until sei
The next part defines the standard `main()` function as the entry point of the program; first actions in the `main()` should always be to call `board::init()` (important initialization for some specific boards), then sooner or later call `sei()` to enable interrupts again, as interrupts are disabled when `main()` is initially called.

@skipline FastPinType
This line declares and initializes a digital pin variable named `led` as output for the board's LED (i.e. `D13` on Arduino boards).

`board::DigitalPin` is a strong enum class that defines all digital pins **for the current target**, that target must be defined in the compiler command-line.

The actual type of `led` is `gpio::FastPin<board::Port:PORT_B, 5>` which means "the pin number 5 within port B"; since this type is not easy to declare when you only know the number of the pin you need, `gpio::FastPinType<board::DigitalPin::LED>::TYPE` is used instead, as it maps directly to the right type.

`led` is initialized as an output pin, its initial level is `false` (i.e. GND) by default.

@skip while
@until }
Then the program enters an endless loop in which, at every iteration:

1. It toggles the level of `led` pin (D13 on Arduino) from GND to Vcc or from Vcc to GND
2. It delays execution (i.e. it "waits") for 500 milliseconds

This part of the program simply makes your Arduino LED blink at 1Hz frequency!

The last part below is never executed (because of the endless loop above) but is necessary to make the compiler happy, as `main()` shall return a value:
@skip return
@until }

Congratulation! We have just studied the typical "blink" program.

At this point, it is interesting to compare our first program with the equivalent with standard Arduino API:

@includelineno arduino/gpiotime_1_blink.ino

Granted that the latter code seems simpler to write! However, it is also simpler to write it wrong, build and upload it to Arduino and only see it not working:

@includelineno arduino/gpiotime_2_blink_wrong.ino

The problem here is that Arduino API accept a simple number when they need a pin, hence it is perfectly possible to pass them the number of a pin that does not exist, as in the faulty code above: this code will compile and upload properly to an Arduino UNO, however it will not work, because pin 53 does not exist!

This problem cannot occur with FastArduino as the available pins are stored in a strong enum and it becomes impossible to select a pin that does not exist for the board we target!

Now, what is really interesting in comparing both working code examples is the size of the built program (measured with UNO as a target, FastArduino project built with AVR GCC 7.2.0, Arduino API project built with Arduino CLI 0.3.6-alpha.preview):

@snippetdoc tables.txt gpiotime_1_blink

As you probably know, Atmel AVR MCU (and Arduino boards that embed them) are much constrained in code and data size, hence we could say that "every byte counts". In the table ablove, one easily sees that Arduino API comes cluttered with lots of code and data, even if you don't need it; on the other hand, FastArduino is highly optimized and will produce code only for what you **do** use.

### LED Chaser example ###

Now `gpio.h` has more API than just `gpio::FastPin` and `gpio::FastPinType`; it also includes `gpio::FastPort` and `gpio::FastMaskedPort` that allow to manipulate several pins at a time, as long as these pis belong to the same Port of the MCU. This allows size and speed optimizations when having to deal with a group of related pins, e.g. if you want to implement a LED chaser project.

With FastArduino, here is a program showing how you could implement a simple 8 LED chaser on UNO:

@includelineno fastarduino/gpiotime_2_led_chaser.cpp

In this example, we selected all pins of the same port to connect the 8 LEDs of our chaser. Concretely on UNO, this is port D, which pins are D0-D7.

We thus declare and initialize `leds` as a `gpio::FastPort<board::Port::PORT_D>` port, with all pins as output (`0xFF`), with initial level to GND (`0x00`, all LEDs off).

Then, we will keep track of the current lit LED through `pattern` byte which each bit represents actually one LED; `pattern` is initialized with `0x01` i.e. D0 should be the first LED to be ON.

In the endless loop that follows, we perform the following actions:

1. Set all pins values at once to the current value of `pattern`
2. Delay execution for 250ms
3. Shift the only 1 bit of `pattern` left; note that after 8 shifts, `pattern` will become `0`, hence we need to check against this condition to reset `pattern` to its initial state.

This should be rather straightforward to understand if you know C or C++.

Here is an equivalent example with Arduino API:

@includelineno arduino/gpiotime_3_led_chaser.ino

We see, with Arduino API, that we have to deal with each pin individually, which makes the program source code longer and not necessarily easier to understand.

Here is a quick comparison of the sizes for both programs:

@snippetdoc tables.txt gpiotime_2_led_chaser


@anchor uartflash Basics: UART & flash
--------------------------------------

### Simple Serial Output example ###

Although not often necessary in many finished programs, `UART` (for serial communication interface) is often very useful for debugging a program while it is being developed; this is why `UART` is presented now.

Here is a first simple program showing how to display, with FastArduino API, a simple string to the serial output (for UNO, this is connected to USB):

@includelineno fastarduino/uartflash_1_helloworld.cpp

As usual, at first we need to include the proper header (`uart.h`) to use its API.

Then, we define a buffer that will be used by the `UART` API to transmit characters to your PC through USB. You may find it cumbersome to do it yourself but it brings a huge advantage: you are the one to decide of the buffer size, whereas in Arduino API, you have no choice. Here, we consider 64 bits to be big enough to store characters that will be transmitted to the PC. How `UART` is using this buffer is not important to you though.

Then we *register an ISR* necessary for transmissions to take place; this is done by the `REGISTER_UATX_ISR(0)` macro. Explicit ISR registration is one important design choice of FastArduino: **you** decide which ISR should be registered to do what. This may again seem cumbersome but once again this gives you the benefit to decie what you need, hence build your application the way you want it.

The code that follows instantiates a `uart::hard::UATX` object that is using `board::USART::USART0` (the only one available on UNO) and based on the previously created buffer. Note that `UATX` class is in charge of **only** transmitting characters, not receiving. Other classes exist for only receiving (`UARX`), or for doing both (`UART`).

Once created, we can set `uart` ready for transmission, at serial speed of 115200 bps.

Next step consists in extracting, from `uart`, a `streams::ostream` that will allow us to send characters or strings to USB:

    out.write("Hello, World!\n");

The last important instruction, `out.flush()`, waits for all characters to be transmitted before leaving the program.

Do note the specific `main` declaration line before its definition: `int main() __attribute__((OS_main));`. This helps the compiler perform some optimization on this function, and may avoid generating several dozens code instructions in some circumstances. In some situations though, this may increase code size by a few bytes; for your own programs, you would have to compile with and without this line if you want to find what is the best for you.

Here is the equivalent code with Arduino API:

@includelineno arduino/uartflash_1_helloworld.ino

Of course, we can see here that the code looks simpler, although one may wonder why we need to define a `loop()` function that does nothing.

Now let's compare the size of both:

@snippetdoc tables.txt uartflash_1_helloworld

The data size is big because the buffer used by `Serial` has a hard-coded size (you cannot change it without modifying and recompiling Arduino API). Moreover, when using `Serial`, 2 buffers are created, one for input and one for output, even though you may only need the latter one!

Now let's take a look at the 90 bytes of data used in the FastArduino version of this example, how are they broken down?
| Source            | data size   |
|-------------------|-------------|
| `output_buffer`   | 64 bytes    |
| `UATX` ISR        | 2 bytes     |
| `UATX` vtable     | 8 bytes     |
| "Hello, World!\n" | 16 bytes    |
| **TOTAL**         | 90 bytes    |

*vtable* is specific data created by C++ compiler for classes with `virtual` methods: every time you use virtual methods in classes, this will add more data size, this is why FastArduino avoids `virtual` as much as possible.

As you can see in the table above, the constant string `"Hello, World!\n"` occupies 16 bytes of data (i.e. AVR SRAM) in addition to 16 bytes of Flash (as it is part of the program and must be stored permanently). If your program deals with a lot of constant strings like this, you may quickly meet a memory problem with SRAM usage. This is why it is more effective to keep these strings exclusively in Flash (you have no choice) but load them to SRAM only when they are needed, i.e. when they get printed to `UATX` as in the sample code.

How do we change our program so that this string is only stored in Flash? We can use FastArduino `flash` API for that, by changing only one line of code:

    out.write(F("Hello, World!\n"));

Note the use of `F()` macro here: this makes the string reside in Flash only, and then it is being read from Flash "on the fly" by `out.write()` method; the latter method is overloaded for usual C-strings (initial example) and for C-strings stored in Flash only.

We can compare the impact on sizes:

@snippetdoc tables.txt uartflash_2_helloworld

Although a bit more code has been added (the code to read the string from Flash into SRAM on the fly), we see 16 bytes have been removed from data, this is the size of the string constant.

You may wonder why `"Hello, World!\n"` occupies 16 bytes, although it should use only 15 bytes (if we account for the terminating `'\0'` character); this is because the string is stored in Flash and Flash is word-addressable, not byte-addressable on AVR.

Note that Flash can also be used to store other read-only data that you may want to access at runtime at specific times, i.e. data you do not want to be stored permanently on SRAM during all execution of your program.

The following example shows how to:
- define, in your source code, read-only data that shall be stored in Flash memory
- read that data when you need it

@includelineno fastarduino/uartflash_3_flashread.cpp

### Formatted Output example ###

Compared to Arduino API, FastArduino brings formatted streams as can be [found in standard C++](https://en.wikipedia.org/wiki/Input/output_(C%2B%2B)); although more verbose than usual C `printf()` function, formatted streams allow compile-time safety.

Here is an example that prints formatted data to USB:

@includelineno fastarduino/uartflash_4_ostream.cpp

Here, we still use `uart.out()`, but here we use its "insertion operator" `<<`.

If you are used to programming with C++ for more usual systems (e.g. Linux), then you will immediately recognize [`std::ostream` API](http://www.cplusplus.com/reference/ostream/ostream/operator%3C%3C/) which FastArduino library tries to implement with some level of fidelity.

You can also find more details in `streams` namespace documentation.

Here is the equivalent code with Arduino API:

@includelineno arduino/uartflash_4_ostream.ino

Once again, we can compare the size of both:

@snippetdoc tables.txt uartflash_4_ostream

### Serial Input example ###

FastArduino also implements input streams connected to serial output; here is a simple example:

@includelineno fastarduino/uartflash_5_input.cpp

Note the similarities between this example and UATX example above for all the setup parts.
The main differences are:
- use `UARX` type instead of `UATX`
- `REGISTER_UARX_ISR()` instead of `REGISTER_UATX_ISR()` macro for ISR registration
- use `istream` instead of `ostream` and `uart.in()` instead of `uart.out()`

Then `UARX` mainly offers one method, `get()`, which returns the next character serially received and buffered; if the input buffer is currently empty, then `get()` will block.

The example uses 2 flavors of this `istream` method:
- `get(char&)`: this method **blocks** until one character is available on serial input.
- `get(char*, size_t, char)`: this blocks until a complete string (terminated by a specified delimiter character, '\\n' by default) gets read on serial input and fills the given buffer parameter with that string content.

Note that these 2 methods use `time::yield()` while waiting; this may be linked to `power` management. Please take a look at the documentation for this API for further details.

### Formatted Input example ###

Similar to output, input streams supports formatted input, as can be found in standard C++; once again, formatted input streams allow compile-time safety.

The following example uses formatted input to read values from USB:

@includelineno fastarduino/uartflash_6_istream.cpp

Here, we still use `uart.in()` but we use its "extraction operator" `>>`. All extractions are blocking and will not return until the required type can be read from the buffer.

If you are used to programming with C++ for more usual systems (e.g. Linux), then you will immediately recognize [`std::istream` API](http://www.cplusplus.com/reference/istream/istream/operator%3E%3E/) which FastArduino library tries to implement with some level of fidelity.

You can also find more details in `streams` namespace documentation.

We have already seen `UATX` and `UARX` as classes for sending, resp. receiving, data through serial. There is also `UARX` which combines both.

As you know, the number of physical (hardware) UART available on an MCU target is limited, some targets (ATtiny) don't even have any hardware UART at all. For this reason, if you need extra UART featurs to connect to some devices, you can use software UART API, documented in namespace `serial::soft`. As this is more complicated to use, it is not part of this basic tutorial, but will be addressed later on.


@anchor analoginput Basics: analog input
----------------------------------------

Here is a simple example using analog input API to read a value from some sensor (thermistor, potentiometer, whatever you like) and lights a LED if the read value is above some threshold:

@includelineno fastarduino/analoginput_1_input.cpp

This example is an adaptation of the [first GPIO example](@ref gpio) of this tutorial.

The first change consists in including the necessary header:
@dontinclude fastarduino/analoginput_1_input.cpp
@line include

Then we have the definition of the `sensor` variable:
@skipline AnalogInput

Here we instantiate `AnalogInput` for analog pin `A0` (on Arduino UNO).

In the infinite loop, we then get the current analog value of `sensor` and compare it to `THRESHOLD` constant:
@skipline sensor.sample

By default, sample values are on 10 bits (0..1023) represented as `uint16_t`.

If you don't need such precision, you can define `sensor` differently:
@snippet tuto_snippets.cpp analoginput_8bits
Note the two additional template arguments provided to `AnalogInput<...>`:
- the first added argument `board::AnalogReference::AVCC`, although seldom changed, may become important when you create your own boards from MCU chips; you can further read API documentation if you need more information about it
- the second added argument is the type of returned samples, either `uint16_t` (default value) or `uint8_t`. The type determines the samples precision:
    - `uint8_t`: 8 bits (samples between 0 and 255)
    - `uint16_t`: 10 bits (samples between 0 and 1023)

Now let's compare the first example with the equivalent Arduino core API program:

@includelineno arduino/analoginput_1_input.ino

As usual, we compare the size of both:

@snippetdoc tables.txt analoginput_1_input

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

@includelineno fastarduino/timer_1_ctc.cpp

This example looks much more complex than all previous examples but it is straightforward to understand once explained part after part.

@dontinclude fastarduino/timer_1_ctc.cpp
@line include
@line include
In addition to GPIO, we include the header containing all Timer API.

For this example, we use Arduino UNO, which MCU (ATmega328P) includes 3 timers (named respectively `Timer0`, `Timer1`, `Timer2` in its datasheet), we use Timer1 which is 16-bits in size:
@skip NTIMER
@until PERIOD_US

Although not needed, it is a good practice to define a `const`, named `NTIMER` in this snippet, valued with the real timer we intend to use.
Then we define 2 new type aliases, `CALCULATOR` and `TIMER` that will help us type less code (this is common recommended practice when using C++ templates heavily in programs):
- `CALCULATOR` is the type of a class which provides `static` utility methods that will help us configure the timer we have selected; do note that, since all timers are different, `CALCULATOR` is specific to one timer only; hence if our program was using 2 distinct timers, we would have to define two distinct calculator type aliases, one for each timer.
- `TIMER` is the type of the class that embed all timer API for the specific timer we have selected.

Finally we define `PERIOD_US` the period, in microseconds, at which we want the LED to blink. Please note that this is in fact half the actual period, because this is the time at which we will toggle the LED light.

@line PRESCALER
@line COUNTER
The above snippet defines constant settings, computed by `CALCULATOR` utility class, that we will later use to initialize our timer:
- `PRESCALER` is the **optimum** prescaler value that we can use for our timer in order to be able to count up to the requested period, i.e. 1 second; the type of prescaler is an `enum` that depends on each timer (because the list of available prescaler values differ from one timer to another). The prescaler defines the number by which the MCU clock frequency will be divided to provide the pulses used to increment the timer. We don't need to know this value or fix it ourselves because `CALCULATOR::CTC_prescaler` calculates the best choice for us.
- `COUNTER` is the maximum counter value that the timer can reach until 1 second has ellapsed; its type is based on the timer we have selected (i.e. `Timer1` => 16 bits => `uint16_t`), but we don't need to fix this type ourselves because it depends on the timer we have selected.

Note that, although we know in advance which timer we use, we always avoid declaring direct types (such as `uint16_t`) in order to facilate a potential change to another timer in the future, without having to change several code locations. 

@skip class Handler
@until };
Here we define the class which implements the code in charge of blinking the LED every time the timer has reached its maximum value, i.e. every second.
There is nothing special to explain here, except that the method `on_timer()` is a *callback function* which will get called asynchronously (from interrupt handlers) when the timer reaches its max.

Since timers generate interruptions, we need to "attach" our handler code above to the suitable interruption, this is done through the following line of code:
@skipline REGISTER
`REGISTER_TIMER_COMPARE_ISR_METHOD` is a macro that will generate extra code (code you do not need, nor want, to see) to declare the Interrupt Service Routine (*ISR*) attached to the proper interruption of our selected timer; it takes 3 arguments:
- `1` is the timer number (`0`, `1` or `2` on UNO)
- `Handler` is the class that contains the code to be called when the interrupt occurs
- `&Handler::on_timer` is the Pointer to Member Function (often abbreviated *PTMF* by usual C++ developers) telling which method from `Handler` shall be called back when the interrupt occurs
In FastArduino, interrupt handling follows some patterns that are further described [here](@ref interrupts) and won't be developed in detail now.

Now we can finally start writing the code of the `main()` function:
@skip main()
@until register_handler
Past the usual initialization stuff, this code performs an important task regarding interrupt handling: it creates `handler`, an instance of the `Handler` class that has been defined before as the class to handle interrupts for the selected timer, and then it **registers** this handler instance with FastArduino. Now we are sure that interrupts for our timer will call `handler.on_timer()`.

The last part of the code creates and starts the timer we need in our program:
@skipline timer
@until }
`timer` is the instance of `timer::Timer` API for `board::Timer::TIMER1`; upon instantiation, it is passed the timer mode to use, the previously calculated clock prescaler, and the interrupt we want to enable.

Here we use *CTC* mode (Clear Timer on Compare); in this mode the counter is incremented until it reaches a maximum value, then it triggers an interrupt and it clears the counter value back to zero and starts counting up again.

To ensure that our handler to get called back when the timer reaches 1 second, we set `timer::TimerInterrupt::OUTPUT_COMPARE_A`, which enables the proper interrupt on this timer: when the counter is reached, an interrupt will occur, the properly registered ISR will be called, and in turn it will call our handler.

Then `timer.begin()` activates the timer with the maximum counter value, that was calculated initially in the program. This value, along with `PRESCALER`, has been calculated in order for `timer` to generate an interrupt (i.e. call `handler.on_timer()`) every second.

Note the infinite loop `while (true);` at the end of `main()`: without it the program would terminate immediately, giving no chance to our timer and handler to operate as expected. What is interesting to see here is that the main code does not do anything besides looping forever: all actual stuff happens asynchronously behind the scenes!

I would have liked to perform a size comparison with Arduino API, but unfortunately, the basic Arduino API does not provide an equivalent way to directly access a timer, hence we cannot produce the equivalent code here. Anyway, here is the size for the example above:

@snippetdoc tables.txt timer_1_ctc


@anchor rtt Basics: real-time timer 
-----------------------------------

A real-time timer is primarily a device that tracks time in standard measurements (ms, us).

It may be used in various situations such as:
- delay program execution for some us or ms
- capture the duration of some event with good accuracy
- implement timeouts in programs waiting for an event to occur
- generate periodic events

The simple example that follows illustrates the first use case:

@includelineno fastarduino/rtt_1_rtt.cpp

This example looks much like the first blinking example in this tutorial, with a few changes.

First off, as usual the neceaary header file is included:
@dontinclude fastarduino/rtt_1_rtt.cpp
@skipline realtime_timer.h

Then we need to register an ISR for the RTT feature to work properly:
@skipline REGISTER

Then, in `main()`, after the usual initialization stuff, we create a real-time timer instance, based on AVR UNO Timer0 (8-bits timer), and start it counting time.
@skip rtt
@until begin()

Finally, we have the usual loop, toogling the LED, and then delay for 10s, using the RTT API: 
@skipline delay

Let's examine the size of this program and compare it with the first example of this tutorial, which used `time::delay_ms()`:

@snippetdoc tables.txt rtt_1_rtt

As you can see, code and data size is higher here, so what is the point of using `RTT::delay()` instead of `time::delay_ms()`? The answer is **power consumption**:
- `time::delay_ms` is a busy loop which requires the MCU to be running during the whole delay, hence consuming "active supply current" (about 15mA for an ATmega328P at 16MHz)
- `RTT::delay()` will set the MCU to pre-defined sleep mode and will still continue to operate well under most available sleep modes (this depends on which timer gets used, refer to [AVR datasheet](http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-42735-8-bit-AVR-Microcontroller-ATmega328-328P_Datasheet.pdf) for further details); this will alow reduction of supply current, hence power consumption. Current supply will be reduced more or less dramatically according to the selected sleep mode.

Another practical use of RTT is to measure the elapsed time between two events. For instance it can be used with an ultrasonic ranging device to measure the duration of an ultrasound wave to do a roundript from the device to an obstacle, then calculate the actual distance in mm. The following snippet shows how it could look like for an HC-SR04 sensor:
@snippet tuto_snippets.cpp rtt_hcsr04
Note that this snippet is just an example and is not usable as is: it does not include a timeout mechanism to avoid waiting the echo signal forever (which can happen if the ultrasonic wave does not encounter an obstacle within its possible range, i.e. 4 meters). Also, this approach could be improved by making it interrupt-driven (i.e. having interrupts generated when the `echo_pin` changes state).

Actually, if you want a complete implementation of HC-SR04 ultrasonic ranging device, then you should take a look at FastArduino provided API in namespace `devices::sonar`.

Another interesting use of RTT is to perform some periodic actions. FastArduino implements an events handling mechanism that can be connected to an RTT in order to deliver periodic events. This mechanism is [described later](@ref events) in this tutorial.


@anchor pwm Basics: PWM 
-----------------------

PWM (*Pulse Width modulation*) is a technique that can be used to simulate generation of an analog voltage level through a purely digital output. This is done by varying the *duty cycle* of a rectangular pulse wave, i.e. the ratio of "on" time over the wave period.

PWM is implemented by MCU through timers.

FastArduino includes special support for PWM. The following example demonstrates PWM to increase then decrease the light emitted by a LED:

@includelineno fastarduino/pwm_1_pwm.cpp

The program starts by including the header for PWM API; this will automatically include the timer API header too.

Then a timer is selected for PWM (note that the choice of a timer imposes the choice of possible pins) and a prescaler value computed for it, based on the PWM frequency we want to use, 450Hz, which is generally good enough for most use cases (dimming a LED, rotating a DC motor...):
@dontinclude fastarduino/pwm_1_pwm.cpp
@skip NTIMER
@until PRESCALER

Then we define the pin that will be connected to the LED and the PWMOutput type for this pin:
@line LED
@line LED_PWM
Note that `board::PWMPin` enum limits the pins to PWM-enabled pins; also note the pin name `D6_PD6_OC0A` includes useful information:
- this is pin `D6` on Arduino UNO
- this pin is on `PD6` i.e. Port D bit #6
- this pin is connectable to `OC0A` i.e. Timer 0 COM A

Then, in `main()`, after the usual initialization code, we initialize and start the timer:
@skip timer
@until begin
Notice we use the *Fast PWM* mode here, but we might as well use *Phase Correct PWM* mode.

Next we connect the LED pin to the timer:
@line LED_PWM

In the main loop, we have 2 consecutive loops, the first increases the light, the second decreases it. Both loops vary the duty cycle between its limits:
@skip for
@until }
Note the use of `LED_PWM::TYPE`, which depends on the timer selected (8 or 16 bits), and `LED_PWM::MAX` which provides the maximum value usable for the duty cycle, i.e. the value mapping to 100% duty cycle. Pay attention to the fact that `LED_PWM::TYPE` is unsigned, this explains the 2nd loop:
@skip for
@until }
Here, we shall not use `duty >= 0` as the `for` condition, because that condition would be always `true`, hence the loop would be infinite.

Now let's compare this example with the Arduino API equivalent:

@includelineno arduino/pwm_1_pwm.ino

Nothing special to comment here, except:
- `duty` values are always limited to 255 even though some PWM pins are attached to a 16-bits timer
- you cannot choose the PWM mode you want to use, it is fixed by Arduino API and varies depending on which timer is used (e.g. for Arduino UNO, Timer0 uses Fast PWM, whereas Time1 and Timer2 use Phase Correct PWM mode)
- you cannot choose the PWM frequency, this is imposed to you by Arduino API and varies depending on which timer is used
- you may pass any pin value to `analogWrite()` and the sketch will still compile and upload but the sketch will not work

Comparing sizes once again shows big differences:

@snippetdoc tables.txt pwm_1_pwm


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
@snippet tuto_snippets.cpp utils_swap_bytes

2. `utils::bcd_to_binary`: this function is useful when you use a sensor device that provides values coded as *BCD* (binary-coded decimal), i.e. where each half-byte (*nibble*) contains the value of one digit (i.e. `0` to `9`), thus holding a range of values from `0` to `99`. Many RTC devices use BCD representation for time. In order to performa calculation on BCD values, you need to first convert them to binary. The opposite function is also provided as `utils::binary_to_bcd`. The following example is an excerpt of `ds1307.h` provided by FastArduino, where each datetime field (seconds, minutes, hours...) have to be converted from BCD to binary:
@snippet tuto_snippets.cpp utils_bcd_to_binary

### Conversion utilities examples

Device sensors measure some physical quantity and generally provide you with some integer value that somehow maps to the physical value. hence to make use of the raw value provided by a sensor, you need to convert it to some more meaningful value that you can understand and operate upon.

Or conversely, you may just need to compare the physical value agains some thresholds (e.g. check the gyroscopic speed according to some axis is less than 10°/s), and perform some action when this is not true. In this situation, you don't really need to convert the raw sensor value into a physical quantity to compare to the physical threshold, but rather convert (once only) the physical threshold into the corresponding raw value (a constant in your program) and then only compare raw values, which is:
- more performant (no conversion needed before comparison)
- more size efficient (conversion of threshold can be done at compile time, hence no code is generated for it)

FastArduino utilities provide several conversion methods between raw and physical quantities, according to raw and physical ranges (known for each sensor), and unit prefix (e.g. kilo, mega, giga, centi, milli...). These methods are `constexpr`, which means that, when provided with constant arguments, they will be evaluated at compile-time and return a value that is itself stored as a constant.

1. `utils::map_physical_to_raw`: although it may seem complicated by its list of arguments, this function is actually pretty simple, as demonstrated in the snippet hereafter:
@snippet tuto_snippets.cpp utils_map_physical_to_raw
In this example, we convert the acceleration value 500mg (g is *9.81 m/s/s*) to the equivalent raw value as produced by an MPU-6050 accelerometer, using *+/-2g* range (`2` is the max physical value we can get with this device using this range) where this raw value is stored on `15 bits` (+1 bit for the sign), i.e. `32767` is the raw value returned by the device when the measured acceleration is `+2g`.

2. `utils::map_raw_to_physical`: this method does the exact opposite of `utils::map_physical_to_raw` with the same parameters, reversed:
@snippet tuto_snippets.cpp utils_map_raw_to_physical
In this example, we convert `raw` which is returned by the MPU-6050 gyroscope, using range *+/-250°/s* with `15 bits` precision (+1 bit for the sign), i.e. `32767` is the raw value returned by the device when the measured rotation speed is `+250°/s`. The calculated value is returned in *c°/s* (centi-degrees per second).

In addition to these functions, FastArduino utilities also include the more common `utils::map` and `utils::constrain` which work like their Arduino API equivalent `map()` and `constrain()`.


@anchor watchdog Advanced: Watchdog
-----------------------------------

In general, a watchdog is a device (or part of a device) that is used to frequently check that a system is not hanging. AVR MCU include such a device and this can be programmed to other purposes than checking the system is alive, e.g. as a simple timer with low-power consumption. This is in that purpose that FastArduino defines a specific Watchdog API.

FastArduino defines 2 watchdog classes. The first one, `WatchdogSignal` allows you to generate watchdog timer interrupts at a given period.
The following example is yet another way to blink a LED:

@includelineno fastarduino/watchdog_1_blink.cpp

In this example, we use `watchdog` API but also `power` API in order to reduce power-consumption.

As we use `WatchdogSignal`, we need to register an ISR, however we do not need any callback, hence we just register an empty ISR.
@dontinclude fastarduino/watchdog_1_blink.cpp
@skipline REGISTER

Then we have the usual `main()` function which, after defining `led` output pin, starts the watchdog timer:
@skip WatchdogSignal
@until begin
Here we use a 500ms timeout period, which means our sleeping code will be awakened every 500ms.

The infinite loop just toggles `led` pin level and goes to sleep:
@skip while
@until }
As explained [later](@ref power), `power::Power::sleep(board::SleepMode::POWER_DOWN)` just puts the MCU to sleep until some external interrupts wake it up; in our example, that interrupt is the watchdog timeout interrupt. 

The size of this example is not much bigger than the first example of this tutorial:

@snippetdoc tables.txt watchdog_1_blink

FastArduino defines another class, `Watchdog`. This class allows to use the AVR watchdog timer as a clock for events generation. This will be demonstrated when [events scheduling](@ref events) is described.


@anchor interrupts Advanced: Interrupts
---------------------------------------

### Introduction to AVR interrupts

Many AVR MCU features are based upon interrupts, so that the MCU can efficiently react to events such as timer overflow, pin level change, without having to poll for these conditions. Using AVR interrupts makes your program responsive to events relevant to you. This also allows taking advantage of power sleep modes thus reducing energy consumption while the MCU has nothing to do other than wait for events to occur.

Each AVR MCU has a predefined list of interrupts sources, each individually activable, and each with an associated *vector* which is the address of an Interrupt Service Routine (**ISR**), executed whenever the matching interrupt occurs. Each ISR is imposed a specific name.

Among common interrupt vectors, you will find for example:
- `TIMERn_OVF_vect`: triggered when Timer *n* overflows
- `INTn_vect`: triggered when input pin *INTn* changes level
- `UARTn_RX_vect`: triggered when a new character has been received on serial receiver `UARTn`
- ...

### AVR interrupts handling in FastArduino

In FastArduino, ISR are created upon your explicit request, FastArduino will never add an ISR without your consent. FastArduino calls this ISR creation a **registration**.

ISR registration is performed through macros provided by FastArduino. There are essentially 4 flavours of registration macros:
1. API-specific registration: in this flavour, a FastArduino feature requires to directly be linked to an ISR, through a dedicated macro and a specific registration method (either implicitly called by constructor or explicitly through a specific method). In the previous examples of this tutorial, you have already encountered `REGISTER_UATX_ISR()`, `REGISTER_UARX_ISR()`, `REGISTER_UART_ISR()` and `REGISTER_RTT_ISR()`.
All macros in this flavour follow the same naming scheme: `REGISTER_XXX_ISR`, where *XXX* is the feature handled.
2. Empty ISR registration: in this flavour, you activate an interrupt but do not want any callback for it, but then you have to define an empty ISR for it; this empty ISR will not increase code size of your program. You may wonder why you would want to enable an interrupt but do nothing when it occurs, in fact this is often used to make the MCU sleep (low power consumption) and wake it once an interrupt occurs. You have already seen such usage in a previous example, where `REGISTER_WATCHDOG_ISR_EMPTY()` was used.
3. Method callback registration: with this flavour, you activate an interrupt and want a specific method of a given class to be called back when the interrupt occurs; in this flavour, a second step is required inside your code: you need to register an instance of the class that was registered. In this tutorial, previous examples used this approach with `REGISTER_TIMER_COMPARE_ISR_METHOD()` macro and `interrupt::register_handler(handler);` instance registration in `main()`. This is probably the most useful approach as it allows to pass an implicit context (`this` class instance) to the callback.
4. Function callback registration: with this flavour, you can register one of your functions (global or static) as a callback of an ISR. This approach does not require an extra registration step. This is not used as often as the Method callback registration flavour above.

Whenever method callback is needed (typically for flavours 1 and 3 above), then a second registration step is needed.

All FastArduino API respects some guidelines for naming ISR registration macros. All macros are in one of the following formats:
- `REGISTER_XXX_ISR()` for API-specific registration
- `REGISTER_XXX_ISR_EMPTY()` for empty ISR
- `REGISTER_XXX_ISR_CALLBACK()` for method callback
- `REGISTER_XXX_ISR_FUNCTION()` for function callback

Here is a table showing all FastArduino macros to register ISR (*Name* is to be replaced in macro name `REGISTER_NAME_ISR`, `REGISTER_NAME_ISR_EMPTY`, `REGISTER_NAME_ISR_CALLBACK` or `REGISTER_NAME_ISR_FUNCTION`):
| Header            | Name                      | Flavours | Comments                                                           |
|-------------------|---------------------------|----------|--------------------------------------------------------------------|
| `eeprom.h`        | `EEPROM`                  | 1,3,4    | Called when asynchronous EEPROM write is finished.                 |
| `int.h`           | `INT`                     | 2,3,4    | Called when an INT pin changes level.                              |
| `pci.h`           | `PCI`                     | 2,3,4    | Called when a PCINT pin changes level.                             |
| `pulse_timer.h`   | `PULSE_TIMER8_A`          | 1        | Called when a PulseTimer8 overflows or equals OCRA.                |
| `pulse_timer.h`   | `PULSE_TIMER8_B`          | 1        | Called when a PulseTimer8 overflows or equals OCRB.                |
| `pulse_timer.h`   | `PULSE_TIMER8_AB`         | 1        | Called when a PulseTimer8 overflows or equals OCRA or OCRB.        |
| `realtime_timer.h`| `RTT`                     | 1,3,4    | Called when RTT timer has one more millisecond elapsed.            |
| `realtime_timer.h`| `RTT_EVENT`               | 1        | Same as above, and trigger RTTEventCallback.                       |
| `soft_uart.h`     | `UART_PCI`                | 1        | Called when a start bit is received on a PCINT pin linked to UATX. |
| `soft_uart.h`     | `UART_INT`                | 1        | Called when a start bit is received on an INT pin linked to UATX.  |
| `timer.h`         | `COMPARE`                 | 2,3,4    | Called when a Timer counter reaches OCRA.                          |
| `timer.h`         | `OVERFLOW`                | 2,3,4    | Called when a Timer counter overflows.                             |
| `timer.h`         | `CAPTURE`                 | 2,3,4    | Called when a Timer counter gets captured (when ICP level changes).|
| `uart.h`          | `UATX`                    | 1        | Called when one character is finished transmitted on UATX.         |
| `uart.h`          | `UARX`                    | 1        | Called when one character is finished received on UARX.            |
| `uart.h`          | `UART`                    | 1        | Called when one character is finished transmitted/received on UART.|
| `watchdog.h`      | `WATCHDOG_CLOCK`          | 1        | Called when Watchdog timeout occurs, and clock must be updated.    |
| `watchdog.h`      | `WATCHDOG_RTT`            | 1        | Called when Watchdog timeout occurs, and RTT clock must be updated.|
| `watchdog.h`      | `WATCHDOG`                | 2,3,4    | Called when WatchdogSignal timeout occurs.                         |
| `devices/sonar.h` | `HCSR04_INT`              | 1,3,4    | Called when HCSR04 echo INT pin changes level.                     |
| `devices/sonar.h` | `HCSR04_PCI`              | 1,3,4    | Called when HCSR04 echo PCINT pin changes level.                   |
| `devices/sonar.h` | `HCSR04_RTT_TIMEOUT`      | 1,3,4    | Called when HCSR04 RTT times out (without any echo).               |
| `devices/sonar.h` | `DISTINCT_HCSR04_PCI`     | 1        | Called when HCSR04 any echo PCINT pin changes level.               |
| `devices/sonar.h` | `MULTI_HCSR04_PCI`        | 3,4      | Called when MultiHCSR04 any echo PCINT pin changes level.          |
| `devices/sonar.h` | `MULTI_HCSR04_RTT_TIMEOUT`| 1,3,4    | Called when MultiHCSR04 RTT times out (without any echo).          |

For further details on ISR registration in FastArduino, you can check [`interrutps.h` API](interrupts_8h.html) for the general approach, and each individual API documentation for specific interrupts.

### Pin Interrupts

One very common usage of AVR interrupts is to handle level changes of specific digital input pins. AVR MCU have two kinds of pin interrupts:
- External Interrupts: an interrupt can be triggered for a specific pin when its level changes or is equal to some value (high or low); the number of such pins is quite limited (e.g. only 2 on Arduino UNO)
- Pin Change Interrupts: an interrupt is triggered when one in a set of pins has a changing level; the same interrupt (hence the same ISR) is used for all pins (typically 8, but not necessarily), thus the ISR must determine, by its own means, which pin has triggered the interrupt; although more pins support this interrupt, it is less convenient to use than external interrupts.

#### External Interrupts

External Interrupts are handled with the API provided by `int.h`. This API is demonstrated in the example below:

@includelineno fastarduino/interrupts_1_ext_blink.cpp

In this example, most of the MCU time is spent sleeping in power down mode. The pin `INT0` (D2 on UNO) is connected to a push button and used to switch on or off the UNO LED (on D13).

Note how the button pin is defined:
@dontinclude fastarduino/interrupts_1_ext_blink.cpp
@skipline SWITCH
Here we use `board::ExternalInterruptPin` enum to find out the possible pins that can be used as External Interrupt pins.

Then we define the class that will handle interrupts occurring when the button pin changes level:
@skip PinChangeHandler
@until };
The interesting code here is the `on_pin_change()` method which be called back when the button is pushed or relaxed, i.e. on any level change.
Since this method is called whatever the new pin level, it must explicitly check the button status (i.e. the pin level) to set the right output for the LED pin. Note that, since we use `gpio::PinMode::INPUT_PULLUP`, the pin value will be `false` when the button is pushed, and `true` when it is not.

You should also note the use of `board::EXT_PIN<SWITCH>()` in the definition of the `_switch` FastPin member; that template method converts an `board::ExternalInterruptPin` value to a `board::DigitalPin` value.

The next line of code registers `PinChangeHandler` class and `on_pin_change()` method as callback for `INT0` interrupt.
@skipline REGISTER
In addition to the class and method, the macro takes the number of the `INT` interrupt (`0`) and the pin connected to this interrupt.
Note that the pin reference is redundant; it is passed as a way to ensure that the provided `INT` number matches the pin that we use, if it doesn't, then your code will not compile.

Then the `main()` function oincludes the following initialization code:
@skip PinChangeHandler
@until enable
Once the interrupt handler has been instantiated, it must be registered (2nd step of method callback registration).
Then an `INTSignal` is created for `INT0` and it is set for any level change of the pin.
Finally, the interrupt is activated.

Note the infinite loop in `main()`:
@skip while
@until }
With this loop, we set the MCU to sleep in lowest energy consumption mode. Only some interrupts (`INT0` is one of them) will awaken the MCU from its sleep, immediately after the matching ISR has been called.

Here is the equivalent example with Arduino API:

@includelineno arduino/interrupts_1_ext_blink.ino

First note that Arduino API has no API for sleep mode management, hence you have to include `<avr/sleep.h>` and handle all this by yourself.
Also note that you can only attach a function to an interrupt. Arduino API offers no way to attach a class member instead.

Now let's just compare examples sizes, as usual:

@snippetdoc tables.txt interrupts_1_ext_blink


#### Pin Change Interrupts

Pin Change Interrupts are handled with the API provided by `pci.h`. This API is demonstrated in the example below:

@includelineno fastarduino/interrupts_2_pci_blink.cpp

This example performs the same as the previous example except it uses another pin for the button.
Most of the code is similar, hence we will focus only on differences.

The first difference is in the way we define the pin to use as interrupt source:
@dontinclude fastarduino/interrupts_2_pci_blink.cpp
@skip SWITCH
@until PCI_NUM
Here we use `board::InterruptPin` enum to find out the possible pins that can be used as Pin Change Interrupt pins. We select pin D14 and see from its name that is belonging to `PCINT1` interrupt vector. We also define the `PCINT` number as a constant, `PCI_NUM`, for later use.

You should also note the use of `board::PCI_PIN<SWITCH>()` in the definition of the `_switch` FastPin member; that template method converts an `board::InterruptPin` value to a `board::DigitalPin` value.

Then we register `PinChangeHandler` class and `on_pin_change()` method as callback for `PCINT1` interrupt.
@skipline REGISTER

Once we have, as before,registered our handler class, we then create a `PCISignal` instance that we will use to activate the proper interrupt:
@skip PCIType
@until enable()
Before enabling the `PCINT1` interrupt, we need to first indicate that we are only interested in changes of the button pin.

For this example, we cannot compare sizes with the Arduino API equivalent because Pin Change Interrupts are not supported by the API.


@anchor events Advanced: Events, Scheduler
------------------------------------------

FastArduino supports (and even encourages) events-driven programming, where events are generally generated and queued by interrupts (timer, pin changes, watchdog...) but not only, and events are then dequeued in an infinite loop (named the "event loop") in `main()`. 

This way of programming allows short handling of interrupts in ISR (important because interrupts are disabled during ISR execution), and defer long tasks execution to the event loop.

### Events API

We will use the following example to explain the general Event API:

@includelineno fastarduino/events_1_blink.cpp

In this example, 8 buttons are connected to one port and generate Pin Change Interrupts when pressed. The registered 
ISR generates an event for each pin change; the main event loop pulls one event from the events queue and starts a 
blinking sequence of the LED connected to D13 (Arduino UNO); the selected sequence (number of blinks and delay between 
blinks) depends on the pressed button.

First off, you need to include the heeader with FastArduino Events API:
@dontinclude fastarduino/events_1_blink.cpp
@skipline events.h

Then we will be using `namespace events` in our example:
@skipline using

Events API defines `template<typename T> class Event<T>` which basically contains 2 properties:
- `type()` which is an `uint8_t` value used to discriminate different event types. FastArduino predefines a few event
types and enables you to define your own types as you need,
- `value()` is a `T` instance which you can use any way you need; each event will transport such a value. If you do not need any additional information on your own events, then you can use `void` for `T`.

In the example, we use events to transport the state of all 8 buttons (as a `uint8_t`):
@skipline EVENT

We also define a specific event type when buttons state changes:
@skipline BUTTON_EVENT

Note that `Type` is a namespace inside `events` namespace, which lists FastArduino pre-defined Event types as constants:
@snippet tuto_snippets.cpp events_types
Numbers from 3 to 127 are reserved for FastArduino future enhancements.

The we define the class that will handle Pin Change Interrupts and generate events:
@skip EventGenerator
@until };
Note the use of `containers::Queue<EVENT>` that represents a [ring-buffer](https://en.wikipedia.org/wiki/Circular_buffer)
queue to which events are pushed when the state of buttons are changing, and from which events are pulled from the main 
event loop (as shown later).

The most interesting code is the single line:
@snippet tuto_snippets.cpp events_1_blink_push
where a new `EVENT` is instantiated, its value is the current level of all 8 buttons pins; then the new event is pushed 
to the events queue.

As [usual](@ref interrupts), we have to register this handler to the right PCI ISR:
@skipline REGISTER

Next we define the size of the events queue, which must be a power of 2 (this allows code size and speed optimizations):
@skipline EVENT_QUEUE_SIZE
This must be known at compile-time as we do not want dynamic memory allocation in an embedded program.

The actual events queue is instantiated inside `main()`:
@skip buffer
@until event_queue
Note that this is done in 2 steps:
1. A buffer of the required size is allocated (in the stack for this example); the actual size, in this example, is `64` 
bytes, i.e. 32 times 1 byte (event type) + 1 byte (event value).
2. A `Queue` is instantiated for this buffer; after this step, you should never directly access `buffer` from your code.

Finally, the `main()` function has the infinite event loop:
@skip while
@until }
The call to `containers::pull(event_queue)` is blocking until an event is available in the queue, i.e. has been pushed
by `EventGenerator::on_pin_change()`. While waiting, this call uses `time::yield()` which may put the MCU in sleep mode (see [Power management tutorial later](@ref power)).

Once an event is pulled from the event queue, we check its type and call `blink()` function with the event value, i.e. 
the state of buttons when Pin Change Interrupt has occurred.

The `blink()` function simply loops for the requested number of iterations and blinking delay, and returns only when the 
full blinking sequence is finished. This function is very simple and there is no need to explain it further.

### Event Dispatcher

In the previous example, only one type of event is expected inside the event loop. In more complex applications though,
many more types of events are foreseeable and we may end up with an event loop like:
@snippet tuto_snippets.cpp events_loop_example
This generally makes code less readable and more difficult to maintain.

FastArduino API supports an *event dispatching* approach, where you define `EventHandler` classes and register 
instances for specific event types:

@snippet tuto_snippets.cpp events_handler_example

With `events::Dispatcher`, you can register one handler for each event type, and then the main event loop
is reduced to the simple code below:
@snippet tuto_snippets.cpp events_dispatcher_loop
The right event handler gets automatically called for each pulled event.

Note that `events::EventHandler` uses a `virtual` method, hence this may have an impact on memory size consumed in
Flash (vtables) and also on method call time.

### Scheduler & Jobs

Some types of events are generated by FastArduino features. In particular, watchdog and realtime timer features can be
setup to trigger some time events at given periods. From these events, FastArduino offers a `events::Scheduler` API, 
that allows you to define periodic `events::Job`s.

The next code demonstrates a new way of blinking a LED by using watchdog generated events and associated jobs:

@includelineno fastarduino/events_2_jobs.cpp

FastArduino `Scheduler` API is defined in its own header, but under the same `events` namespace:
@dontinclude fastarduino/events_2_jobs.cpp
@skipline scheduler.h

This API is based on FastArduino events management, hence we need to define the `Event` type we need:
@skipline EVENT
FastArduino `Scheduler` does not use `Event`'s`value`, hence we can simply use `void`, so that one `EVENT` will be
exactly one byte in size.

Next step consists in registering Watchdog clock ISR:
@skipline REGISTER

We then define a `Job` subclass that will be called back at the required period:
@skip LedBlinkerJob
@until };
`Job` constructor takes 2 arguments:
- `next`: the delay in ms after which this job will execute for the first time; `0` means this job should execute
immediately
- `period`: the period in ms at which the job shall be re-executed; `0` means this job will execute only once.

In this example, we set the job period to 1000ms, i.e. the actual blink period will be 2s.

The `virtual` method `Job::on_schedule()` will get called when the time to executed has elapsed, not necessarily at the 
exact expected time (the exact time depends on several factors exposed hereafter).

In the example, we then declare the event queue, this time as static memory (not in the stack):
@skip EVENT_QUEUE_SIZE
@until event_queue

In `main()`, we create a `Dispatcher` that will be used in the event loop, a `Watchdog` that
will generate and queue `Type::WDT_TIMER` events on each watchdog timeout, and a `Scheduler` that will
get notified of  `Type::WDT_TIMER` events and will use the created `Watchdog` as a clock, in order to
determine which `Job`s to call.
@skip dispatcher
@until insert
Note the last code line, which links the `Scheduler` to the `Dispatcher` so it can handle proper events.

Next, we instantiate the job that will make the LED blink, and schedule with `Scheduler`:
@skip job
@until scheduler

We can then start the watchdog with any suitable timeout value (the selected value impacts the accuracy of the 
watchdog clock):
@skipline begin

Finally, then event loop is quite usual:
@skip while
@until }

Jobs scheduling accuracy depends on several factors:
- the accuracy of the `CLOCK` used for `Scheduler`; FastArduino provides two possible clocks (but you may also 
provide your own): `watchdog::Watchdog` and `timer::RTT`.
- the CPU occupation of your main event loop, which may trigger a `Job` long after its expected time.

As mentioned above, `Scheduler` can also work with `timer::RTT`, that would mean only a few changes in the previous 
example, as shown in the next snippet, only showing new lines of code:
@snippet tuto_snippets.cpp events_rtt_scheduler

Since this code uses a realtime timer, two lines of code instantiate, then start one such timer:
@snippet tuto_snippets.cpp events_rtt_scheduler_snip1

In addition, we use `timer::RTTEventCallback` utility class, which purpose is to generate events from
RTT ticks:
@snippet tuto_snippets.cpp events_rtt_scheduler_snip2
Here we create a callback that will generate an event every `RTT_EVENT_PERIOD` milliseconds.

We also need to register the right ISR for the RTT and the callback:
@snippet tuto_snippets.cpp events_rtt_scheduler_snip3

Now the scheduler can be instantiated with the RTT instance as its clock and `Type::RTT_TIMER` as
the type of events to listen to:
@snippet tuto_snippets.cpp events_rtt_scheduler_snip4


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

@includelineno fastarduino/power_1_blink.cpp

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

@includelineno fastarduino/eeprom_1_toneplay.cpp

This example plays a short melody, stored in EEPROM, to a buzzer wired on Arduino UNO D9 pin.
It uses FastArduino `devices::audio::ToneGenerator` support to generate square waves at the required
frequencies, in order to generate a melody intro that Star Wars fan will recognize!

Past the headers inclusion, UNO-specific definitions (timer and output pin) and various `using` directives,
the example defines `struct TonePlay` which embeds a `Tone` (frequency) and a duration, which is then used 
in an array of notes to be played:
@dontinclude fastarduino/eeprom_1_toneplay.cpp
@skip TonePlay
@until Melody
@skip TonePlay
@until };
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
@skip TonePlay*
@until EEPROM
In this snippet, `play` is used just as an "index" (an address actually) to the current note
in `music` array. Beware that `play` cannot be used directly in your program because it points nowhere 
actually, really.

The interesting bit here is `EEPROM::read(play, tone);` which uses `play` address as a reference to the next note
in EEPROM, reads the EEPROM content at this location and copies it into `tone`.

At the end of the loop, `play` address gets incremented in order to point to the next note of `music` in EEPROM:
@skip ++play
@until }

At this point, you may wonder when the `while (true)` loop exits. Since the program does not in advance what melody
it is going to play, it it not possible to use a `for` loop with an upper boundary as we could do if `music` was
directly in SRAM.

Thus, we use another way to determine the end of the meoldy to play: we use a special `Tone`, `Tone::END`, which is
just a marker of the end of the melody; we check it for every tone read from EEPROM:
@snippet tuto_snippets.cpp eeprom_tone_play_stop

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

The following example stores a WIFI access point name and password in EEPROM; through USB connected to 
a serial console, it asks the user if she wants to use current settings (provided there are already
WIFI settings in EEPROM), and if not, it asks the user to fill in new WIFI credentials and stores them to EEPROM
for next time the program will be reset.

@includelineno fastarduino/eeprom_2_write.cpp

Most of this snippet is code that deals with input/output streams through UART; we will not explain it here
as it is already demonstrated in a previous tutorial section.

We will focus on EEPROM sections:
@dontinclude fastarduino/eeprom_2_write.cpp
@skip MAX_LEN
@until wifi_password
Here we define `wifi_name` and `wifi_password` as strings of 64 characters maximum (+ terminating `'\0'`),
stored in EEPROM and both "initialized" as empty strings.

Initialization here does not mean that these variables will automatically be empty string the first 
time this program is executed. It means that after `make build`  is invoked, an `.eep` file is
generated in order to be uploaded to EEPROM, and that file contains both variables as empty strings.
Hence, provided you have uploaded that `.eep` file to your MCU EEPROM, both variables will then
be empty strings when first read from EEPTOM by your program. 

In the `main()` function, we then read both variables from EEPROM into local variables:
@skip wifi
@until wifi_password
Here we use `EEPROM::read(uint16_t address, T* value, uint16_t count)` function template,
instantiated with `T = char`: we have to specify the count of characters to be read, i.e. the maximum
allowed string size, plus the null termination.

As in the previous example, we use `wifi_name` and `wifi_password` variables to provide the address 
in EEPROM, of the location of these 2 strings.

Later in `main()`, if the user decided to enter a new WIFI netwrok name and pasword, those are
written immediately to EEPROM:
@skipline EEPROM::write
@skipline EEPROM::write
The `EEPROM::write()` function arguments are similar to `EEPROM::read()` arguments. 

Note that here, we do not have to write the full string allocated space to the EEPROM but only the currently 
relevant count of characters, e.g. if `wifi` was input as `"Dummy"`, we will write only `6` characters
for `wifi_name` in EEPROM, i.e. the actual string length `5` + `1` for null termination.
This is particularly improtant to do this as writing bytes to EEPROM is very slow (up to ~4ms) so we want to limit
that writing time to the strict minimum necessary.

Because writing content to the EEPROM is a very slow operation, there are situations where you do not want
to "stop" your program running because it is waiting for `EEPROM::write()` operations to complete.

This is the reason why `eeprom` namespace also defines a `QueuedWriter` class that uses interruptions to
write content to EEPROM, allowing your `main()` function to run normally during EEPROM writes. For more
details, please check the API.


@anchor spi Advanced: SPI devices example
-----------------------------------------

FastArduino supports SPI ([Serial Peripheral Interface](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface))
as provided by all AVR MCU (ATmega MCU support SPI natively, ATtiny MCU support it through their USI, Universal 
Serial Interface).

FastArduino also brings specific support to some SPI devices:
- NRF24L01P (radio-frequency device)
- WinBond chips (flash memory devices)

Basically, FastArduino core SPI API is limited to the following:
- `spi::init()` function to call once before any use of SPI in your program
- `spi::SPIDevice` class that is the abstract base class of all concrete SPI devices

Although very important, this sole API is useless for any tutorial example! If you want to develop an API for 
your own SPI device then please refer to `spi::SPIDevice` API documentation.

Hence, to illustrate SPI usage in this tutorial, we will focus on a concrete example with the WinBond memory chip.

The following example reads, erases, writes and reads again flash memory:

@includelineno fastarduino/spi_1_winbond.cpp

The first important piece of code initializes the SPI system and the WinBond device:
@dontinclude fastarduino/spi_1_winbond.cpp
@skip Initialize
@until out
Note that `devices::WinBond` is a template that takes as parameter the `board::DigitalPin` that
will be used as "Chip Select" pin (**CS**, part of SPI wiring).

`status()` returns the WinBond status as specified in the chip datasheet and implemented
in FastArduino `winbond.h`.

Next code piece is reading a part of a flash page from the device:
@skipline read_data
There is nothing special to mention here, the API is straightforward.

Then the example write one page of flash memory. This must be done in 2 steps:
1. Erase flash page
2. Write flash page

@skip Erase
@until F("Wait
Any write action must be preceded with a call to `enable_write()` and followed by
`wait_until_ready()` (because writing to flash memory is rather long, all timing values can 
be found in the datasheet). FastArduino provides API for every case.

Erasing, like writing, cannot be done on single bytes, but must be done on packs of bytes
(e.g. pages, sectors, blocks, as defined in WinBond datasheet).

Here is the code performing the writing:
@skip Write
@until F("Wait
There is only one API, writing at most one page (256 bytes) at once.
Please note the last argument of `write_page()` which is the size of data to write.
For performance reasons, size is an `uint8_t`, consequently, `0` is used to mean `256` bytes,
i.e. a complete page.

The last part of the example reads again the data page and displays it for control. This is
the same code as the reading code presented above.


@anchor i2c Advanced: I2C devices example
-----------------------------------------

FastArduino supports I2C ([Inter-Integrated Circuit](https://en.wikipedia.org/wiki/I%C2%B2C))
as provided by all AVR MCU (ATmega MCU support I2C natively, ATtiny MCU support it through their USI, Universal 
Serial Interface). I2C is also often called *TWI* for Two-Wires Interface.

FastArduino also brings specific support to several I2C devices:
- DS1307 (real-time clock)
- MPU6050 (accelerometer and gyroscope)
- HMC5883L (compass)
- MCP23017 (16-Bit I/O Expander)

FastArduino core I2C API is defined in several headers and made of a few types:
- `i2c.h` contains a few constants and enumerations used everywhere else in the I2C API
- `i2c_manager.h` defines the `i2c::I2CManager` template class which is central to the API
- `i2c_device.h` mainly defines `i2c::I2CDevice` template class, which is the abstract base
class of all concrete I2C devices

In order to illustrate concretely I2C API usage in this tutorial, we will focus on a concrete
example with the DS1307 RTC chip. If you want to develop an API for your own I2C device then
please refer to `i2c::I2CDevice` API documentation.

The following example reads the current clock time from a DS1307 chip:

@includelineno fastarduino/i2c_1_rtc.cpp

This example has 3 important parts.

The first part is the I2C and the RTC device initialization:
@dontinclude fastarduino/i2c_1_rtc.cpp
@skip I2CManager
@until DS1307
`i2c::I2CManager` is a template class with a parameter of type `i2c::I2CMode`, which can any of:
- `i2c::I2CMode::Standard`: slow I2C mode (100 kHz), this the default
- `i2c::I2CMode::Fast`: fast I2C mode (400 kHz)

The mode selection depends on all devices you wire on the I2C bus, if one is using standard mode, 
then all the bus must be set to standard mode. Since DS1307 chip does not support fast mode,
its device forces standard mode, and that mode must be used for the `I2CManager`.

It is important to ensure `begin()` has been called on `i2c::I2CManager` before any use of 
the I2C bus by devices.

Next code piece is reading current clock date and time from the RTC chip:
@skip now
@until get_datetime
In that code, `tm` is a structure containing all fields related to a date/time, `get_datetime()`
just fills it with every information, as can be seen in the following lines of code that 
display the current date and time.

The last line just stops the I2C circuitry of the AVR MCU.
@skipline end()


@anchor softuart Advanced: software UART
----------------------------------------

AVR ATtiny MCU do not include hardware UART. For these MCU, UART can be simulated if needed.
FastArduino has support for software UART. That support can also be useful with ATmega MCU, 
where one would need more UART ports than available.

It is important to note that it is not possible, with software UART, to reach bitrates as high
as with hardware UART.

Here a simple example using software UATX for output through USB on an Arduino UNO:

@includelineno fastarduino/softuart_1_ostream.cpp

Except for a few lines, this is the same example as one hardware UART example above in 
this tutorial.

The main difference is in `serial::soft::UATX<TX> uart{output_buffer};` where `TX` is the
`board::DigitalPin` where the serial output will be directed.

Besides, the same operations as for `serial::hard::UATX` are available, in particular output
streams work the same with `serial::soft::UATX`.

There are also two classes, `serial::soft::UARX_EXT` and `serial::soft::UARX_PCI`, that work similarly as `serial::hard::UARX`:

@includelineno fastarduino/softuart_2_istream.cpp

Note the following differences with a previous example using `serial::hard::UARX`:

1. `RX` must be an interrupt pin (either `board::InterruptPin` 
or `board::ExternalInterruptPin`)
2. `REGISTER_UART_PCI_ISR(RX, PCI_NUM)` (or `REGISTER_UART_INT_ISR(RX, INT_NUM)`) is needed to register an ISR on 
pin `RX` changes
3. the `uarx` variable must be defined as a `serial::soft::UARX_PCI` if it is connected to an `board::InterruptPin` or a `serial::soft::UARX_EXT` if it is connected to an `board::ExternalInterruptPin`
3. an interrupt handler must be setup (either `interrupt::PCISignal` or 
`interrupt::INTSignal`) and enabled
4. `begin()` is passed the interrupt handler as an extra argument

Finally, there are also two classes, `serial::soft::UART_PCI` and `serial::soft::UART_EXT`, that combine `serial::soft::UATX`
and `serial::soft::UARX_PCI` or `serial::soft::UARX_EXT`, and work similarly as `serial::hard::UART`.
