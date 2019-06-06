Latest News
===========

FastArduino v1.2 has just been released on 6th June 2019.
The implemented enhancements are listed [here](https://github.com/jfpoilpret/fast-arduino-lib/milestone/4?closed=1).
Please note that this version breaks compatibility on some API, as described in further detail in the [release notes](https://github.com/jfpoilpret/fast-arduino-lib/releases/tag/v1.1).

FastArduino
===========

FastArduino is a C++ object-oriented library for Arduino boards based on AVR MCU and also for bare AVR MCU.

FastArduino API is fully documented [here](http://jfpoilpret.github.io/fast-arduino-lib/) and documentation also includes a complete [tutorial](http://jfpoilpret.github.io/fast-arduino-lib/tutorial.html).

FastArduino C++ code is also analyzed by SonarQube and results of this analysis are published from time to time [here](https://sonarcloud.io/dashboard?id=FastArduino-UNO).

FastArduino objectives are:

- provide smaller and faster code than other libraries for the same functionality
- ensure you pay (size and speed) only for what you use
- use modern C++ Object-Oriented Design everywhere
- reduce risk of bad code, e.g. by preventing usage, at compile time, of unexisting features (pins, timers...) for the target MCU
- support event-driven programs
- support both ATmega and ATtiny chips

It was originally inspired by [Cosa library](https://github.com/mikaelpatel/Cosa) from Mikael Patel.

After usage of Cosa libraries for several projects on ATmega328, and particularly on ATtiny84, I found out that the current way Cosa was built had a few drawbacks related to:

- code size (for small AVR MCU)
- speed (for specific situations such as software UART)

From my viewpoint, the main source of those drawbacks was essentially heavy usage of `virtual` methods, which quickly increases code size when you start to define deep classes hierarchies; this also can have a slight impact on speed due to additional indirection when calling methods. Calling `virtual` methods from an ISR also has a big impact on code size as the generated code for ISR will push all registers to the stack, then call your ISR code, and finally pop all registers back from the stack; of course, this also has a dramatic impact on ISR execution speed. Avoiding `virtual` methods calls from an ISR ensures the compiler generates only the strict minimum of push and pop necessary.

FastArduino tries to favour C++ templates rather than virtual methods whenever possible; when virtual methods are used, their number is reduced to the minimum needed (abstract virtual methods only, typically used for event handlers, generally limited to hierarchy of 2 levels only, one abstract parent and direct children). 

Also, no ISR gets automatically declared by FastArduino: every program declares the ISR it needs by using pre-defined FastArduino ISR-registration macros (note that ISR registration is the only feature for which FastArduino uses macros). FastArduino does not use `virtual` methods for ISR callbacks, thus permitting optimization of ISR code size, which would not have been possible with `virtual` methods as callbacks.

All this comes at a cost: 

1. Template usage is often more complex in applications. The provided examples are here to help.
2. Build times may be increased a bit as most code is inside C++ headers (recompiled every time included); for this point however, please note that compile time difference is hardly noticeable.

Also, if you consider using FastArduino for your projects, be aware that FastArduino does not support Arduino API and does not intend to do so some day. That means you will have to first learn FastArduino API (you can use the complete [tutorial](http://jfpoilpret.github.io/fast-arduino-lib/tutorial.html) and the numerous examples provided for that) in order to reap its benefits. FastArduino is definitely not for newcomers to C++ programming as it makes heavy use of C++ specificities. Note that FastArduino currently uses C++14 standard.

Since the initial drafts, I decided not to be compliant with Arduino IDE as I find it is a real pain. All my projects (including FastArduino itself) are now built with [Visual Studio Code](https://code.visualstudio.com/) along with a [small extension](https://github.com/jfpoilpret/vscode-fastarduino) I developed specifically for FastArduino. Once properly setup, I find VS Code environment much easier and friendlier to use than Arduino IDE or even [netbeans](https://netbeans.org/) which I originally used for FastArduino but finally ditched out.

FastArduino is also buildable from the command line (on a linux system) through the standard `make`. Its make system can also be used for projects using the FastArduino library.

Making FastArduino buildable on Arduino IDE is not on my roadmap currently (and probably won't until long, as I'm more than happy with my current setup until now).

My complete setup is described in [ArduinoDevSetup.docx](ArduinoDevSetup.docx). This document also describes how I setup Visual Studio Code for my projects.

One easy way to start a new project using FastArduino is to checkout [this project template](https://github.com/jfpoilpret/fastarduino-project-template).

Status
======

Latest FastArduino release [**v1.1**](https://github.com/jfpoilpret/fast-arduino-lib/releases/tag/v1.1) has been published on 26.04.2019.

First FastArduino official release [**v1.0**](https://github.com/jfpoilpret/fast-arduino-lib/releases/tag/v1.0) was published on 22.02.2018.

In the current version, the project covers a lot of features; a few missing, less important, features will be released in future versions (see roadmap below); the API of current features is deemed stable and should not change in the future.

What the library already has:

- General utilities (queues, linked lists, busy loop delays)
- Fast IO support: template-based, optimized for speed and size
- Analog Digital Conversion support (in Single Conversion mode)
- Power supply voltage measurement support
- Flash memory data support (PROGMEM data), particularly strings (useful to limit used SRAM)
- EEPROM support, with synchronous or asynchronous write
- General Events handling
- Watchdog timer
- Timer modes support, including Input Capture and Square Waves generation
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

- Tones generation & melody player (e.g. to produce notes through a buzzer)
- SIPO (*Serial in Parallel out*) chips
- Servo control API
- WinBond flash memory support (SPI-based)
- NRF24L01 device support (SPI-based)
- DS1307 RTC device support (I2C-based)
- HMC5883L magnetometer device support (I2C-based)
- MPU-6050 accelerometer/gyroscope device support (I2C-based)
- HC-SR04 sonar device support in synchronous and asynchromous modes

As of now, the following platforms are supported (and tested):

- Arduino UNO
- Arduino NANO
- Arduino MEGA
- Arduino Leonardo (without USB support)
- Breadboard ATmega328 at 8MHz and 16MHz
- Breadboard ATtiny84 at 8MHz
- Breadboard ATtiny85 at 8MHz

I intend to probably add support some day for:

- Arduino Leonardo with USB support

A [step-by-step tutorial](http://jfpoilpret.github.io/fast-arduino-lib/tutorial.html) is available for those who want to learn FastArduino API; it covers the whole FastArduino core API. In addition to that, the [FastArduino API](http://jfpoilpret.github.io/fast-arduino-lib/) is fully documented through [doxygen](http://www.stack.nl/~dimitri/doxygen/).

Roadmap
=======

The roadmap of next activities and new supported features is the following:

- [Milestone v1.0](https://github.com/jfpoilpret/fast-arduino-lib/milestone/1?closed=1) (released on 22.02.2018)
- [Milestone v1.1](https://github.com/jfpoilpret/fast-arduino-lib/milestone/2?closed=1) (released on 26.04.2019)
- [Milestone v1.2](https://github.com/jfpoilpret/fast-arduino-lib/milestone/4?closed=1) (released on 06.06.2019)
- [Milestone v1.3](https://github.com/jfpoilpret/fast-arduino-lib/milestone/5) (31.08.2019)
- [Milestone v1.4](https://github.com/jfpoilpret/fast-arduino-lib/milestone/6) (31.10.2019)
- [Milestone v2.0](https://github.com/jfpoilpret/fast-arduino-lib/milestone/3) (31.12.2019)
- [2020 and later](https://github.com/jfpoilpret/fast-arduino-lib/issues?q=is%3Aopen+is%3Aissue+no%3Amilestone)

Documentation - no milestones
-----------------------------
1. Add SPI and I2C new device creation to Tutorial (31.12.2019)
2. Document how to add support for another board target in Tutorial (31.12.2019)

Milestones dates are "best effort" and may change based on contributors' availability.

The following features are not in FastArduino roadmap currently (but may be in the future):

- OWI support
