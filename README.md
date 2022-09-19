Latest News
===========

FastArduino v1.8 has been released on 10th October 2021.
The implemented enhancements are listed [here](https://github.com/jfpoilpret/fast-arduino-lib/milestone/10?closed=1).
Please note that this version breaks compatibility on I2C API, as described in further detail in the [release notes](https://github.com/jfpoilpret/fast-arduino-lib/releases/tag/v1.8).

FastArduino
===========

FastArduino is a C++ object-oriented library for Arduino boards based on AVR MCU and also for bare AVR MCU.

FastArduino API is fully documented [here](http://jfpoilpret.github.io/fast-arduino-lib/) and documentation also includes a complete [tutorial](http://jfpoilpret.github.io/fast-arduino-lib/tutorial.html).

FastArduino C++ code is also analyzed by SonarQube and results of this analysis are published from time to time [here](https://sonarcloud.io/dashboard?id=FastArduino-UNO).

FastArduino benefits are:

- provide smaller and faster code than other libraries for the same functionality
- ensure you pay (size and speed) only for what you use
- use modern C++ Object-Oriented Design everywhere
- reduce risk of bad code, e.g. by preventing usage, at compile time, of unexisting features (pins, timers...) for the target MCU
- support event-driven programs
- support both ATmega and ATtiny chips

From my viewpoint, the main issues with other third-party Arduino libraries are essentially heavy usage of `virtual` methods, which quickly increases code size when you start to define deep classes hierarchies; this also can have a slight impact on speed due to additional indirection when calling methods. Calling `virtual` methods **from an ISR** also has a big impact on code size as the generated code for ISR will push all registers to the stack, then call your ISR code, and finally pop all registers back from the stack; of course, this also has a dramatic impact on ISR execution speed. Avoiding `virtual` methods calls from an ISR ensures the compiler generates only the strict minimum of push and pop necessary.

FastArduino tries to favour C++ templates rather than virtual methods whenever possible; in the very rare locations where virtual methods are used, their number is reduced to the minimum needed (abstract virtual methods only, typically used for event handlers, generally limited to hierarchy of 2 levels only, one abstract parent and direct children). 

Also, no ISR gets automatically declared by FastArduino: every program declares the ISR it needs by using pre-defined FastArduino ISR-registration macros (note that ISR registration is the only feature for which FastArduino uses macros). FastArduino does not use `virtual` methods for ISR callbacks, thus permitting optimization of ISR code size, which would not have been possible with `virtual` methods as callbacks.

Note that, if you consider using FastArduino for your projects, you should be aware that FastArduino does not support Arduino API and does not intend to do so some day. That means you will have to first learn FastArduino API (you can use the complete [tutorial](http://jfpoilpret.github.io/fast-arduino-lib/tutorial.html) and the numerous examples provided for that) in order to reap its benefits. FastArduino is definitely not for newcomers to C++ programming as it makes heavy use of C++ specificities. FastArduino currently uses C++17 standard.

Since the initial drafts, I decided not to be compliant with Arduino IDE as I find it is a real pain. All my projects (including FastArduino itself) are now built with [Visual Studio Code](https://code.visualstudio.com/) along with a [small extension](https://github.com/jfpoilpret/vscode-fastarduino) I developed specifically for FastArduino. Once properly setup, I find VS Code environment very easy and friendly to use.

FastArduino is also buildable from the command line (on a linux system) through the standard `make`. Its make system can also be used for projects using the FastArduino library.

Making FastArduino buildable on Arduino IDE is not on the roadmap.

My complete setup is described in [ArduinoDevSetup.docx](basedoc/ArduinoDevSetup.docx). This document also describes how I setup Visual Studio Code for my projects.

One easy way to start a new project using FastArduino is to checkout [this project template](https://github.com/jfpoilpret/fastarduino-project-template).

Limitations
===========

As of 07.08.2022 (future version 1.9), FastArduino prevents usage of dynamic allocation (`new` and `delete`) because it is bad for AVR MCU with such limited amount of SRAM, as it may lead to heap fragmentation and ultimately program crash.

FastArduino has always encouraged static allocation only throughout its API and implementation.

Allowing dynamic allocation would have undesirable effects on classes with virtual methods: they should also define a virtual destructor, which has an impact on generated code and data (vtable) size.
Hence, we have decided to not define virtual destructors in such FastArduino classes, and prevent overall dynamic allocation to avoid virtual destructors.

Status
======

Latest FastArduino release [**v1.8**](https://github.com/jfpoilpret/fast-arduino-lib/releases/tag/v1.8) has been published on 10.10.2021.

In the current version, the project covers almost all features; a few missing, less important, features will be released in future versions (see roadmap below); the API of current features is deemed stable and should not change in the future.

What the library has:

- General utilities (queues, linked lists, busy loop delays)
- "Future" support for asynchronous computation (e.g. ISR-based)
- Fast IO support: template-based, optimized for speed and size
- Analog Digital Conversion support (in Single Conversion mode)
- Analog Comparator support
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
- I2C master support with both synchronous and asynchronous modes supported (asynchronous only for ATmega MCU)
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
- MCP23008 8-Bit I/O Expander (I2C-based)
- MCP23017 16-Bit I/O Expander (I2C-based)
- MCP3008 8-channel Analog-Digital Converter (SPI-based) and other chips from the same family
- HC-SR04 sonar device support in synchronous and asynchronous modes
- VL53L0X laser distance sensor device (I2C-based)
- Grove 125KHz RFID reader (version 1.9-alpha)

As of now, the following platforms are supported (and tested):

- Arduino UNO
- Arduino NANO
- Arduino MEGA
- Arduino Leonardo (with only partial USB support)
- Breadboard ATmega328 at 8MHz and 16MHz
- Breadboard ATmega164/324/644/1284 at 8MHz and 16MHz (note: only 644 and 1284 were actually tested)
- Breadboard ATtiny84 at 8MHz
- Breadboard ATtiny85 at 8MHz

I intend to probably add support some day for:

- Arduino Leonardo with full USB support

A [step-by-step tutorial](http://jfpoilpret.github.io/fast-arduino-lib/tutorial.html) is available for those who want to learn FastArduino API; it covers the whole FastArduino core API. In addition to that, the [FastArduino API](http://jfpoilpret.github.io/fast-arduino-lib/) is fully documented through [doxygen](http://www.stack.nl/~dimitri/doxygen/).

Project structure
=================

The project is organized according to the following structure:
- `/`: root project directory
  - `cores/fastarduino`: FastArduino platform C++ source code
  - `make`: utility makefiles and scripts to build FastArduino library, generate docs, and prepare releases
  - `examples`: source code of basic examples of all FastArduino features, gathered by categories
  - `.vscode`: project settings for Visual Studio Code
  - `docs`: created by doxygen, contains all docs generated in HTML format, published [here](http://jfpoilpret.github.io/fast-arduino-lib/)
  - `basedoc`: other various project documentation
  - `dox`: doxygen documentation source files and settings
  - `tuto-samples`: source code of samples used in tutorial; samples are often present in 2 flavours, Arduino API and FastArduino, in order to compare their respective sizes
  - `example-boards`: contains schemas (produced with LochMaster software) of stripboards that I use for FastArduino runtime tests 
  - `refs`: contains datasheets for AVR MCU and external devices supported by FastArduino

  - `build`: created at build time, contains object files
  - `deps`: created at build time, contains all source code dependencies
  - `dist`: created at build time, contains FastArduino library for various targets

  - `apidocs`: created by doxygen, contains docs generated in LATEX format

Roadmap
=======

The roadmap of next activities and new supported features is the following:

- [Milestone v1.0](https://github.com/jfpoilpret/fast-arduino-lib/milestone/1?closed=1) (released on 22.02.2018)
- [Milestone v1.1](https://github.com/jfpoilpret/fast-arduino-lib/milestone/2?closed=1) (released on 26.04.2019)
- [Milestone v1.2](https://github.com/jfpoilpret/fast-arduino-lib/milestone/4?closed=1) (released on 06.06.2019)
- [Milestone v1.3](https://github.com/jfpoilpret/fast-arduino-lib/milestone/5?closed=1) (released on 1.09.2019)
- [Milestone v1.4](https://github.com/jfpoilpret/fast-arduino-lib/milestone/6?closed=1) (released on 22.09.2019)
- [Milestone v1.5](https://github.com/jfpoilpret/fast-arduino-lib/milestone/7?closed=1) (released on 18.01.2020)
- [Milestone v1.6](https://github.com/jfpoilpret/fast-arduino-lib/milestone/8?closed=1) (released on 22.11.2020)
- [Milestone v1.7](https://github.com/jfpoilpret/fast-arduino-lib/milestone/9?closed=1) (released on 19.09.2021)
- [Milestone v1.8](https://github.com/jfpoilpret/fast-arduino-lib/milestone/10?closed=1) (release on 10.10.2021)
- [Milestone v1.9](https://github.com/jfpoilpret/fast-arduino-lib/milestone/11) (to be released on 30.09.2021)
- [Milestone v2.0](https://github.com/jfpoilpret/fast-arduino-lib/milestone/3) (undefined date)
- [Later](https://github.com/jfpoilpret/fast-arduino-lib/issues?q=is%3Aopen+is%3Aissue+no%3Amilestone)

Documentation - no milestones
-----------------------------
- Document how to add support for other boards in Tutorial (1st half 2022)

Milestones dates are "best effort" and may change based on contributors' availability.

The following features are not in FastArduino roadmap currently (but may be in the future):

- OWI support

