Adding support to an SPI device {#spidevices}
===============================

There are plenty of devices of all kinds, based on SPI interface, that you may want to connect to your Arduino
or a board you created with an AVR ATmega or ATtiny MCU.

If you want to learn more about SPI concepts and vocabulary, you can find further information on 
[Wikipedia](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface).

Unfortunately, FastArduino obviously cannot provide specific support for all existing SPI devices.

However, based on a given device datasheet, it can be quite easy to add a FastArduino driver for any SPI device.

FastArduino provides all the necessary classes and methods for you to implement such a specific driver.

The following sections describe the FastArduino API for SPI device driver implementation, and list the steps
to successfully implement such a driver.


FastArduino SPI driver API
--------------------------
The generic support for SPI device driver in FastArduino is quite simple, it is entirely embedded in 2 classes:
@image html classspi_1_1_s_p_i_device__inherit__graph.png
@image latex classspi_1_1_s_p_i_device__inherit__graph.pdf

TODOREF: http://jfpoilpret.github.io/fast-arduino-lib/classspi_1_1_s_p_i_device__inherit__graph.png

The important class here is `spi::SPIDevice`, this is a template class (with many parameters discussed later) 
which all actual SPI device drivers shall derive from.

The abstract base class `AbstractSPIDevice` contains most methods used in transferring (both ways) content to/from
a device on the SPI bus. It exists solely for the sake of code size: it factors all methods that do not depend
on any `spi::SPIDevice` template parameter, into a non-template class so that generated code is not repeated for each 
device driver. All its important methods are available directly from `spi::SPIDevice` and its children, as 
`protected` methods.

As you can see in the following diagrams, the drivers for SPI devices currently supported by FastArduino directly
derive from `spi::SPIDevice`, sometimes enforcing template parameter values:

1. Winbond SPI memory chip
@image html classdevices_1_1_win_bond__inherit__graph.png
@image latex classdevices_1_1_win_bond__inherit__graph.pdf

2. NRF24L01 SPI Radio-Frequency transmitter/receiver
@image html classdevices_1_1rf_1_1_n_r_f24_l01__inherit__graph.png
@image latex classdevices_1_1rf_1_1_n_r_f24_l01__inherit__graph.pdf

Hence, creating a new driver for an SPI device is as simple as:
1. Creating a `spi::SPIDevice` subclass; let's call it `MySPIDevice` in the rest of this page.
2. Add proper `public` API on this `MySPIDevice` class, based on actual device features we want to use
3. Implement this API through the basic `protected` API methods inherited from `spi::SPIDevice`

### SPIDevice template parameters ###
The `spi::SPIDevice` template class is instantiated through the following template parameters:
- CS: this `board::DigitalPin` is the most important parameter of the template; it defines on which digital pin 
of the MCU the targeted device "chip select" pin shall be connected; in `MySPIDevice`, this shall remain a template 
parameter because you never know in advance how the device will be connected to the MCU acrodd different projects.
- CS_MODE: this parameter defines if the CS pin is active HIGH or LOW (the default); this is specific to every SPI
device and shall be forced to the proper value in `MySPIDevice` class definition.
- RATE: this parameter fixes the SPI clock frequency to the maximum value supported by the actual device. This is 
actually a divider of the MCU clock, used to provide the actual SPI frequency. Note that the proper selection for this 
template parameter depends on the maximum transfer rate supported by the target device and the MCU frequency used in
your project.
- MODE: one of the 4 SPI modes (as explained 
[here](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface#Clock_polarity_and_phase) and
[there](https://en.wikipedia.org/wiki/Serial_Peripheral_Interface#Mode_numbers)); typically a given device supports
exactly one mode, you should thus enforce the proper mode for your target device.
- ORDER: this parameter defines the the order in which bits of a byte are transferred: 
MSB (most significant bit) first, or LSB (least significant bit) first; typically a given device supports
exactly one bit transfer order, you should thus enforce the proper order for your target device.

### SPIDevice API ###
Subclassing `spi::SPIDevice` gives `MySPIDevice` access to all low-level `protected` methods:
- `spi::SPIDevice.start_transfer()`: any SPI transfer to the slave SPI device must start with this call. Such a 
transfer must end by calling `spi::SPIDevice.end_transfer()`.
- `spi::SPIDevice.transfer(uint8_t)`: send one byte to the slave SPI device and return the byte that was received
during transmission (SPI is a full-duplex protocol where master and slave can send data at the same time).
- `spi::SPIDevice.transfer(uint8_t*, uint16_t)`: send a packet of bytes to the slave SPI device, and receive all bytes
simultaneously transmitted by that device.
- `spi::SPIDevice.transfer(const uint8_t*, uint16_t)`: send a packet of bytes to the slave SPI device, but trash any
bytes simultaneously transmitted by that device.
- `spi::SPIDevice.end_transfer()`: finished the current transfer to the slave SPI device, that was initiated with
`spi::SPIDevice.start_transfer()`.

Any feature implementation in `MySPIDevice` will always consist in a sequence of calls to the methods above, like:
@code
    this->start_transfer();
    this->transfer(0x01);
    uint8_t result1 = this->transfer(0x80);
    uint8_t result2 = this->transfer(0x00);
    this->end_transfer();
@endcode

Most SPI devices use codes to perform various features, either write-only (sending values to the device) or read-only
(asking the device for some values).

In the sections below, we will sometimes refer to a simple SPI device, the 
[MCP3008](http://ww1.microchip.com/downloads/en/DeviceDoc/21295C.pdf), an 8-channel Analog-Digital Converter, which
communication protocol is super simple (because the number of features for such a chip is quite limited).


Debugging support for a new device (low-level)
----------------------------------------------

In general, before developing a full-fledged driver for an SPI device, you need to learn how to use that device.

Based on the device datasheet, you first learn how to manipulate the device through the SPI bus.

For better understanding, you generally use a debugging example that helps demonstrate how the device works. 

One easy way to develop such a debugging sample is to create a program with just one source code file containing:
- proper `#include` directives
- a `PublicDevice` class that derives from `spi::SPIDevice` but declares `main()` as a `friend`, which allows direct 
calls, from `main()`, to `protected` API of `spi::SPIDevice`, for easy testing
- directly call SPI API on a `PublicDevice` instance, from `main()` and trace results to a console, through UART

FastArduino includes such a debugging sample in `examples/spi/SPIDeviceProto` example, copied hereafter:

TODO include example here and explain how to use.

This example demonstrates how to simply test the MCP3008 ADC chip.


Defining the driver API based on device features
------------------------------------------------

At this level, you have already been able to debug how the device works and you have a good overview of what
features you want to provide to developers (and you as the first of all) who will want to use this device.

An easy way is to provide an API that maps every feature found in the datasheet to its dedicated method. This is what
we would call a low-level API; that is the minimum your driver should provide.

Additionally `MySPIDevice` might implement a higher level API, based on the low-level one, but this is not
mandatory; actually, this is not even advised generally, as this high-level API might be implemented in a distinct
class. Using a separate class for high-level API allows other developers to develop their own high-level API without
having to use yours if it does not fit their needs.

It is often advised to add `begin()` and `end()` methods to `MySPIDevice` when it makes sense. `begin()` would
initialize the device before usage (most devices will require special setup before use).

In the MCP3008 example, the API could be rather simple; for instance, we could:
- define an `enum` for the selection of channel to read (including single-ended Vs. differential input modes)
- define a template class `MCP3008Device` as device driver keeping only `CS` as template parameter
- add only one API method `uint16_t read_channel()`


Implementing the driver API
---------------------------

We proceed with the MCP3008 example. When implementing the API, you must scrupulously follow the device datasheet
for every method!

Here is a simple implementation attempt for MCP3008 driver:

@code
enum class MCP3008Channel : uint8_t
{
    // singled-ended input
    CH0 = 0x80,
    CH1 = 0x90,
    CH2 = 0xA0,
    CH3 = 0xB0,
    CH4 = 0xC0,
    CH5 = 0xD0,
    CH6 = 0xE0,
    CH7 = 0xF0,
    // differential input
    CH0_CH1 = 0x00,
    CH1_CH0 = 0x10,
    CH2_CH3 = 0x20,
    CH3_CH2 = 0x30,
    CH4_CH5 = 0x40,
    CH5_CH4 = 0x50,
    CH6_CH7 = 0x60,
    CH7_CH6 = 0x70
};

using namespace spi;

template<board::DigitalPin CS>
class MCP3008Device : public SPIDevice<CS, ChipSelect::ACTIVE_LOW, compute_clockrate(3600000UL), Mode::MODE_0, DataOrder::MSB_FIRST>
{
public:
    MCP3008Device() = default;

    uint16_t read_channel(MCP3008Channel channel)
    {
        this->start_transfer();
        this->transfer(0x01);
        uint8_t result1 = this->transfer(uint8_t(channel));
        uint8_t result2 = this->transfer(0x00);
        this->end_transfer();
		return utils::as_uint16_t(result1 & 0x03, result2);
    }
}
@endcode

Note the implementation of `read_channel()` which is mainly the same as in the debugging example described earlier.

Of course, the MCP3008 is a very simple device which is easy to interact with through SPI, but there are many SPI
devices (cameras, B&W and color display controllers, RF transmitters...) 
For those devices, the number of features can be large and this would result in dozens or even hundreds of API methods! 


Support for ATtiny MCU
----------------------

ATtiny MCU provide some support (through its USI feature) for SPI but it is quite limited in comparison to ATmega devices; 
hence FastArduino SPI support for ATtiny chips has derived limitations:

1. The only `spi::DataOrder` supported is `spi::DataOrder::MSB_FIRST`
2. The only `spi::Mode` supported are `spi::Mode::MODE_0` and `spi::Mode::MODE_1`
3. `spi::ClockRate` parameter is not used in `spi::SPIDevice` implementation, hence the maximum clock rate
is always used, and is roughly equal to `CPU frequency / 7`, hence typically a bit more than 1MHz for common
clock frequency used in ATtiny boards (internal RC clock); this might be a problem for devices supporting only 1MHz SPI.

These limitations might prevent proper support, on ATtiny MCU, of some SPI devices.

If your device is in this situation, then you should add compile error checks (through `static_assert()`, or `#if` 
and `#error`)) in your SPI device driver header file, so that it cannot compile for these unsupported ATtiny targets.


The last mile: add driver to FastArduino project!
-------------------------------------------------

Bravo! You successfully added FastArduino support, in your own project, for a specific SPI device!

The last mile would now consist in adding your valuable work to FastArduino library! 
You do not have to, of course, but this a good way to:
- thank other people who provided FastArduino open source library to you
- feel part of the community
- get feedback on your work, potentially allowing it to be further improved
- share your work with the rest of the world

However, like for marathon, the last mile is often very difficult! In order to run this last mile, you will have to:
- first accept FastArduino Apache License 2.0 for your contribution, or discuss with FastArduino owner for another one, 
if compatible
- follow FastArduino coding guidelines: this might impose some code rewrite or reformatting
- add API documentation with doxygen: this is mandatory for all `public` methods, and advised for `protected` ones. 
- add one (or more) usage example and integrate it in the `examples/spi` directory; examples must be kept simple but
still demonstrate the API usage; example circuits (connection pins) shall be described. These examples can be further
used as "tests" before new releases of FastArduino.
- optionally develop a tutorial for this device
- prepare and propose a PR to FastArduino project

**Important condition**: in order to accept merging a PR to FastArduino, I must be able to check it by myself, hence
I need to first have the new supported device available on my workbench; I will buy one (or a few) if it is affordable
and easy to find.
