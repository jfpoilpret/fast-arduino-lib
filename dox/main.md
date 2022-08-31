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
- is based on C++17 standard
- favours C++ templates rather than virtual methods whenever possible
- virtual methods are used only when needed (mostly for event handlers)
- never automatically registers any ISR but requires explicit registration through provided macros

The rest of this guide provides indications where to start and proceed with learning how to use FastArduino for your projects.

Building FastArduino
--------------------
First off, ensure your Linux box contains the ATmel AVR 8 bit toolchain and your `$PATH` points to its `bin` directory.

The following commands show how to build the FastArduino library:

    > git clone https://github.com/jfpoilpret/fast-arduino-lib.git
    > cd fast-arduino-lib
    > make CONF=UNO build

This creates the FastArduino library, built for Arduino UNO, into `dist/UNO-Release/AVR-GNU-Toolchain-3.5.3-Linux/libfastarduino.a`).

Note that `make` builds one version of FastArduino library for a specific target, that target is specified with `CONF=<target>`. The currently supported targets are:
- UNO: Arduino UNO
- NANO: Arduino NANO
- LEONARDO: Arduino LEONARDO
- MEGA: Arduino MEGA
- ATmega328-8Mhz: bare ATmega328P MCU at 8 MHz
- ATmega328-16MHz: bare ATmega328P MCU at 16 MHz
- ATmega644-8MHz: bare ATmega644P MCU at 8 MHz
- ATmega644-16MHz: bare ATmega644P MCU at 16 MHz
- ATmega1284-8MHz: bare ATmega1284P MCU at 8 MHz
- ATmega1284-16MHz: bare ATmega1284P MCU at 16 MHz
- ATtinyX4-8MHz: bare ATtinyX4 MCU at 8 MHz
- ATtinyX5-8MHz: bare ATtinyX5 MCU at 8 MHz

Building a project using FastArduino
------------------------------------
One simple way to start a project using FastArduino is probably to use [Visual Studio Code](https://code.visualstudio.com/), first setup as described [here](https://github.com/jfpoilpret/fast-arduino-lib/blob/master/ArduinoDevSetup.docx).

Then you can use [FastArduino Project Template](https://github.com/jfpoilpret/fastarduino-project-template) to create your first project (just follow instructions in that repository).

Your project shall have at least one ".cpp" source file with an `int main()` function, as shown in the [tutorial](tutorial.html). The FastArduino Project Template includes this source file already.

That FastArduino project template also supports building from a Linux shell:

    > $ make CONF=UNO
    > $ make flash CONF=UNO

The first `make` builds your program for the target specified with `CONF=...`.

The second `make` uploads the built program to the Flash memory of the target MCU.

The `make` command takes optional arguments that help you specify how to upload to your MCU target, among a set of ways, and through which USB port.

FastArduino library organisation
--------------------------------
FastArduino library is organized in a sinple directory structure:
- **fastarduino**: contains "core" FastArduino source files, handling all AVR MCU internal parts: Digital Input/Output, Timers, Analog-Digital Converters, Interrupts, I2C, SPI, Watchdog, UART...
    - **boards**: contains MCU target specific files, defines as "traits"
    - **devices**: contains source code to implement support for various external devices, such as DS1307 RTC chip, HC-SR04 ultrasonic range sensor, NRF24L01+ Radio-frequency chip, Servomotor drives...

Most of the library's source code is inside `.h` header files, only a small part of it is in `.cpp` files. This is due to heavy C++ template usage which requires most source code to be present on header files.

All FastArduino source code is defined inside namespaces, in order to avoid names conflicts across programs. Namespaces can include sub-namespaces in some occasions. Namespaces are organized as follows:
- [analog](namespaceanalog.html): contains the API to handle analog input and "pseudo-analog" output (PWM).
- [bits](namespacebits.html): provides a few utilities for bits manipulation.
- [containers](namespacecontainers.html): utility API to handle useful containers such as linked lists and queues; those are internally used by some FastArduino API but you can use them in your own programs as well.
- [devices](namespacedevices.html): this namespace is used for all devices external to the MCU itself; most devices API comes in a sub namespace:
    - [audio](namespacedevices_1_1audio.html): API for tone generation to buzzers or small audio amplifiers
    - [magneto](namespacedevices_1_1magneto.html): API for magnetometers, gyroscopes, accelerometers
    - [mcp230xx](namespacedevices_1_1mcp230xx.html): API for Microchip I/O expanders MCP23008 and MCP23017
    - [mcp3x0x](namespacedevices_1_1mcp3x0x.html): API for Microchip ADC chips family
    - [rf](namespacedevices_1_1rf.html): API for radio-frequency chips
    - [rtc](namespacedevices_1_1rtc.html): API for real time clock chips
    - [servo](namespacedevices_1_1servo.html): API to handle servomotors
    - [sonar](namespacedevices_1_1sonar.html): API to handle sonar range sensors
    - [vl53l0x](namespacedevices_1_1vl53l0x.html): API to handle VL53L0X Time of Flight distance sensor
- [eeprom](namespaceeeprom.html): contains the API to handle read and write to and from the internal MCU EEPROM.
- [errors](namespaceerrors.html): all errors that can be returned by FastArduino API are defined here as constants.
- [events](namespaceevents.html): this namespace defines general event handling that can be used in your programs. Most FastArduino are able to generate events on specific conditions. This namespace also contain the scheduler API which permits scheduling of jobs at specific times or periods.
- [flash](namespaceflash.html): contains the API to handle read of data from the internal MCU flash memory; this is particular useful in order to reduce SRAM storage when dealing with constant strings.
- [future](namespacefuture.html): that namespace brings the concept of futures to FastArduino, which is heavily used in FastArduino I2C asynchronous API.
- [gpio](namespacegpio.html): that namespace deals with all API to manage digital input and outputs.
- [i2c](namespacei2c.html): that namespace contains all API to deal with I2C (also known as *Two Wires Interface*), including a base class to help you define support for new devices based on I2C protocol.
- [interrupt](namespaceinterrupt.html): this namespace implements the concepts of managing all interrupts within FastArduino; it also contains the API dedicated to handling AVR interrupt pins, either Pin Change Interrupt pins or External Interrupt pins.
- [memory](namespacememory.html): provides a few utility to check available SRAM (useful for debugging).
- [power](namespacepower.html): contains the API to handle AVR power modes.
- [serial](namespaceserial.html): contains the API to handle serial communication; sub namespaces definespecific API for hardware or software based serial communication:
    - [hard](namespaceserial_1_1hard.html): this namespace support AVR embedded UART (for ATmega MCU only, as ATtiny do not have this feature)
    - [soft](namespaceserial_1_1soft.html): this namespace supports software UART (for all MCU); software serial is less efficient and bigger in code size than its hardware equivalent
- [spi](namespacespi.html): that namespace deals with all API to deal with SPI interface, including a base class to help you define support for new devices based on SPI protocol.
- [std](namespacestd.html): subset of C++ std namespace; it includes a few utilities necessary for some FastArduino features.
- [streams](namespacestreams.html): this namespace provide a C++ streams like API for input and output (used by serial UART API).
- [time](namespacetime.html): provides API to delay your program for some amount of time (through busy loops) and a few more API to deal with time data.
- [timer](namespacetimer.html): defines a basic API to deal with MCU timers and their available operation modes, more specific API to use timers for generating pulses, generating square waves, and an API dedicated to track real time through MCU timers, with microsecond precision.
- [types_traits](namespacetypes__traits.html): provides a few utility dealing with types.
- [utils](namespaceutils.html): provides general utilities that did not pertain to any other namespace; many of these utilities allow easy conversion of values between two referentials or "encoding".
- [watchdog](namespacewatchdog.html): this namespace defines an API to deal with MCU watchdog timer as a way.

In addition to all namespaces above, there is one omnipresent namespace `board` that defines all configuration (list of digital & analog pins, timers...) specific to an MCU target. The documentation for this namespace depends on the MCU target, as described [here](supportedboards.html).

As you can see, all namespaces in FastArduino are 100% lowercase by convention.

However, please note that FastArduino defines a few specific namespaces, which names start with an uppercase letter, and whose sole purpose is to scope a set of constants. As an example, namespace `events` defines such a sub namespace `Type` which defines all predefined event types as `uint8_t` constants. The reason why `Type` was not defined as an enum class is that it is possible for you to define new types of events, which would not be possible if using an enum.

FastArduino is **heavily** based on C++ templates as you will see when developing with it. Although many people will tell you that "C++ templates are evil and generate bloated code, they are not for embedded", don't believe them! C++ templates, when properly used, bring many advantages without the abovementioned "drawback".

One downside of templates is its syntax, that may sound less familiar to many developers, but that should not be a barrier against its use.

In FastArduino, templates offer a huge advantage over competing libraries: type-safety is guaranteed at compile-time whatever the MCU/Arduino board target of your program: for instance, it is **not possible**, with FastArduino, to compile a program that would try to access an IO pin that does not exist on the target; most other libraries I know allow you to do that without complaining.

Finally, regarding FastArduino organization, it is important to note that a special concept has been developed to deal with interrupt vectors. The rationale behind the concept is that FastArduino will **never** register an interrupt vector by itself; instead it provides you specific macros for all kinds of interrupt vectors that will generate proper ISR functions, including type-safety checks embedded in the macro. The major advantage of that is that you will never have libraries competing for the same ISR, as is often the case when using Arduino API. This point is further detailed in [FastArduino ISR Handling](interrupts_8h.html#details).

Discovering FastArduino API step by step
----------------------------------------
The best way to discover FastArduino is to follow [this tutorial](tutorial.html).

There are also specific, more advanced, pieces of documentation for the following topics:
- [Adding support for an SPI device](spidevices.html)
- [Adding support for an I2C device](i2cdevices.html)
