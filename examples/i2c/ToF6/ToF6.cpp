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
 * Simple ranger example, using VL53L0X Time-of-flight range sensor I2C device.
 * This program uses FastArduino VL53L0X support API.
 * It checks conditions of use of GPIO pin.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A4 (PC4, SDA): connected to VL53L0X SDA pin
 *   - A5 (PC5, SCL): connected to VL53L0X SCL pin
 *   - D2 (EXT0): connected to VL53L0X GPIO pin
 */

#include <fastarduino/gpio.h>
#include <fastarduino/int.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/time.h>
#include <fastarduino/uart.h>
#include <fastarduino/i2c_handler.h>
#include <fastarduino/devices/vl53l0x.h>

// #define FORCE_SYNC

static constexpr const i2c::I2CMode MODE = i2c::I2CMode::FAST;
static constexpr const board::ExternalInterruptPin GPIO = board::ExternalInterruptPin::D2_PD2_EXT0;

static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

REGISTER_RTT_ISR(0)
REGISTER_UATX_ISR(0)

#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
using MANAGER = i2c::I2CAsyncManager<MODE, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS>;
static constexpr const uint8_t I2C_BUFFER_SIZE = 32;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];
REGISTER_I2C_ISR(MANAGER)
#else
using MANAGER = i2c::I2CSyncManager<MODE>;
#endif

using streams::dec;
using streams::hex;
using streams::fixed;
using streams::endl;
using streams::flush;

using namespace devices::vl53l0x;
using TOF = VL53L0X<MANAGER>;

#define CHECK_OK(STATEMENT) if (! STATEMENT) out << F(#STATEMENT) << F(" ERROR!") << endl

class ToFController
{
public:
	ToFController(TOF& tof) : tof_{tof}
	{
		interrupt::register_handler(*this);
		signal_.enable();
	}

	void await_gpio()
	{
		while (!gpio_)
		{
			time::yield();
		}
		gpio_ = false;
	}

private:
	void gpio_raised()
	{
		gpio_ = true;
	}

	TOF& tof_;
	gpio::FAST_EXT_PIN<GPIO> gpio_pin_{gpio::PinMode::INPUT};
	interrupt::INTSignal<GPIO> signal_{interrupt::InterruptTrigger::FALLING_EDGE};
	volatile bool gpio_ = false;

	DECL_INT_ISR_HANDLERS_FRIEND
};

REGISTER_INT_ISR_METHOD(0, GPIO, ToFController, &ToFController::gpio_raised)

static void loop_gpio(
	streams::ostream& out, timer::RTT<board::Timer::TIMER0>& rtt, ToFController& controller, TOF& tof)
{
	out << F("await GPIO") << endl;
	// Limit loop to 1'
	for (uint8_t i = 0; i < 30; ++i)
	{
		uint16_t range = 0;
		rtt.millis(0);
		controller.await_gpio();
		time::RTTTime time1 = rtt.time();

		rtt.millis(0);
		CHECK_OK(tof.get_direct_range(range));
		time::RTTTime time2 = rtt.time();
		// Clear interrupt
		CHECK_OK(tof.clear_interrupt());

		out << F("GPIO after ") << dec << time1.millis() << "ms " << time1.micros() << "us" << streams::endl;
		out << F("Range after ") << time2.millis() << "ms " << time2.micros() << "us" << streams::endl;
		out << F("range = ") << range << F("mm") << endl;
	}
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	// open UART for traces
	serial::hard::UATX<UART> uart{output_buffer};
	streams::ostream out = uart.out();
	uart.begin(115200);
	out << streams::boolalpha;

	timer::RTT<board::Timer::TIMER0> rtt;
	rtt.begin();

	// Initialize I2C async handler
#if I2C_TRUE_ASYNC and not defined(FORCE_SYNC)
	MANAGER manager{i2c_buffer};
#else
	MANAGER manager;
#endif

	TOF tof{manager};
	manager.begin();
	ToFController controller{tof};

	uint8_t value = 0;
	CHECK_OK(tof.get_register<Register::GPIO_HV_MUX_ACTIVE_HIGH>(value));
	out << F("Register::GPIO_HV_MUX_ACTIVE_HIGH = ") << hex << value << endl;

	CHECK_OK(tof.begin(Profile::STANDARD));
	CHECK_OK(tof.get_register<Register::GPIO_HV_MUX_ACTIVE_HIGH>(value));
	out << F("Register::GPIO_HV_MUX_ACTIVE_HIGH = ") << hex << value << endl;

	out << F("GPIOSettings::sample_ready()") << endl;
	CHECK_OK(tof.set_GPIO_settings(GPIOSettings::sample_ready()));
	CHECK_OK(tof.get_register<Register::GPIO_HV_MUX_ACTIVE_HIGH>(value));
	out << F("Register::GPIO_HV_MUX_ACTIVE_HIGH = ") << hex << value << endl;
	CHECK_OK(tof.start_continuous_ranging(1000));
	loop_gpio(out, rtt, controller, tof);

	out << F("GPIOSettings::low_threshold(200)") << endl;
	CHECK_OK(tof.set_GPIO_settings(GPIOSettings::low_threshold(200)));
	CHECK_OK(tof.get_register<Register::GPIO_HV_MUX_ACTIVE_HIGH>(value));
	out << F("Register::GPIO_HV_MUX_ACTIVE_HIGH = ") << hex << value << endl;
	CHECK_OK(tof.start_continuous_ranging(1000));
	loop_gpio(out, rtt, controller, tof);

	out << F("GPIOSettings::high_threshold(400)") << endl;
	CHECK_OK(tof.set_GPIO_settings(GPIOSettings::high_threshold(400)));
	CHECK_OK(tof.get_register<Register::GPIO_HV_MUX_ACTIVE_HIGH>(value));
	out << F("Register::GPIO_HV_MUX_ACTIVE_HIGH = ") << hex << value << endl;
	CHECK_OK(tof.start_continuous_ranging(1000));
	loop_gpio(out, rtt, controller, tof);

	out << F("GPIOSettings::out_of_window(200, 400)") << endl;
	CHECK_OK(tof.set_GPIO_settings(GPIOSettings::out_of_window(200, 400)));
	CHECK_OK(tof.get_register<Register::GPIO_HV_MUX_ACTIVE_HIGH>(value));
	out << F("Register::GPIO_HV_MUX_ACTIVE_HIGH = ") << hex << value << endl;
	CHECK_OK(tof.start_continuous_ranging(1000));
	loop_gpio(out, rtt, controller, tof);

	tof.stop_continuous_ranging();
	manager.end();

	out << F("Finished!") << endl;
}
