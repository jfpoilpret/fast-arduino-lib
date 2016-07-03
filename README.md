FastArduino
-----------

Originally inspired by Cosa library from Mikael Patel.

After usage of Cosa libraries for several projects on ATmega328 and ATtiny84, I found out that the current way Cosa is built has a few drawbacks related to:

- code size (for small AVR MCU)
- speed (for specific situations such as software UART)

From my viewpoint, the main source of those drawbacks is essentially heavy usage of `virtual` methods, which quickly increases code size when you start to define deep classes hierarchies; this also can have a slight impact on speed due to additional indirection when calling methods.

FastArduino tries to favour C++ templates rather than virtual methods whenever possible.

This comes at a cost: 

1. Template usage is often more complex in applications. The provided examples are here to help.
2. Build times are increased as most code is inside C++ headers

In the initial drafts, I decided not to be compliant with Arduino IDE as I find it is a real pain. All my projects (including FastArduino) are built through netbeans, which was hard to initially setup, but much more friendly to use once setup is done. Also netbeans automatically provides makefiles that make it possible to build projects in command line.

Status
------

The project has just started, hence it does not cover much yet.

What the project already has:

- Utilities (queue, list, timing)
- Fast IO support (ATmega328 and ATtinyX4) with examples for UNO
- Events handling with examples for UNO
- Watchdog timer
- Timed (periodic or not) jobs scheduling with examples for UNO
- Power sleep

Roadmap
-------

The roadmap of supported features is the following:

1. Fast IO support (done)
2. Multiple boards variants support (UNO, MEGA, ATtinyX4, ATtinyX5)
3. Watchdog and events support (done)
4. Power management (done)
5. Hardware UART support
6. Trace support
7. Real Time Timer support
8. Software UART support
9. SPI support (ATmega328 first)
10. SPI support (ATtinyX4)
11. NRF24L01P support
12. ... To be determined later
