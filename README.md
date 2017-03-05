FastArduino
===========

FastArduino is a C++, object-oriented library for Arduino boards based on AVR MCU and also bare AVR MCU. Its objectives are:

- provide smaller and faster code than other libraries for the same functionality
- ensure you pay (size and speed) only for what you use
- use real Object-Oriented Design everywhere
- reduce risk of bad code, e.g. by preventing usage, at compile time, of unexisting features (pins, timers...) for the target MCU
- enforce event-driven programs
- support both ATmega and ATtiny chips

It was originally inspired by Cosa library from Mikael Patel.

After usage of Cosa libraries for several projects on ATmega328 and particularly ATtiny84, I found out that the current way Cosa is built has a few drawbacks related to:

- code size (for small AVR MCU)
- speed (for specific situations such as software UART)

From my viewpoint, the main source of those drawbacks is essentially heavy usage of `virtual` methods, which quickly increases code size when you start to define deep classes hierarchies; this also can have a slight impact on speed due to additional indirection when calling methods.

FastArduino tries to favour C++ templates rather than virtual methods whenever possible; when virtual methods are used, their number is reduced to the minimum needed (abstract virtual methods only, typically used for event handlers, generally limited to hierarchy of 2 levels only, one parent and direct children).

This comes at a cost: 

1. Template usage is often more complex in applications. The provided examples are here to help.
2. Build times are increased as most code is inside C++ headers

Also, if you consider using FastArduino for your projects, be aware that FastArduino does not support Arduino API and does not intend to do so some day. That means you will have to first learn FastArduino API (you can use numerous examples provided for that) in order to reap its benefits. FastArduino is definitely not for newcomers to C++ programming as it makes heavy use of C++ specificities. Note that FastArduino currently uses C++11 standard.

In the initial drafts, I decided not to be compliant with Arduino IDE as I find it is a real pain. All my projects (including FastArduino) are built through netbeans, which was hard to initially setup, but much more friendly to use once setup is done. Also netbeans automatically provides makefiles that make it possible to build projects in command line.

Making FastArduino buildable on Arduino IDE is not on my roadmap currently (and probably won't until long, as I'm more than happy with my current setup until now).

My special setup (I work on Windows but compile everything on an Ubuntu virtual machine) is described in [ArduinoDevSetup.docx](ArduinoDevSetup.docx). This document also describes how I setup netbeans for my projects.

Status
======

The project has started less than one year ago only, hence it does not cover everything yet, also its API is likely to change, more or less drastically, until April (see roadmap).

What the project already has:

- General utilities (queue, list, timing)
- Fast IO support: template-based, optimized for speed and size.
- General Events handling
- Watchdog timer
- Timed (periodic or not) jobs scheduling
- Real Time Timer with microsecond precision
- Power sleep
- Pin Change Interrupt (PCI) handling
- External Pin Interrupt handling
- Hardware UART support (for MCU that support it, ie not for ATtiny)
- Software UART support (for all MCU)
- "C++ like" Input/Output streams (used by UART implementations)
- SPI master support
- WinBond flash memory support (SPI-based)
- NRF24L01 device support (SPI-based)
- Analog Digital Conversion support (in Single Conversion mode only)
- Power supply voltage measurement support
- Flash memory data support (PROGMEM data), particularly strings (useful to limit used SRAM)
- EEPROM support, with synchronous or asynchronous write
- PWM output support

As of now, the following platforms are supported (and tested):

- Arduino UNO
- Arduino MEGA
- Breadboard ATmega328 at 8MHz and 16MHz
- Breadboard ATtiny84 at 8MHz

I intend to later add support for:

- Breadboard ATtiny85 at 8MHz
- Arduino Leonardo

Roadmap
=======

The roadmap of next activities and new supported features is the following:

March-April 2017
----------------
1. I2C support
2. Add support for Arduino Leonardo (without CDC feature)
3. Improve Timer support (support input capture where available)

May 2017
--------
4. Add support for ATtinyX5
5. Add CDC feature support for Arduino Leonardo

June 2017
---------
6. Improve Analog Input support and code
7. Improve SPI to support slave mode

In addition to these activities, I will start to perform continuous improvements at all times, regarding:

- coding guidelines applied across the library code (not necessarily example code)
- documentation (ISR handling, Board support adding, API...)

The following features are not in FastArduino roadmap currently (but may be in the future):

- OWI support
