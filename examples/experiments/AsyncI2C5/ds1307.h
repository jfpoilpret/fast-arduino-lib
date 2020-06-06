/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * This one is a proof of concept on I2C asynchronous handling, to be later integrated
 * to FastArduino library.
 * For tests, I just use a DS1307 connected through I2C (SDA/SCL) to an Arduino UNO.
 */

#include <fastarduino/array.h>
#include <fastarduino/i2c.h>
#include <fastarduino/future.h>
#include <fastarduino/utilities.h>

#include "i2c_handler.h"
#include "i2c_device.h"

// Device driver guidelines:
// - Template on MODE only if both modes accepted, otherwise force MODE
// - Define Future subclass (inside device class) for every future requiring input (constant, or user-provided)
//   naming convention: MethodNameFuture
// - Future subclass shall have explicit constructor with mandatory input arguments (no default)
// - define using types (inside device class) for each future subclass (UPPER_CASE same as METHOD_NAME)
// - each API method returns int (error code) and takes reference to specific future as unique argument

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

	class SetDatetimeFuture : public future::Future<void, set_tm>
	{
	public:
		explicit SetDatetimeFuture(const tm& datetime)
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
			this->reset_input(set);
		}
		SetDatetimeFuture(SetDatetimeFuture&&) = default;
		SetDatetimeFuture& operator=(SetDatetimeFuture&&) = default;
	};
	using SET_DATETIME = SetDatetimeFuture;
	int set_datetime(SET_DATETIME& future)
	{
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
	using GET_DATETIME = GetDatetimeFuture;
	int get_datetime(GET_DATETIME& future)
	{
		return launch_commands(future, {write(), read(i2c::I2CFinish::FORCE_STOP)});
	}

	template<uint8_t SIZE_>
	class SetRamFuture : public future::Future<void, containers::array<uint8_t, SIZE_ + 1>>
	{
		//TODO add static assert on SIZE_
		using PARENT = future::Future<void, containers::array<uint8_t, SIZE_ + 1>>;
	public:
		SetRamFuture() = default;
		explicit SetRamFuture(uint8_t address, const uint8_t (&data)[SIZE_])
		{
			typename PARENT::IN input;
			input[0] = static_cast<uint8_t>(address + RAM_START);
			input.set(uint8_t(1), data);
			this->reset_input(input);
		}
		SetRamFuture(SetRamFuture<SIZE_>&&) = default;
		SetRamFuture& operator=(SetRamFuture<SIZE_>&&) = default;

		bool is_input_valid() const
		{
			return (this->get_input()[0] + SIZE_ <= RAM_END);
		}
	};
	template<uint8_t SIZE> using SET_RAM = SetRamFuture<SIZE>;
	template<uint8_t SIZE> int set_ram(SET_RAM<SIZE>& future)
	{
		if (!future.is_input_valid())
			return errors::EINVAL;
		return launch_commands(future, {write(i2c::I2CFinish::FORCE_STOP | i2c::I2CFinish::FUTURE_FINISH)});
	}

	class SetRam1Future : public future::Future<void, containers::array<uint8_t, 2>>
	{
		using PARENT = future::Future<void, containers::array<uint8_t, 2>>;
	public:
		SetRam1Future() = default;
		explicit SetRam1Future(uint8_t address, uint8_t data)
			:	PARENT{{static_cast<uint8_t>(address + RAM_START), data}} {}
		SetRam1Future(SetRam1Future&&) = default;
		SetRam1Future& operator=(SetRam1Future&&) = default;

		bool is_input_valid() const
		{
			return (this->get_input()[0] < RAM_END);
		}
	};
	using SET_RAM1 = SetRam1Future;
	int set_ram(SET_RAM1& future)
	{
		if (!future.is_input_valid())
			return errors::EINVAL;
		return launch_commands(future, {write(i2c::I2CFinish::FORCE_STOP | i2c::I2CFinish::FUTURE_FINISH)});
	}

	template<uint8_t SIZE_>
	class GetRamFuture : public future::Future<containers::array<uint8_t, SIZE_>, uint8_t>
	{
		//TODO add static assert on SIZE_
		using PARENT = future::Future<containers::array<uint8_t, SIZE_>, uint8_t>;
	public:
		explicit GetRamFuture(uint8_t address) : PARENT{static_cast<uint8_t>(address + RAM_START)} {}
		GetRamFuture(GetRamFuture<SIZE_>&&) = default;
		GetRamFuture& operator=(GetRamFuture<SIZE_>&&) = default;

		bool is_input_valid() const
		{
			return (this->get_input() + SIZE_ <= RAM_END);
		}
	};

	template<uint8_t SIZE> using GET_RAM = GetRamFuture<SIZE>;
	template<uint8_t SIZE> int get_ram(GET_RAM<SIZE>& future)
	{
		if (!future.is_input_valid())
			return errors::EINVAL;
		return launch_commands(future, {write(), read(i2c::I2CFinish::FORCE_STOP)});
	}

	class GetRam1Future : public future::Future<uint8_t, uint8_t>
	{
		using PARENT = future::Future<uint8_t, uint8_t>;
	public:
		explicit GetRam1Future(uint8_t address = 0) : PARENT{static_cast<uint8_t>(address + RAM_START)} {}
		GetRam1Future(GetRam1Future&&) = default;
		GetRam1Future& operator=(GetRam1Future&&) = default;

		bool is_input_valid() const
		{
			return this->get_input() < RAM_END;
		}
	};
	using GET_RAM1 = GetRam1Future;
	int get_ram(GET_RAM1& future)
	{
		if (!future.is_input_valid())
			return errors::EINVAL;
		return launch_commands(future, {write(), read(i2c::I2CFinish::FORCE_STOP)});
	}

	class HaltClockFuture : public future::Future<void, containers::array<uint8_t, 2>>
	{
	public:
		// just write 0x80 at address 0
		HaltClockFuture() : future::Future<void, containers::array<uint8_t, 2>>{{TIME_ADDRESS, CLOCK_HALT}} {}
		HaltClockFuture(HaltClockFuture&&) = default;
		HaltClockFuture& operator=(HaltClockFuture&&) = default;
	};
	using HALT_CLOCK = HaltClockFuture;
	int halt_clock(HALT_CLOCK& future)
	{
		return launch_commands(future, {write(i2c::I2CFinish::FORCE_STOP | i2c::I2CFinish::FUTURE_FINISH)});
	}

	class EnableOutputFuture : public future::Future<void, containers::array<uint8_t, 2>>
	{
		using PARENT = future::Future<void, containers::array<uint8_t, 2>>;
	public:
		explicit EnableOutputFuture(SquareWaveFrequency frequency)
		{
			ControlRegister control;
			control.sqwe = 1;
			control.rs = uint8_t(frequency);
			typename PARENT::IN input;
			input[0] = CONTROL_ADDRESS;
			input[1] = control.data;
			this->reset_input(input);
		}
		EnableOutputFuture(EnableOutputFuture&&) = default;
		EnableOutputFuture& operator=(EnableOutputFuture&&) = default;
	};
	using ENABLE_OUTPUT = EnableOutputFuture;
	int enable_output(ENABLE_OUTPUT& future)
	{
		return launch_commands(future, {write(i2c::I2CFinish::FORCE_STOP | i2c::I2CFinish::FUTURE_FINISH)});
	}

	class DisableOutputFuture : public future::Future<void, containers::array<uint8_t, 2>>
	{
		using PARENT = future::Future<void, containers::array<uint8_t, 2>>;
	public:
		explicit DisableOutputFuture(bool output_value)
		{
			ControlRegister control;
			control.out = output_value;
			typename PARENT::IN input;
			input[0] = CONTROL_ADDRESS;
			input[1] = control.data;
			this->reset_input(input);
		}
		DisableOutputFuture(DisableOutputFuture&&) = default;
		DisableOutputFuture& operator=(DisableOutputFuture&&) = default;
	};
	using DISABLE_OUTPUT = DisableOutputFuture;
	int disable_output(DISABLE_OUTPUT& future)
	{
		return launch_commands(future, {write(i2c::I2CFinish::FORCE_STOP | i2c::I2CFinish::FUTURE_FINISH)});
	}

	// synchronous API
	bool set_datetime(const tm& datetime)
	{
		SET_DATETIME future{datetime};
		if (set_datetime(future) != 0) return false;
		return (future.await() == future::FutureStatus::READY);
	}
	bool get_datetime(tm& datetime)
	{
		GET_DATETIME future;
		if (get_datetime(future) != 0) return false;
		return future.get(datetime);
	}
	bool halt_clock()
	{
		HALT_CLOCK future;
		if (halt_clock(future) != 0) return false;
		return (future.await() == future::FutureStatus::READY);
	}
	bool enable_output(SquareWaveFrequency frequency = SquareWaveFrequency::FREQ_1HZ)
	{
		ENABLE_OUTPUT future{frequency};
		if (enable_output(future) != 0) return false;
		return (future.await() == future::FutureStatus::READY);
	}
	bool disable_output(bool output_value = false)
	{
		DISABLE_OUTPUT future{output_value};
		if (disable_output(future) != 0) return false;
		return (future.await() == future::FutureStatus::READY);
	}
	bool set_ram(uint8_t address, uint8_t data)
	{
		SET_RAM1 future{address, data};
		if (set_ram(future) != 0) return false;
		return (future.await() == future::FutureStatus::READY);
	}
	uint8_t get_ram(uint8_t address)
	{
		GET_RAM1 future{address};
		if (get_ram(future) != 0) return false;
		uint8_t data = 0;
		future.get(data);
		return data;
	}
	template<uint8_t SIZE> bool set_ram(uint8_t address, const uint8_t (&data)[SIZE])
	{
		SET_RAM<SIZE> future{address, data};
		if (set_ram(future) != 0) return false;
		return (future.await() == future::FutureStatus::READY);
	}
	template<uint8_t SIZE> bool get_ram(uint8_t address, uint8_t (&data)[SIZE])
	{
		GET_RAM<SIZE> future{address};
		if (get_ram(future) != 0) return false;
		typename GET_RAM<SIZE>::OUT temp;
		if (!future.get(temp)) return false;
		memcpy(data, temp.data(), SIZE);
		return true;
	}

	template<typename T> bool set_ram(uint8_t address, const T& data)
	{
		uint8_t temp[sizeof(T)];
		utils::as_array<T>(data, temp);
		SET_RAM<sizeof(T)> future{address, temp};
		if (set_ram(future) != 0) return false;
		return (future.await() == future::FutureStatus::READY);
	}
	template<typename T> bool get_ram(uint8_t address, T& data)
	{
		GET_RAM<sizeof(T)> future{address};
		if (get_ram(future) != 0) return false;
		return future.get(reinterpret_cast<uint8_t&>(data));
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
