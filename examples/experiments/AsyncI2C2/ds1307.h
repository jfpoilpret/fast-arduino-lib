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

class RTC
{
	struct set_tm
	{
		uint8_t address_;
		tm tm_;
	};

	public:
	RTC(I2CHandler<i2c::I2CMode::STANDARD>& handler) : handler_{handler} {}

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
		// send register address to write to (0)
		// send datetime at address 0
		auto& manager = future::AbstractFutureManager::instance();
		synchronized
		{
			// pre-conditions (must be synchronized)
			if (manager.available_futures_() == 0) return errors::EAGAIN;
			if (!handler_.ensure_num_commands_(1)) return errors::EAGAIN;
			// prepare future and I2C transaction
			SET_DATETIME temp{set};
			// NOTE: normally 3 following calls should never return false!
			if (!manager.register_future_(temp)) return errors::EAGAIN;
			if (!handler_.write_(DEVICE_ADDRESS, temp.id(), true, true)) return errors::EAGAIN;
			future = std::move(temp);
			return 0;
		}
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
		auto& manager = future::AbstractFutureManager::instance();
		synchronized
		{
			// pre-conditions (must be synchronized)
			if (manager.available_futures_() == 0) return errors::EAGAIN;
			if (!handler_.ensure_num_commands_(2)) return errors::EAGAIN;
			// prepare future and I2C transaction
			// NOTE: normally 3 following calls should never return false!
			if (!manager.register_future_(future)) return errors::EAGAIN;
			if (!handler_.write_(DEVICE_ADDRESS, future.id(), false, false)) return errors::EAGAIN;
			if (!handler_.read_(DEVICE_ADDRESS, future.id(), true, false)) return errors::EAGAIN;
			return 0;
		}
	}

	template<uint8_t SIZE, typename T = uint8_t>
	using SET_RAM = future::Future<void, containers::array<T, SIZE + 1>>;
	int set_ram(uint8_t address, uint8_t data, SET_RAM<1>& future)
	{
		address += RAM_START;
		if (address >= RAM_END)
			return errors::EINVAL;
		auto& manager = future::AbstractFutureManager::instance();
		synchronized
		{
			// pre-conditions (must be synchronized)
			if (manager.available_futures_() == 0) return errors::EAGAIN;
			if (!handler_.ensure_num_commands_(1)) return errors::EAGAIN;
			// prepare future and I2C transaction
			SET_RAM<1> temp{{address, data}};
			// NOTE: normally 3 following calls should never return false!
			if (!manager.register_future_(temp)) return errors::EAGAIN;
			if (!handler_.write_(DEVICE_ADDRESS, temp.id(), true, true)) return errors::EAGAIN;
			future = std::move(temp);
			return 0;
		}
	}

	template<uint8_t SIZE>
	int set_ram(uint8_t address, const uint8_t (&data)[SIZE], SET_RAM<SIZE>& future)
	{
		address += RAM_START;
		if ((address + SIZE) > RAM_END)
			return errors::EINVAL;
		auto& manager = future::AbstractFutureManager::instance();
		synchronized
		{
			// pre-conditions (must be synchronized)
			if (manager.available_futures_() == 0) return errors::EAGAIN;
			if (!handler_.ensure_num_commands_(1)) return errors::EAGAIN;
			// prepare future and I2C transaction
			typename SET_RAM<SIZE>::IN input;
			// containers::array<uint8_t, SIZE + 1> input;
			input[0] = address;
			input.set(uint8_t(1), data);
			SET_RAM<SIZE> temp{input};
			// NOTE: normally 3 following calls should never return false!
			if (!manager.register_future_(temp)) return errors::EAGAIN;
			if (!handler_.write_(DEVICE_ADDRESS, temp.id(), true, true)) return errors::EAGAIN;
			future = std::move(temp);
			return 0;
		}
	}

	using GET_RAM1 = future::Future<uint8_t, uint8_t>;
	int get_ram(uint8_t address, GET_RAM1& future)
	{
		address += RAM_START;
		if (address >= RAM_END)
			return errors::EINVAL;
		auto& manager = future::AbstractFutureManager::instance();
		//NOTE maybe an abstract device class could encapsulate all that block with a list of commands in args?
		synchronized
		{
			// pre-conditions (must be synchronized)
			if (manager.available_futures_() == 0) return errors::EAGAIN;
			if (!handler_.ensure_num_commands_(2)) return errors::EAGAIN;
			// prepare future and I2C transaction
			GET_RAM1 temp{address};
			// NOTE: normally 3 following calls should never return false!
			if (!manager.register_future_(temp)) return errors::EAGAIN;
			if (!handler_.write_(DEVICE_ADDRESS, temp.id(), false, false)) return errors::EAGAIN;
			if (!handler_.read_(DEVICE_ADDRESS, temp.id(), true, false)) return errors::EAGAIN;
			future = std::move(temp);
			return 0;
		}
	}

	template<uint8_t SIZE, typename T = uint8_t>
	using GET_RAM = future::Future<containers::array<T, SIZE>, uint8_t>;
	template<uint8_t SIZE>
	int get_ram(uint8_t address, GET_RAM<SIZE>& future)
	{
		address += RAM_START;
		if (address >= RAM_END)
			return errors::EINVAL;
		auto& manager = future::AbstractFutureManager::instance();
		synchronized
		{
			// pre-conditions (must be synchronized)
			if (manager.available_futures_() == 0) return errors::EAGAIN;
			if (!handler_.ensure_num_commands_(2)) return errors::EAGAIN;
			// prepare future and I2C transaction
			GET_RAM<SIZE> temp{{address}};
			// NOTE: normally 3 following calls should never return false!
			if (!manager.register_future_(temp)) return errors::EAGAIN;
			if (!handler_.write_(DEVICE_ADDRESS, temp.id(), false, false)) return errors::EAGAIN;
			if (!handler_.read_(DEVICE_ADDRESS, temp.id(), true, false)) return errors::EAGAIN;
			future = std::move(temp);
			return 0;
		}
	}

	private:
	static constexpr const uint8_t DEVICE_ADDRESS = 0x68 << 1;
	static constexpr const uint8_t RAM_START = 0x08;
	static constexpr const uint8_t RAM_END = 0x40;
	static constexpr const uint8_t RAM_SIZE = RAM_END - RAM_START;
	static constexpr const uint8_t TIME_ADDRESS = 0x00;

	I2CHandler<i2c::I2CMode::STANDARD>& handler_;
};
