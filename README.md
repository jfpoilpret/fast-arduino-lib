FastArduino
-----------

FastArduino is a C++, object-oriented library for Arduino boards based on AVR MCU and also bare AVR MCU. Its objectives are:

- provide smaller and faster code for the same functionality
- ensure you pay (size and speed) only for what you use
- use real Object-Oriented Design everywhere
- reduce risk of bad code, e.g. by preventing usage of unexisting features (pins, timers...) for the target MCU
- enforce event-driven programs
- support both ATmega and ATtiny chips

It was originally inspired by Cosa library from Mikael Patel.

After usage of Cosa libraries for several projects on ATmega328 and ATtiny84, I found out that the current way Cosa is built has a few drawbacks related to:

- code size (for small AVR MCU)
- speed (for specific situations such as software UART)

From my viewpoint, the main source of those drawbacks is essentially heavy usage of `virtual` methods, which quickly increases code size when you start to define deep classes hierarchies; this also can have a slight impact on speed due to additional indirection when calling methods.

FastArduino tries to favour C++ templates rather than virtual methods whenever possible; when virtual methods are used, their number is reduced to the minimum needed (abstract virtual methods only, typically used for event handlers).

This comes at a cost: 

1. Template usage is often more complex in applications. The provided examples are here to help.
2. Build times are increased as most code is inside C++ headers

In the initial drafts, I decided not to be compliant with Arduino IDE as I find it is a real pain. All my projects (including FastArduino) are built through netbeans, which was hard to initially setup, but much more friendly to use once setup is done. Also netbeans automatically provides makefiles that make it possible to build projects in command line.

My special setup (I work on Windows but compile everything on an Ubuntu virtual machine) is described in ArduinoDevSetup.docx. This document also describes how I setup netbeans for my projects.

Status
------

The project has started a few months ago only, hence it does not cover much yet.

What the project already has:

- Utilities (queue, list, timing)
- IO support: this comes in 2 flavors, one standard implementation and one template-based, optimized for speed and size. It is possible to mix both flavors in one program.
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
- SPI master support (with WinBond flash memory as a first device example)

As of now, the following platforms are supported (and tested):

- Arduino UNO
- Arduino MEGA
- Breadboard ATmega328 at 8MHz and 16MHz
- Breadboard ATtiny84 at 8MHz

I intend to later add support for:

- Breadboard ATtiny85 at 8MHz
- Arduino Leonardo

Roadmap
-------

The roadmap of supported features is the following:

1. Fast IO support (done)
2. Watchdog and events support (done)
3. Power management (done)
4. Hardware UART support (done)
5. Pin Change Interrupt support (done)
6. Software UART support (done)
7. SPI support (done)
8. External Pin Interrupt support (done)
9. Real Time Timer support

10. NRF24L01P support (on going)
11. Trace support (including PROGMEM strings support)
12. Support of bandgap reference
13. More boards variants support (Leonardo, ATtinyX5)
14. ... To be determined later, based on my own projects needs

Some refactoring is also planned, in order to revisit current way to support multiple boards with different capabilities, based on a "traits" approach. This has started with initial RTT implementation.

The following features are not in roadmap currently:

- Analog Input support
- PWM Output support
- I2C support
- OWI support
