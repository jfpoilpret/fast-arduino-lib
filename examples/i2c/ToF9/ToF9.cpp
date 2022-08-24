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
 * ISR based ranger example, using VL53L0X Time-of-flight range sensor I2C device.
 * This program uses FastArduino VL53L0X support API.
 * It uses continuous reading and Futures to read asynchronously from I2C bus.
 * It also uses GPIO and interrupts on low threshold.
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

#if not I2C_TRUE_ASYNC
#error "Target must support asynchronous I2C"
#endif

static constexpr const i2c::I2CMode MODE = i2c::I2CMode::FAST;
static constexpr const board::ExternalInterruptPin GPIO = board::ExternalInterruptPin::D2_PD2_EXT0;

static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

REGISTER_RTT_ISR(0)
REGISTER_UATX_ISR(0)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)

using MANAGER = i2c::I2CAsyncManager<MODE, i2c::I2CErrorPolicy::CLEAR_ALL_COMMANDS>;
static constexpr const uint8_t I2C_BUFFER_SIZE = 32;
static MANAGER::I2CCOMMAND i2c_buffer[I2C_BUFFER_SIZE];

using streams::dec;
using streams::hex;
using streams::fixed;
using streams::endl;
using streams::flush;

using namespace devices::vl53l0x;
using TOF = VL53L0X<MANAGER>;
using RTT = timer::RTT<board::Timer::TIMER0>;

#define CHECK_OK(STATEMENT) if (! STATEMENT) out << F(#STATEMENT) << F(" ERROR!") << endl

//TODO improve efficiency by reducing number of I2C commands between 2 reads (33ms!)
//TODO use RTT events to trigger beginning of reading sequence (status loop, read range, clear interrupts)
class ToFController
{
public:
	ToFController(RTT& rtt, TOF& tof) : rtt_{rtt}, tof_{tof}
	{
		interrupt::register_handler(*this);
		signal_.enable();
	}

	void refresh()
	{
		if (phase_ == Phase::INACTIVE && next_range_time() <= rtt_.millis())
		{
			phase_ = Phase::READ_STATUS;
			//TODO what if error in this call?
			status_future_.reset_();
			tof_.get_range_status(status_future_);
		}
	}

	uint16_t count_reads() const
	{
		synchronized return count_;
	}

	uint16_t count_gpios() const
	{
		synchronized return gpio_count_;
	}

	uint16_t get_range() const
	{
		synchronized return range_;
	}

private:
	enum class Phase : uint8_t
	{
		INACTIVE = 0,
		READ_STATUS,
		READ_RANGE,
		CLEAR_INTERRUPT
	};

	uint32_t next_range_time() const
	{
		synchronized return next_range_time_;
	}

	void i2c_change(i2c::I2CCallback callback, UNUSED MANAGER::FUTURE_PROXY proxy)
	{
		if (callback != i2c::I2CCallback::END_TRANSACTION) return;
		switch (phase_)
		{
			case Phase::READ_STATUS:
			if (status_future_.status() == future::FutureStatus::READY)
			{
				DeviceStatus status;
				// if (status_future_.get(status) && status.data_ready())
				if (status_future_.get(status) && status.error() == DeviceError::RANGE_COMPLETE)
				{
					phase_ = Phase::READ_RANGE;
					next_range_time_ = rtt_.millis_() + READING_PERIOD_MS;
					range_future_.reset_();
					tof_.get_direct_range(range_future_);
				}
				else
				{
					status_future_.reset_();
					tof_.get_range_status(status_future_);
				}
			}
			break;

			case Phase::READ_RANGE:
			if (range_future_.status() == future::FutureStatus::READY)
			{
				uint16_t range = 0;
				if (range_future_.get(range))
				{
					++count_;
					range_ = range;
				}
				phase_ = Phase::CLEAR_INTERRUPT;
				clear_int_future_.reset_(CLEAR_INTERRUPT_MASK);
				tof_.clear_interrupt(clear_int_future_);
			}
			break;

			case Phase::CLEAR_INTERRUPT:
			if (clear_int_future_.status() == future::FutureStatus::READY)
			{
				// Start whole cycle again
				phase_ = Phase::INACTIVE;
			}
			break;

			default:
			break;
		}
	}

	// Normally this should be called during a state read loop, so we do not want
	// to trigger an extra read maybe (but we could possibly advance it if not yet done) 
	void gpio_raised()
	{
		++gpio_count_;
	}

	gpio::FAST_EXT_PIN<GPIO> gpio_pin_{gpio::PinMode::INPUT};
	interrupt::INTSignal<GPIO> signal_{interrupt::InterruptTrigger::FALLING_EDGE};
	static constexpr uint16_t READING_PERIOD_MS = 33;
	RTT& rtt_;
	TOF& tof_;
	TOF::GetDirectRangeFuture range_future_;
	TOF::GetRangeStatusFuture status_future_;
	const uint8_t CLEAR_INTERRUPT_MASK = 0x01;
	TOF::ClearInterruptFuture clear_int_future_;
	
	volatile Phase phase_ = Phase::INACTIVE;
	volatile uint32_t next_range_time_ = 0;
	volatile uint16_t range_ = 0;
	volatile uint16_t count_ = 0;

	volatile uint16_t gpio_count_ = 0;

	DECL_INT_ISR_HANDLERS_FRIEND
	DECL_I2C_ISR_HANDLERS_FRIEND
};

REGISTER_I2C_ISR_METHOD(MANAGER, ToFController, &ToFController::i2c_change)
REGISTER_INT_ISR_METHOD(0, GPIO, ToFController, &ToFController::gpio_raised)
REGISTER_FUTURE_STATUS_LISTENERS(i2c::I2CSameFutureGroup<MANAGER>, TOF::SetGPIOSettingsFuture)

static constexpr uint16_t NUM_LOOPS = 1000U;

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	RTT rtt;
	rtt.begin();

	// open UART for traces
	serial::hard::UATX<UART> uart{output_buffer};
	streams::ostream out = uart.out();
	uart.begin(115200);
	out << streams::boolalpha;

	// Initialize I2C async handler
	MANAGER manager{i2c_buffer};
	TOF tof{manager};
	manager.begin();
	ToFController controller{rtt, tof};

	CHECK_OK(tof.begin(Profile::STANDARD));
	CHECK_OK(tof.set_GPIO_settings(GPIOSettings::low_threshold(100)));
	CHECK_OK(tof.start_continuous_ranging());
	// controller.refresh();

	out << F("Await ranges") << endl;
	// Sensor is capable of 1 read every 33ms with current profile, that is 30 reads/mn
	// Limit loop to 1' i.e. 30 reads
	uint16_t last_count = 0;
	uint16_t last_gpio_count = 0;
	for (uint16_t i = 0; i < NUM_LOOPS; )
	{
		controller.refresh();
		if (controller.count_gpios() != last_gpio_count)
		{
			last_gpio_count = controller.count_gpios();
			out << last_gpio_count << F(" GPIO") << endl;
		}
		if (controller.count_reads() != last_count)
		{
			last_count = controller.count_reads();
			uint16_t range = controller.get_range();
			out << range << F("mm") << endl;
			++i;
		}
		time::delay_ms(1);
	}

	tof.stop_continuous_ranging();
	manager.end();

	out << F("Finished!") << endl;
}
