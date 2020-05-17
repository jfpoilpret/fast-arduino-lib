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
	uint8_t tm_sec;  /**< seconds after the minute - [ 0 to 59 ] */
	uint8_t tm_min;  /**< minutes after the hour - [ 0 to 59 ] */
	uint8_t tm_hour; /**< hours since midnight - [ 0 to 23 ] */
	WeekDay tm_wday; /**< days since Sunday - [ 1 to 7 ] */
	uint8_t tm_mday; /**< day of the month - [ 1 to 31 ] */
	uint8_t tm_mon;  /**< months since January - [ 1 to 12 ] */
	uint8_t tm_year; /**< years since 2000 */
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
		// containers::array<uint8_t, SIZE + 1> input;
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

	private:
	static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;
	static constexpr const uint8_t RAM_START = 0x08;
	static constexpr const uint8_t RAM_END = 0x40;
	static constexpr const uint8_t RAM_SIZE = RAM_END - RAM_START;
	static constexpr const uint8_t TIME_ADDRESS = 0x00;
};
