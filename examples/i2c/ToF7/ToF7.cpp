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
 * Ranger example, using 2 VL53L0X Time-of-flight range sensor I2C devices.
 * This program uses FastArduino VL53L0X support API.
 * It checks I2C address change and also GPIO pins.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A4 (PC4, SDA): connected to VL53L0X SDA pins
 *   - A5 (PC5, SCL): connected to VL53L0X SCL pins
 *   - A0 (PC0, PCI1): connected to VL53L0X GPIO pin 1
 *   - A2 (PC2, PCI1): connected to VL53L0X GPIO pin 2
 *   - A1 (PC1): connected to VL53L0X SHDN pin 1
 *   - A3 (PC3): connected to VL53L0X SHDN pin 2
 */

#include <fastarduino/gpio.h>
#include <fastarduino/pci.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/time.h>
#include <fastarduino/uart.h>
#include <fastarduino/i2c_handler.h>
#include <fastarduino/devices/vl53l0x.h>

// #define FORCE_SYNC

static constexpr const i2c::I2CMode MODE = i2c::I2CMode::FAST;

static constexpr const board::InterruptPin GPIO1 = board::InterruptPin::A0_PC0_PCI1;
static constexpr const board::DigitalPin SHDN1 = board::DigitalPin::A1_PC1;

static constexpr const board::InterruptPin GPIO2 = board::InterruptPin::A2_PC2_PCI1;
static constexpr const board::DigitalPin SHDN2 = board::DigitalPin::A3_PC3;

static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

REGISTER_RTT_ISR(0)
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)

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

REGISTER_FUTURE_STATUS_LISTENERS(MANAGER_FUTURE(MANAGER), i2c::I2CSameFutureGroup<MANAGER>, TOF::SetGPIOSettingsFuture)
REGISTER_FUTURE_OUTPUT_NO_LISTENERS()

#define CHECK_OK(STATEMENT) if (! STATEMENT) out << F(#STATEMENT) << F(" ERROR!") << endl

class ToFController
{
public:
	ToFController()
	{
		interrupt::register_handler(*this);
		signal_.enable_pin<GPIO1>();
		signal_.enable_pin<GPIO2>();
		signal_.enable();
	}

	uint8_t await_gpio()
	{
		while (!gpio_)
		{
			time::yield();
		}
		uint8_t trigger = gpio_;
		gpio_ = 0;
		return trigger;
	}

private:
	void gpio_raised()
	{
		if (!gpio1_pin_.value()) gpio_ |= 0x01;
		if (!gpio2_pin_.value()) gpio_ |= 0x02;
	}

	gpio::FAST_INT_PIN<GPIO1> gpio1_pin_{gpio::PinMode::INPUT};
	gpio::FAST_INT_PIN<GPIO2> gpio2_pin_{gpio::PinMode::INPUT};
	interrupt::PCI_SIGNAL<GPIO1> signal_{};
	volatile uint8_t gpio_ = 0;

	DECL_PCI_ISR_HANDLERS_FRIEND
};

REGISTER_PCI_ISR_METHOD(1, ToFController, &ToFController::gpio_raised, GPIO1, GPIO2)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	time::delay_ms(10000);

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

	TOF tof1{manager};
	TOF tof2{manager};
	manager.begin();

	// Shutdown all devices
	gpio::FAST_PIN<SHDN1> shutdown1{gpio::PinMode::OUTPUT};
	gpio::FAST_PIN<SHDN2> shutdown2{gpio::PinMode::OUTPUT};
	time::delay_ms(10);

	// Activate TOF1 and set address
	shutdown1.set();
	time::delay_ms(2);
	CHECK_OK(tof1.set_address(0x30));

	// Activate TOF2 and set address
	shutdown2.set();
	time::delay_ms(2);
	CHECK_OK(tof2.set_address(0x2A));

	ToFController controller;

	// Initialize both devices
	CHECK_OK(tof1.begin(Profile::STANDARD));
	CHECK_OK(tof2.begin(Profile::STANDARD));

	CHECK_OK(tof1.set_GPIO_settings(GPIOSettings::low_threshold(200)));
	CHECK_OK(tof2.set_GPIO_settings(GPIOSettings::low_threshold(200)));

	CHECK_OK(tof1.start_continuous_ranging(100));
	CHECK_OK(tof2.start_continuous_ranging(100));

	out << F("await GPIO") << endl;
	while (true)
	{
		uint16_t range1 = 0;
		uint16_t range2 = 0;
		rtt.millis(0);
		uint8_t trigger = controller.await_gpio();
		time::RTTTime time1 = rtt.time();

		rtt.millis(0);
		if (trigger & 0x01)
			CHECK_OK(tof1.get_direct_range(range1));
		if (trigger & 0x02)
			CHECK_OK(tof2.get_direct_range(range2));
		time::RTTTime time2 = rtt.time();
		// Clear interrupt
		if (trigger & 0x01)
			CHECK_OK(tof1.clear_interrupt());
		if (trigger & 0x02)
			CHECK_OK(tof2.clear_interrupt());

		out << F("GPIO after ") << dec << time1.millis() << "ms " << time1.micros() << "us" << streams::endl;
		out << F("Range after ") << time2.millis() << "ms " << time2.micros() << "us" << streams::endl;
		out << F("GPIO trigger = ") << hex << trigger << endl;
		out << F("range 1 = ") << dec << range1 << F("mm") << endl;
		out << F("range 2 = ") << range2 << F("mm") << endl;
	}
}
