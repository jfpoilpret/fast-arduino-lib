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

Building a project using FastArduino
------------------------------------
The easiest way to start a project using FastArduino is probably to use [netbeans](https://netbeans.org/), first setup as described [here](https://github.com/jfpoilpret/fast-arduino-lib/blob/master/ArduinoDevSetup.docx).

Then you can create a new C++ project in netbeans, set the proper configuration for it, based on your target MCU or board, and include FastArduino library to the project.

Your project shall have at least one ".cpp" source file with an `int main()` function, as shown in the [tutorial](tutorial.html).

In the generated `Makefile` of the project, you can include FastArduino provided `Makefile-FastArduino.mk` which will help setup of all predefined macros. Take a look at comments in this provided Makefile to understand how to use it.

Once you can build your project from netbeans, you can build it from a Linux shell as well:

> $ make CONF=UNO-Release
> 
> $ make flash CONF=UNO-Release

TODO: copy from Conway example readme!

FastArduino library organisation
--------------------------------
FastArduino library is organized in a sinple directory structure:
- **fastarduino**: contains "core" FastArduino source files, handling all AVR MCU internal parts: Digital Input/Output, Timers, Analog-Digital Converters, Interrupts, I2C, SPI, Watchdog, UART...
    - **boards**: contains MCU target specific files, defines as "traits"
    - **devices**: contains source code to implement support for various external devices, such as DS1307 RTC chip, HC-SR04 ultrasonic range sensor, NRF24L01+ Radio-frequency chip, Servomotor drives...

Most of the library's source code is inside `.h` header files, only a small part of it is in `.cpp` files. This is due to heavy C++ template usage which requires most source code to be present on header files.

All FastArduino source code is defined inside namespaces, in order to avoid names conflicts across programs. Namespaces can include sub-namespaces in some occasions. Namespaces are organized as follows:

TODO add links to doxygen namespace doc for each namespace!

- analog: contains the API to handle analog input and "pseudo-analog" output (PWM).
- containers: utility API to handle useful containers such as linked lists and queues; those are internally used by some FastArduino API but you can use them in your own programs as well.
- devices: this namespace is used for all devices external to the MCU itself; most devices API comes in a sub namespace:
    - magneto: API for magnetometers, gyroscopes, accelerometers
    - rf: API for radio-frequency chips
    - rtc: API for real time clock chips
    - servo: API to handle servomotors
    - sonar: API to handle sonar range sensors
- eeprom: contains the API to handle read and write to and from the internal MCU EEPROM.
- errors: all errors that can be returned by FastArduino API are defined here as constants.
- events: this namespace defines general event handling that can be used in your programs. Most FastArduino are able to generate events on specific conditions. This namespace also contain the scheduler API which permits scheduling of jobs at specific times or periods.
- flash: contains the API to handle read of data from the internal MCU flash memory; this is particular useful in order to reduce SRAM storage when dealing with constant strings.
- gpio: that namespace deals with all API to manage digital input and outputs.
- i2c: that namespace contains all API to deal with I2C (also known as *Two Wires Interface*), including a base class to help you define support for new devices based on I2C protocol.
- interrupt: this namespace implements the concepts of managing all interrupts within FastArduino; it also contains the API dedicated to handling AVR interrupt pins, either Pin Change Interrupt pins or External Interrupt pins.
- power: contains the API to handle AVR power modes.
- serial: contains the API to handle serial communication; sub namespaces definespecific API for hardware or software based serial communication:
    - hard: this namespace support AVR embedded UART (for ATmega MCU only, as ATtiny do not have this feature)
    - soft: this namespace supports software UART (for all MCU); software serial is less efficient and bigger in code size than its hardware equivalent
- spi: that namespace deals with all API to deal with SPI interface, including a base class to help you define support for new devices based on SPI protocol.
- streams: this namespace provide a C++ streams like API for input and output (used by serial UART API).
- time: provides API to delay your program for some amount of time (through busy loops) and a few to deal with time data.
- timer: defines a basic API to deal with MCU timers and their available operation modes, more specific API to use timers for generating pulses, and an API dedicated to track real time through MCU timers, with microsecond precision.
- utils: provides general utilities that did not pertain to any other namespace; many of these utilities allow easy conversion of values between two referentials or "encoding".
- watchdog: this namespace defines an API to deal with MCU watchdog timer as a way 

As you can see, all namespaces in FastArduino are 100% lowercase by convention.

However, please note that FastArduino defines a few specific namespaces, which names start with an uppercase letter, and whose sole purpose is to scope a set of constants. As an example, namespace `events` defines such a sub namespace `Type` which defines all predefined event types as `uint8_t` constants. The reason why `Type` was not defined as an enum class is that it is possible for you to define new types of events, which would not be possible if using an enum.

FastArduino is **heavily** based on C++ templates as you will see when developing with it. Although many people will tell you that "C++ templates are evil and generate bloated code, they are not for embedded", don't believe them! C++ templates, when properly used, bring many advantages without the abovementioned "drawback".

One downside of templates is its syntax, that may sound less familiar to many developers, but that should not be barrier against its use.

In FastArduino, templates offer a huge advantage over competing libraries: type-safety is guaranteed at compile-time whatever the MCU/Arduino board target of your program: for instance, it is **not possible**, with FastArduino, to compile a program that would try to access an IO pin that does not exist on the target; most other libraries I know allow you to do that without complaining.

Finally, regarding FastArduino organization, it is important to note that a special concept has been developed to deal with interrupt vectors. The rationale behind the concept is that FastArduino will **never** register an interrupt vector by itself; instead it provides you specific macros for all kinds of interrupt vectors that will generate proper ISR functions, including type-safety checks embedded in the macro. The major advantage of that is that you will never have libraries competing for the same ISR, as is often the case when using Arduino API. This point is further detailed in [FastArduino ISR Handling](TODO).

Discovering FastArduino API step by step
----------------------------------------
TODO link to tutorial

