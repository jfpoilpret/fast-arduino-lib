//   Copyright 2016-2020 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/*
 * This is a program to help test FastArduino I2C bus support implementation.
 * It tries to connect to a ghost device (no suchd evice on the bus) and check
 * that I2C status is correct in this situation.
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - A4 (PC4, SDA): connected to pullup resistor (10K-22K)
 *   - A5 (PC5, SCL): connected to pullup resistor (10K-22K)
 *   - direct USB access (traces output)
 */

#include <fastarduino/array.h>
#include <fastarduino/i2c_device.h>
#include <fastarduino/uart.h>
#include <fastarduino/i2c_debug.h>

// I2C Device specific constants go here
//======================================
static constexpr const i2c::I2CMode MODE = i2c::I2CMode::FAST;
static constexpr const uint8_t DEVICE_ADDRESS = 0x77 << 1;

static constexpr const uint8_t DEBUG_SIZE = 32;
using DEBUGGER = i2c::debug::I2CDebugStatusRecorder<DEBUG_SIZE, DEBUG_SIZE>;
using MANAGER = i2c::I2CSyncStatusDebugManager<MODE, DEBUGGER&, DEBUGGER&>;
#define DEBUG(OUT) debugger.trace(OUT, false)

// Define vectors we need in the example
REGISTER_UATX_ISR(0)

// UART for traces
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

// Subclass I2CDevice to make protected methods available
class FakeDevice: public i2c::I2CDevice<MANAGER>
{
	// The following type aliases will be useful for declaring proper Futures and calling I2CDevice API
	using PARENT = i2c::I2CDevice<MANAGER>;
	template<typename T> using PROXY = typename PARENT::template PROXY<T>;
	template<typename OUT, typename IN> using FUTURE = typename PARENT::template FUTURE<OUT, IN>;

public:
	FakeDevice(MANAGER& manager): PARENT{manager, DEVICE_ADDRESS, i2c::Mode<MODE>{}, true} {}

	class WriteRegister : public FUTURE<void, containers::array<uint8_t, 2>>
	{
		using PARENT = FUTURE<void, containers::array<uint8_t, 2>>;
	public:
		WriteRegister(uint8_t address, uint8_t value) : PARENT{{address, value}} {}
		WriteRegister(WriteRegister&&) = default;
		WriteRegister& operator=(WriteRegister&&) = default;
	};

	int write_register(PROXY<WriteRegister> future)
	{
		return this->launch_commands(future, {this->write()});
	}
	bool write_register(uint8_t address, uint8_t value)
	{
		WriteRegister future{address, value};
		if (write_register(PARENT::make_proxy(future)) != 0) return false;
		return (future.await() == future::FutureStatus::READY);
	}

	class ReadRegister : public FUTURE<uint8_t, uint8_t>
	{
		using PARENT = FUTURE<uint8_t, uint8_t>;
	public:
		ReadRegister(uint8_t address) : PARENT{address} {}
		ReadRegister(ReadRegister&&) = default;
		ReadRegister& operator=(ReadRegister&&) = default;
	};

	int read_register(PROXY<ReadRegister> future)
	{
		return this->launch_commands(future, {this->write(), this->read()});
	}
	bool read_register(uint8_t address, uint8_t& value)
	{
		ReadRegister future{address};
		if (read_register(PARENT::make_proxy(future)) != 0) return false;
		return future.get(value);
	}
};

using streams::endl;
using streams::dec;
using streams::hex;

int main()
{
	board::init();
	sei();
	
	serial::hard::UATX<board::USART::USART0> uart{output_buffer};
	streams::ostream out = uart.out();
	uart.begin(115200);
	out.width(2);
	out << streams::boolalpha;
	
	// Start TWI interface
	//====================
	DEBUGGER debugger{};
	MANAGER manager{debugger, debugger};
	manager.begin();
	out << F("I2C interface started") << endl;
	
	FakeDevice device{manager};
	
	// Simulate write register
	bool ok = device.write_register(0x35, 0x23);
	out << F("write_register() = ") << ok << endl;
	DEBUG(out);

	// Simulate read register
	uint8_t value = 0;
	ok = device.read_register(0x35, value);
	out << F("read_register() = ") << ok << endl;
	out << F("value = ") << hex << value << endl;
	DEBUG(out);

	// Stop TWI interface
	//===================
	manager.end();
	out << F("End") << endl;
}
