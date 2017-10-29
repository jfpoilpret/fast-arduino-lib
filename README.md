FastArduino
===========

FastArduino is a C++ object-oriented library for Arduino boards based on AVR MCU and also bare AVR MCU.

FastArduino API is documented [here](http://jfpoilpret.github.io/fast-arduino-lib/) and documentation also includes a [simple tutorial](http://jfpoilpret.github.io/fast-arduino-lib/tutorial.html).

FastArduino objectives are:

- provide smaller and faster code than other libraries for the same functionality
- ensure you pay (size and speed) only for what you use
- use modern C++ Object-Oriented Design everywhere
- reduce risk of bad code, e.g. by preventing usage, at compile time, of unexisting features (pins, timers...) for the target MCU
- support event-driven programs
- support both ATmega and ATtiny chips

It was originally inspired by [Cosa library](https://github.com/mikaelpatel/Cosa) from Mikael Patel.

After usage of Cosa libraries for several projects on ATmega328 and particularly on ATtiny84, I found out that the current way Cosa is built has a few drawbacks related to:

- code size (for small AVR MCU)
- speed (for specific situations such as software UART)

From my viewpoint, the main source of those drawbacks is essentially heavy usage of `virtual` methods, which quickly increases code size when you start to define deep classes hierarchies; this also can have a slight impact on speed due to additional indirection when calling methods. Calling `virtual` methods from an ISR also has a big impact on code size as the generated code for ISR will push all registers to the stack, then call your ISR code, and finally pop all regsiters back from the stack; of course, this also has a dramatic impact on ISR execution speed. Avoiding `virtual` methods calls from an ISR ensures the compiler generates only the strict minimum of push and pop necessary.

FastArduino tries to favour C++ templates rather than virtual methods whenever possible; when virtual methods are used, their number is reduced to the minimum needed (abstract virtual methods only, typically used for event handlers, generally limited to hierarchy of 2 levels only, one abstract parent and direct children). 

Also, no ISR gets automatically declared by FastArduino: every program declares the ISR it needs by using pre-defined FastArduino ISR-registration macros (note that ISR registration is the only feature for which FastArduino uses macros). FastArduino does not use `virtual` methods for ISR callbacks, thus permitting optimization of ISR code size, which would not have been possible with `virtual` methods as callbacks.

All this comes at a cost: 

1. Template usage is often more complex in applications. The provided examples are here to help.
2. Build times may be increased a bit as most code is inside C++ headers (recompiled every time included); for this point however, please note that compile time difference is hardly noticeable.

Also, if you consider using FastArduino for your projects, be aware that FastArduino does not support Arduino API and does not intend to do so some day. That means you will have to first learn FastArduino API (you can use the complete [tutorial](http://jfpoilpret.github.io/fast-arduino-lib/tutorial.html) and the numerous examples provided for that) in order to reap its benefits. FastArduino is definitely not for newcomers to C++ programming as it makes heavy use of C++ specificities. Note that FastArduino currently uses C++11 standard.

In the initial drafts, I decided not to be compliant with Arduino IDE as I find it is a real pain. All my projects (including FastArduino) are now built with [Visual Studio Code](https://code.visualstudio.com/) along with a [small extension](https://github.com/jfpoilpret/vscode-fastarduino) I developed specifically for FastArduino. Once properly setup, I find VS Code environment much easier and friendlier to use than Arduino IDE or even [netbeans](https://netbeans.org/) which I originally used for FastArduino but finally ditched out.

FastArduino is also buildable from the command line (on a linux system) through the standard `make`. Its make system can also be used for projects using the FastArduino library.

Making FastArduino buildable on Arduino IDE is not on my roadmap currently (and probably won't until long, as I'm more than happy with my current setup until now).

My complete setup is described in [ArduinoDevSetup.docx](ArduinoDevSetup.docx). This document also describes how I setup Visual Studio Code for my projects.

One easy way to start a new project using FastArduino is to checkout [this project template](https://github.com/jfpoilpret/fastarduino-project-template).

Status
======

The project has started a bit more than one year ago, it covers a lot already, but is not fully complete yet; also its API may still change, more or less drastically, during Fall 2017 (see roadmap).

What the project already has:

- General utilities (queues, linked lists, busy loop delays)
- Fast IO support: template-based, optimized for speed and size.
- Analog Digital Conversion support (in Single Conversion mode only)
- Power supply voltage measurement support
- Flash memory data support (PROGMEM data), particularly strings (useful to limit used SRAM)
- EEPROM support, with synchronous or asynchronous write
- General Events handling
- Watchdog timer
- Timer modes support, including Input Capture
- Timed (periodic or not) jobs scheduling
- Real Time Timer with microsecond precision
- PWM output support
- "Pulsed" Timer support (useful for Servos)
- Power sleep
- Pin Change Interrupt (PCI) handling
- External Pin Interrupt handling
- SPI master support
- I2C master support (synchronous mode)
- Hardware UART support (for MCU that support it, ie not for ATtiny)
- Software UART support (for all MCU)
- "C++ like" Input/Output streams (used by UART implementations)

In addition, FastArduino brings support for the following devices:

- SIPO (*Serial in Parallel out*) chips
- Servo control API
- WinBond flash memory support (SPI-based)
- NRF24L01 device support (SPI-based)
- DS1307 RTC device support (I2C-based)
- HMC5883L magnetometer device support (I2C-based)
- MPU-6050 accelerometer/gyroscope device support (I2C-based)

As of now, the following platforms are supported (and tested):

- Arduino UNO
- Arduino NANO
- Arduino MEGA
- Arduino Leonardo (without USB support)
- Breadboard ATmega328 at 8MHz and 16MHz
- Breadboard ATtiny84 at 8MHz

I intend to later add support for:

- Breadboard ATtiny85 at 8MHz
- Arduino Leonardo with USB support

I have started writing a [step-by-step tutorial](http://jfpoilpret.github.io/fast-arduino-lib/tutorial.html) to learn FastArduino API, which shall be finished by end 2017, but it can already be used to understand some of FastArduino API. In parallel to that, I am documenting [FastArduino API](http://jfpoilpret.github.io/fast-arduino-lib/) through [doxygen](http://www.stack.nl/~dimitri/doxygen/) and that should also be finished by end 2017.

Roadmap
=======

The roadmap of next activities and new supported features is the following:

October 2017
------------
1. Finalize sonar ranger API

November 2017
-------------
2. Improve streams support ala C++
3. Add support for ATtinyX5
4. Refactor Event system to allow any value type for events

1st Quarter 2018
----------------
5. Support other I2C devices: MCP23017 (IO multiplexer)
6. Improve Analog Input support and code
7. Improve I2C master support (asynchronous mode)
8. Improve SPI to support slave mode
9. Improve I2C to support slave mode
10. Add USB support for Arduino Leonardo
11. Add high-speed timer support of Arduino Leonardo

In addition to these activities, I intend to perform various continuous improvements in the following months, regarding:

- complete documentation (Tutorial, API, ISR handling, Board support adding...)
- coding guidelines applied across the library code (not necessarily example code)

The following features are not in FastArduino roadmap currently (but may be in the future):

- OWI support
