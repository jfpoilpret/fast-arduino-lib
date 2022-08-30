//   Copyright 2016-2022 Jean-Francois Poilpret
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
 * Check HMC5883L I2C device (3D compass) and display output to the UART console.
 * This program uses FastArduino HMC5883L support API.
 * 
 * Wiring:
 * NB: you should add pullup resistors (10K-22K typically) on both SDA and SCL lines.
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A4 (PC4, SDA): connected to HMC5883L SDA pin
 *   - A5 (PC5, SCL): connected to HMC5883L SCL pin
 *   - D2 (PD2, EXT0): connected to HMC5883L DRDY pin
 *   - direct USB access
 * - on Arduino LEONARDO:
 *   - D2 (PD1, SDA): connected to HMC5883L SDA pin
 *   - D3 (PD0, SCL): connected to HMC5883L SCL pin
 *   - D7 (PE7, EXT6): connected to HMC5883L DRDY pin
 *   - direct USB access
 * - on Arduino MEGA:
 *   - D20 (PD1, SDA): connected to HMC5883L SDA pin
 *   - D21 (PD0, SCL): connected to HMC5883L SCL pin
 *   - D18 (PD3, EXT3): connected to HMC5883L DRDY pin
 *   - direct USB access
 * - on ATtinyX4 based boards:
 *   - D6 (PA6, SDA): connected to HMC5883L SDA pin
 *   - D4 (PA4, SCL): connected to HMC5883L SCL pin
 *   - D8 (PB0, TX): connected to SerialUSB converter
 *   - D10 (PB2, EXT0): connected to HMC5883L DRDY pin
 * - on ATmega644 based boards:
 *   - D17 (PC1, SDA): connected to HMC5883L SDA pin
 *   - D16 (PC0, SCL): connected to HMC5883L SCL pin
 *   - D25 (PD1): TX output connected to SerialUSB converter
 *   - D26 (PD2, EXT0): connected to HMC5883L DRDY pin
 */

#include <fastarduino/int.h>
#include <fastarduino/time.h>
#include <fastarduino/i2c_handler.h>
#include <fastarduino/devices/hmc5883l.h>
#include <fastarduino/i2c_debug.h>
#include <fastarduino/i2c_status.h>

#if defined(ARDUINO_UNO) || defined(ARDUINO_NANO) || defined(BREADBOARD_ATMEGA328P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::ExternalInterruptPin DRDY = board::ExternalInterruptPin::D2_PD2_EXT0;
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
#define INT_NUM 0
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_LEONARDO)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::ExternalInterruptPin DRDY = board::ExternalInterruptPin::D7_PE6_EXT6;
static constexpr const board::USART UART = board::USART::USART1;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
#define INT_NUM 6
// Define vectors we need in the example
REGISTER_UATX_ISR(1)
#elif defined(ARDUINO_MEGA)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::ExternalInterruptPin DRDY = board::ExternalInterruptPin::D18_PD3_EXT3;
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
#define INT_NUM 3
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined(BREADBOARD_ATTINYX4)
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::ExternalInterruptPin DRDY = board::ExternalInterruptPin::D10_PB2_EXT0;
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
#define INT_NUM 0
#elif defined (BREADBOARD_ATMEGAXX4P)
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::ExternalInterruptPin DRDY = board::ExternalInterruptPin::D26_PD2_EXT0;
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr uint8_t I2C_BUFFER_SIZE = 32;
#define INT_NUM 0
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

#if HARDWARE_UART
	REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)
#else
	REGISTER_OSTREAMBUF_LISTENERS(serial::soft::UATX<TX>)
#endif

// #define DEBUG_I2C
#define FORCE_SYNC

// UART for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];
#if HARDWARE_UART
static serial::hard::UATX<UART> uart{output_buffer};
#else
static serial::soft::UATX<TX> uart{output_buffer};
#endif
static streams::ostream out = uart.out();

using devices::magneto::DataOutput;
using devices::magneto::Gain;
using devices::magneto::Sensor3D;
using devices::magneto::MeasurementMode;
using devices::magneto::OperatingMode;
using devices::magneto::SamplesAveraged;

#ifdef DEBUG_I2C
static constexpr const uint8_t DEBUG_SIZE = 32;
using DEBUGGER = i2c::debug::I2CDebugStatusRecorder<DEBUG_SIZE, DEBUG_SIZE>;
#	if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using MANAGER = i2c::I2CAsyncStatusDebugManager<
	i2c::I2CMode::FAST, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, DEBUGGER&, DEBUGGER&>;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#	else
using MANAGER = i2c::I2CSyncStatusDebugManager<i2c::I2CMode::FAST, DEBUGGER&, DEBUGGER&>;
#	endif
#define DEBUG(OUT) debugger.trace(OUT)
#define RESET_DEBUG() debugger.reset()

#else

#	if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using MANAGER = i2c::I2CAsyncManager<
	i2c::I2CMode::FAST, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS>;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#	else
using MANAGER = i2c::I2CSyncManager<i2c::I2CMode::FAST>;
#	endif
#define DEBUG(OUT)
#define RESET_DEBUG()

#endif

#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
REGISTER_I2C_ISR(MANAGER)
#endif
REGISTER_FUTURE_NO_LISTENERS()

using streams::dec;
using streams::hex;
using streams::endl;

void trace_fields(const Sensor3D& fields)
{
	out << dec << F("x=") << fields.x << F(",y=") << fields.y << F(",z=") << fields.z << endl;
}

// This handler gets notified when HMC5883L data is ready to read
class DataReadyHandler
{
public:
	DataReadyHandler() : ready_{false}
	{
		interrupt::register_handler(*this);
	}

	void reset()
	{
		ready_ = false;
	}

	bool ready() const
	{
		return ready_;
	}

private:
	void data_ready()
	{
		ready_ = true;
	}

	volatile bool ready_;

	DECL_INT_ISR_HANDLERS_FRIEND
};

REGISTER_INT_ISR_METHOD(INT_NUM, DRDY, DataReadyHandler, &DataReadyHandler::data_ready)

using MAGNETOMETER = devices::magneto::HMC5883L<MANAGER>;

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	uart.begin(115200);
	out << F("Start") << endl;
	
	// Initialize I2C async handler
#ifdef DEBUG_I2C
	DEBUGGER debugger;
#endif

#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
#	ifdef DEBUG_I2C
	MANAGER manager{i2c_buffer, debugger, debugger};
#	else
	MANAGER manager{i2c_buffer};
#	endif
#else
#	ifdef DEBUG_I2C
	MANAGER manager{debugger, debugger};
#	else
	MANAGER manager;
#	endif
#endif

	manager.begin();
	out << F("I2C interface started") << endl;
	
	DataReadyHandler handler;
	interrupt::INTSignal<DRDY> signal{interrupt::InterruptTrigger::RISING_EDGE};
	signal.enable();
	MAGNETOMETER compass{manager};
	
	bool ok = compass.begin(
		OperatingMode::CONTINUOUS, Gain::GAIN_4_0GA, DataOutput::RATE_0_75HZ, SamplesAveraged::EIGHT_SAMPLES);
	out << dec << F("begin() ") << ok << endl;
	DEBUG(out);
	while (true)
	{
		while (!handler.ready()) time::yield();
		handler.reset();

		Sensor3D fields{};
		ok = compass.magnetic_fields(fields);
		DEBUG(out);
		compass.convert_fields_to_mGA(fields);
		trace_fields(fields);
	}
	
	// Stop TWI interface
	//===================
	manager.end();
	out << F("End") << endl;
}
