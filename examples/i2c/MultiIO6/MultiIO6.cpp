//   Copyright 2016-2023 Jean-Francois Poilpret
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
 * Simple example using MCP23008 I2C device (GPIO expander). It simply directly
 * maps switches to LED (4 switches, 4 LEDs).
 * This program uses FastArduino MCP23008 support API, including MCP23008 interrupts
 * to be notified when an input switch changes states.
 * 
 * Wiring:
 * - MCP23008:
 *   - GP4-GP7: each pin is connected to LED through a ~1K resistor to the ground
 *   - GP0-GP3: each pin shall be connected to a DIP switch, itself connected to the ground. 
 *     3 first switches define a "LED pattern" that will progress through the 8 LEDs chain
 *     last switch defines the progress direction of the pattern
 * 
 * NB: you should add pullup resistors (10K-22K typically) on both SDA and SCL lines.
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A4 (PC4, SDA): connected to MCP23008 SDA pin
 *   - A5 (PC5, SCL): connected to MCP23008 SCL pin
 *   - D2 (PD2): connected to MCP23008 INT pin
 */

#include <fastarduino/gpio.h>
#include <fastarduino/int.h>
#include <fastarduino/power.h>
#include <fastarduino/time.h>
#include <fastarduino/uart.h>
#include <fastarduino/i2c_device.h>
#include <fastarduino/i2c_debug.h>
#include <fastarduino/i2c_status.h>
#include <fastarduino/devices/mcp23008.h>

#define DEBUG_I2C
#define FORCE_SYNC

static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)

// UART for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];

static constexpr uint8_t I2C_BUFFER_SIZE = 32;

#ifdef DEBUG_I2C
static constexpr const uint8_t DEBUG_SIZE = 64;
using DEBUGGER = i2c::debug::I2CDebugStatusRecorder<DEBUG_SIZE, DEBUG_SIZE>;
#	if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using MANAGER = i2c::I2CAsyncStatusDebugManager<
	i2c::I2CMode::FAST, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS, DEBUGGER&, DEBUGGER&>;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
#	else
using MANAGER = i2c::I2CSyncStatusDebugManager<i2c::I2CMode::FAST, DEBUGGER&, DEBUGGER&>;
#	endif
#define DEBUG(OUT) debugger_.trace(OUT)
#define RESET_DEBUG() debugger_.reset()

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

#define INT_NUM 0
static constexpr const board::ExternalInterruptPin INT_PIN = board::ExternalInterruptPin::D2_PD2_EXT0;

class SwitchHandler
{
public:
	SwitchHandler(streams::ostream& out)
		: out_{out}, mcp_{manager_, 0x00}, signal_{interrupt::InterruptTrigger::RISING_EDGE}
	{
		out_ << F("SwitchHandler()") << endl;
		interrupt::register_handler(*this);
		manager_.begin();
		out_ << F("I2C interface started") << endl;
		time::delay_ms(100);

		bool ok = mcp_.begin();
		out_ << dec << F("begin() ") << ok << endl;
		DEBUG(out_);
		ok = mcp_.configure_gpio(0x0F, 0x0F);
		out_ << dec << F("configure_gpio() ") << ok << endl;
		DEBUG(out_);
		ok = mcp_.configure_interrupts(0x0F, 0x00, 0x00);
		out_ << dec << F("configure_interrupts() ") << ok << endl;
		DEBUG(out_);
		signal_.enable();
	}

	void loop()
	{
		while (true)
		{
			power::Power::sleep();
			if (changed_)
			{
				changed_ = false;
				uint8_t switches = mcp_.values() & 0x0F;
				out_ << F("switches = 0x") << hex << switches << endl;
				DEBUG(out_);
				bool ok = mcp_.values(switches << 4);
				out_ << dec << F("values() ") << ok << endl;
				DEBUG(out_);
			}
		}
	}

private:
	void on_change()
	{
		changed_ = true;
	}

	using MCP = devices::mcp230xx::MCP23008<MANAGER>;

	streams::ostream& out_;
#ifdef DEBUG_I2C
	DEBUGGER debugger_;
#endif
#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
#	ifdef DEBUG_I2C
	MANAGER manager_{i2c_buffer, debugger_, debugger_};
#	else
	MANAGER manager_{i2c_buffer};
#	endif
#else
#	ifdef DEBUG_I2C
	MANAGER manager_{debugger_, debugger_};
#	else
	MANAGER manager_;
#	endif
#endif
	MCP mcp_;
	interrupt::INTSignal<INT_PIN> signal_;
	volatile bool changed_ = true;

	DECL_INT_ISR_HANDLERS_FRIEND
};

REGISTER_INT_ISR_METHOD(INT_NUM, INT_PIN, SwitchHandler, &SwitchHandler::on_change)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	serial::hard::UATX<UART> uart{output_buffer};
	streams::ostream out = uart.out();
	uart.begin(115200);
	out.width(2);
	out << streams::boolalpha << streams::unitbuf;
	out << F("Start") << endl;

	SwitchHandler handler{out};
	handler.loop();
}
