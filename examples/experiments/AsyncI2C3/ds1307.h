/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * This one is a proof of concept on I2C asynchronous handling, to be later integrated
 * to FastArduino library.
 * For tests, I just use a DS1307 connected through I2C (SDA/SCL) to an Arduino UNO.
 */

#include <fastarduino/i2c.h>
#include <fastarduino/future.h>
#include <fastarduino/utilities.h>

#include "array.h"
#include "i2c_handler.h"
#include "i2c_device.h"

enum class WeekDay : uint8_t
{
	SUNDAY = 1,
	MONDAY,
	TUESDAY,
	WEDNESDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY
};

struct tm
{
	uint8_t tm_sec = 0;           /**< seconds after the minute - [ 0 to 59 ] */
	uint8_t tm_min = 0;           /**< minutes after the hour - [ 0 to 59 ] */
	uint8_t tm_hour = 0;          /**< hours since midnight - [ 0 to 23 ] */
	WeekDay tm_wday = WeekDay(0); /**< days since Sunday - [ 1 to 7 ] */
	uint8_t tm_mday = 0;          /**< day of the month - [ 1 to 31 ] */
	uint8_t tm_mon = 0;           /**< months since January - [ 1 to 12 ] */
	uint8_t tm_year = 0;          /**< years since 2000 */
};

enum class SquareWaveFrequency : uint8_t
{
	FREQ_1HZ = 0x00,
	FREQ_4096HZ = 0x01,
	FREQ_8192HZ = 0x02,
	FREQ_32768HZ = 0x03
};

class RTC : public i2c::AbstractDevice<i2c::I2CMode::STANDARD>
{
	struct set_tm
	{
		uint8_t address_;
		tm tm_;
	};

	public:
	RTC(i2c::I2CHandler<i2c::I2CMode::STANDARD>& handler)
		: i2c::AbstractDevice<i2c::I2CMode::STANDARD>{handler, DEVICE_ADDRESS} {}

	static constexpr uint8_t ram_size()
	{
		return RAM_SIZE;
	}

	using SET_DATETIME = future::Future<void, set_tm>;
	int set_datetime(tm& datetime, SET_DATETIME& future)
	{
		set_tm set;
		// 1st convert datetime for DS1307 (BCD)
		set.address_ = TIME_ADDRESS;
		set.tm_.tm_sec = utils::binary_to_bcd(datetime.tm_sec);
		set.tm_.tm_min = utils::binary_to_bcd(datetime.tm_min);
		set.tm_.tm_hour = utils::binary_to_bcd(datetime.tm_hour);
		set.tm_.tm_mday = utils::binary_to_bcd(datetime.tm_mday);
		set.tm_.tm_mon = utils::binary_to_bcd(datetime.tm_mon);
		set.tm_.tm_year = utils::binary_to_bcd(datetime.tm_year);
		future.reset_input(set);
		// send register address to write to (0)
		// send datetime at address 0
		return launch_commands(future, {write(i2c::I2CFinish::FORCE_STOP | i2c::I2CFinish::FUTURE_FINISH)});
	}

	class GetDatetimeFuture : public future::Future<tm, uint8_t>
	{
	public:
		GetDatetimeFuture() : future::Future<tm, uint8_t>{TIME_ADDRESS} {}
		GetDatetimeFuture(GetDatetimeFuture&&) = default;
		GetDatetimeFuture& operator=(GetDatetimeFuture&&) = default;

		bool get(tm& datetime)
		{
			if (!future::Future<tm, uint8_t>::get(datetime)) return false;
			// convert DS1307 output (BCD) to integer type
			datetime.tm_sec = utils::bcd_to_binary(datetime.tm_sec);
			datetime.tm_min = utils::bcd_to_binary(datetime.tm_min);
			datetime.tm_hour = utils::bcd_to_binary(datetime.tm_hour);
			datetime.tm_mday = utils::bcd_to_binary(datetime.tm_mday);
			datetime.tm_mon = utils::bcd_to_binary(datetime.tm_mon);
			datetime.tm_year = utils::bcd_to_binary(datetime.tm_year);
			return true;
		}
	};

	int get_datetime(GetDatetimeFuture& future)
	{
		return launch_commands(future, {write(), read(i2c::I2CFinish::FORCE_STOP)});
	}

	template<uint8_t SIZE, typename T = uint8_t>
	using SET_RAM = future::Future<void, containers::array<T, SIZE + 1>>;
	int set_ram(uint8_t address, uint8_t data, SET_RAM<1>& future)
	{
		address += RAM_START;
		if (address >= RAM_END)
			return errors::EINVAL;
		future.reset_input({address, data});
		return launch_commands(future, {write(i2c::I2CFinish::FORCE_STOP | i2c::I2CFinish::FUTURE_FINISH)});
	}

	template<uint8_t SIZE>
	int set_ram(uint8_t address, const uint8_t (&data)[SIZE], SET_RAM<SIZE>& future)
	{
		address += RAM_START;
		if ((address + SIZE) > RAM_END)
			return errors::EINVAL;
		// prepare future and I2C transaction
		typename SET_RAM<SIZE>::IN input;
		input[0] = address;
		input.set(uint8_t(1), data);
		future.reset_input(input);
		return launch_commands(future, {write(i2c::I2CFinish::FORCE_STOP | i2c::I2CFinish::FUTURE_FINISH)});
	}

	using GET_RAM1 = future::Future<uint8_t, uint8_t>;
	int get_ram(uint8_t address, GET_RAM1& future)
	{
		address += RAM_START;
		if (address >= RAM_END)
			return errors::EINVAL;
		future.reset_input(address);
		return launch_commands(future, {write(), read(i2c::I2CFinish::FORCE_STOP)});
	}

	template<uint8_t SIZE, typename T = uint8_t>
	using GET_RAM = future::Future<containers::array<T, SIZE>, uint8_t>;
	template<uint8_t SIZE>
	int get_ram(uint8_t address, GET_RAM<SIZE>& future)
	{
		address += RAM_START;
		if (address >= RAM_END)
			return errors::EINVAL;
		// prepare future and I2C transaction
		future.reset_input(address);
		return launch_commands(future, {write(), read(i2c::I2CFinish::FORCE_STOP)});
	}

	using HALT_CLOCK = future::Future<void, containers::array<uint8_t, 2>>;
	int halt_clock(HALT_CLOCK& future)
	{
		// just write 0x80 at address 0
		future.reset_input({TIME_ADDRESS, CLOCK_HALT});
		return launch_commands(future, {write(i2c::I2CFinish::FORCE_STOP | i2c::I2CFinish::FUTURE_FINISH)});
	}

	using ENABLE_OUTPUT = future::Future<void, containers::array<uint8_t, 2>>;
	bool enable_output(ENABLE_OUTPUT& future, SquareWaveFrequency frequency = SquareWaveFrequency::FREQ_1HZ)
	{
		ControlRegister control;
		control.sqwe = 1;
		control.rs = uint8_t(frequency);
		typename ENABLE_OUTPUT::IN input;
		input[0] = CONTROL_ADDRESS;
		input[1] = control.data;
		future.reset_input(input);
		return launch_commands(future, {write(i2c::I2CFinish::FORCE_STOP | i2c::I2CFinish::FUTURE_FINISH)});
	}

	using DISABLE_OUTPUT = future::Future<void, containers::array<uint8_t, 2>>;
	bool disable_output(DISABLE_OUTPUT& future, bool output_value = false)
	{
		ControlRegister control;
		control.out = output_value;
		typename ENABLE_OUTPUT::IN input;
		input[0] = CONTROL_ADDRESS;
		input[1] = control.data;
		future.reset_input(input);
		return launch_commands(future, {write(i2c::I2CFinish::FORCE_STOP | i2c::I2CFinish::FUTURE_FINISH)});
	}

	private:
	static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;
	static constexpr const uint8_t RAM_START = 0x08;
	static constexpr const uint8_t RAM_END = 0x40;
	static constexpr const uint8_t RAM_SIZE = RAM_END - RAM_START;
	static constexpr const uint8_t TIME_ADDRESS = 0x00;
	static constexpr const uint8_t CLOCK_HALT = 0x80;
	static constexpr const uint8_t CONTROL_ADDRESS = 0x07;

	union ControlRegister
	{
		explicit ControlRegister(uint8_t data = 0) : data{data} {}

		uint8_t data;
		struct
		{
			uint8_t rs : 2;
			uint8_t res1 : 2;
			uint8_t sqwe : 1;
			uint8_t res2 : 2;
			uint8_t out : 1;
		};
	};
};
