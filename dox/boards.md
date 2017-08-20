Supported MCU and Arduino boards {#supportedboards}
================================

FastArduino currently supports a certain number of AVR MCU and Arduino boards based on these.

Since each MCU has its specificities, FastArduino defines distinct header files to ensure compile-time checking works fine for the current selected target.

MCU selection at compile-time is done by setting the following pre-processor macros in the command-line of the compiler:

- `VARIANT` which can be one of:
    - `ARDUINO_UNO`
    - `ARDUINO_NANO`
    - `ARDUINO_LEONARDO`
    - `BREADBOARD_ATMEGA328P`
    - `BREADBOARD_ATTINYX4`
    - `ARDUINO_MEGA`
- `F_CPU`

In addition, the following option shall be set at compile time:

- `-mmcu=<MCU>` where <MCU> is the code of the target MCU as understood by the AVR build toolchain:
    - `atmega328`
    - `atmega32u4`
    - `attiny84`
    - `atmega2560`

When using FastArduino makefile, then you first need to create your project under netbeans, generate a proper configuration for it, and pass it to `make`. This is described in further details in [ArduinoDevSetup](https://github.com/jfpoilpret/fast-arduino-lib/blob/master/ArduinoDevSetup.docx?raw=true).

Note that you should **never** include directly a specific board header file (e.g. `#include <fastarduno/boards/uno.h>`) in your own source code, this will not compile properly. Instead, you should simply `#include <fastarduino/boards/board.h>` which will include the proper header file for the board defined by `VARIANT`.

Capabilities of each supported target
-------------------------------------

For each of the following supported targets, you can find the documentation of its specific header file:

- [Arduino UNO](boards/uno/namespaceboard.html)
- [Arduino NANO](boards/nano/namespaceboard.html)
- [ATmega328P based board](boards/atmega328/namespaceboard.html)
- [ATtinyX4 based board](boards/attinyx4/namespaceboard.html)
- [Arduino LEONARDO](boards/leonardo/namespaceboard.html)
- [Arduino MEGA](boards/mega/namespaceboard.html)

Then all standard FastArduino API works on each target and shares this common documentation.

Supporting other targets
------------------------

FastArduino may be added support for other AVR-based targets that share features similar to other AVR MCU: digital IO, PWM, analog inputs, timers, watchdog...

Adding support to a new target MCU or a new specific board embedding a supported MCU is rather easy thanks to FastArduino approach to board configuration, based on "traits".

This is further described later (TODO).


