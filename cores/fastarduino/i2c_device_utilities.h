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

/// @cond api

/**
 * @file 
 * Various utilities to use for I2C Device support developers.
 */
#ifndef I2C_DEVICE_UTILITIES_H
#define I2C_DEVICE_UTILITIES_H

// #include "array.h"
#include "flash.h"
#include "future.h"
#include "iterator.h"
#include "time.h"
#include "utilities.h"
#include "i2c_device.h"

namespace i2c
{
	/// @cond notdocumented
	// Internal type used by WriteRegisterFuture
	template<typename T, bool BIG_ENDIAN = true> class WriteContent
	{
	public:
		WriteContent() : register_{0}, value_{0} {}
		WriteContent(uint8_t reg, const T& value)
			:	register_{reg}, value_{BIG_ENDIAN ? utils::change_endianness(value) : value} {}

	private:
		const uint8_t register_;
		const T value_; 
	};
	/// @endcond

	// Future to read a register
	//TODO Add transformer functor to template?
	//TODO DOCS
	template<typename MANAGER, typename T, bool BIG_ENDIAN = true>
	class ReadRegisterFuture: public MANAGER::template FUTURE<T, uint8_t>
	{
		using PARENT = typename MANAGER::template FUTURE<T, uint8_t>;

	protected:
		using ABSTRACT_FUTURE = typename MANAGER::ABSTRACT_FUTURE;
		using FUTURE_STATUS_LISTENER = future::FutureStatusListener<ABSTRACT_FUTURE>;
		using FUTURE_OUTPUT_LISTENER = future::FutureOutputListener<ABSTRACT_FUTURE>;

	public:
		explicit ReadRegisterFuture(uint8_t reg,
			FUTURE_STATUS_LISTENER* status_listener = nullptr,
			FUTURE_OUTPUT_LISTENER* output_listener = nullptr)
			:	PARENT{reg, status_listener, output_listener} {}
		ReadRegisterFuture(ReadRegisterFuture&&) = default;
		ReadRegisterFuture& operator=(ReadRegisterFuture&&) = default;

		bool get(T& result)
		{
			if (!PARENT::get(result)) return false;
			if (BIG_ENDIAN)
				result = utils::change_endianness(result);
			return true;
		}
	};

	//TODO DOCS
	template<typename MANAGER, uint8_t REGISTER, typename T, bool BIG_ENDIAN = true>
	class TReadRegisterFuture: public ReadRegisterFuture<MANAGER, T, BIG_ENDIAN>
	{
		using PARENT = ReadRegisterFuture<MANAGER, T, BIG_ENDIAN>;
	public:
		explicit TReadRegisterFuture(
			typename PARENT::FUTURE_STATUS_LISTENER* status_listener = nullptr,
			typename PARENT::FUTURE_OUTPUT_LISTENER* output_listener = nullptr)
			:	PARENT{REGISTER, status_listener, output_listener} {}
		TReadRegisterFuture(TReadRegisterFuture&&) = default;
		TReadRegisterFuture& operator=(TReadRegisterFuture&&) = default;
	};

	//TODO Add transformer functor to template?
	//TODO Add checker functor to template?
	//TODO DOCS
	template<typename MANAGER, typename T, bool BIG_ENDIAN = true>
	class WriteRegisterFuture: public MANAGER::template FUTURE<void, WriteContent<T, BIG_ENDIAN>>
	{
		using PARENT = typename MANAGER::template FUTURE<void, WriteContent<T, BIG_ENDIAN>>;
		using CONTENT = WriteContent<T, BIG_ENDIAN>;

	protected:
		using ABSTRACT_FUTURE = typename MANAGER::ABSTRACT_FUTURE;
		using FUTURE_STATUS_LISTENER = future::FutureStatusListener<ABSTRACT_FUTURE>;

	public:
		explicit WriteRegisterFuture(
			uint8_t reg, const T& value, 
			FUTURE_STATUS_LISTENER* status_listener = nullptr)
			:	PARENT{CONTENT{reg, value}, status_listener} {}
		WriteRegisterFuture(WriteRegisterFuture&&) = default;
		WriteRegisterFuture& operator=(WriteRegisterFuture&&) = default;
	};

	//TODO DOCS
	template<typename MANAGER, uint8_t REGISTER, typename T, bool BIG_ENDIAN = true>
	class TWriteRegisterFuture: public WriteRegisterFuture<MANAGER, T, BIG_ENDIAN>
	{
		using PARENT = WriteRegisterFuture<MANAGER, T, BIG_ENDIAN>;
	public:
		explicit TWriteRegisterFuture(const T& value,
			typename PARENT::FUTURE_STATUS_LISTENER* status_listener = nullptr)
			:	PARENT{REGISTER, value, status_listener} {}
		TWriteRegisterFuture(TWriteRegisterFuture&&) = default;
		TWriteRegisterFuture& operator=(TWriteRegisterFuture&&) = default;
	};

	//TODO put to own header file? What name?
	// For all I2C futures using flash read-only date to write to device, the following
	// convention is used
	// 1. The whole array is broken down into individual lines, each representing an action
	// 2. Each action is starting with an action byte
	// 3. Each action byte will have additional bytes depending on its actual action
	// 4. The last line uses action 0x00 (end of stream)
	//
	// Action bytes are defined as follows:
	// - 0x00	end of stream marker (Future is complete)
	// - 0x1H	write (1+H) bytes to the device; the action is followed by the register index and H bytes to write
	// - 0x9H	same as 0x1H, but this is the last command for this future (force stop is needed)
	// - 0x2H	read H bytes from device register, after writing the register index; 
	//			the action is followed by the register index
	// - 0xAH	same as 0x2H, but this is the last command for this future (force stop is needed)
	// - 0x30	special marker, used by the Future code to do something special at this point; this is followed by
	//			one byte that can be any free value indicating to code what shall be done
	// - 0x31	special marker, to include another Future into this Future; this is followed by one byte
	//			that can be any free value used to idnicate which Future shall be used
	// - other codes are kept for future (and Future) use

	// Action codes
	//--------------
	namespace actions
	{
		static constexpr uint8_t END = 0x00;
		static constexpr uint8_t WRITE = 0x10;
		static constexpr uint8_t READ = 0x20;
		static constexpr uint8_t MARKER = 0x30;
		static constexpr uint8_t LOOP = 0x31;
		static constexpr uint8_t INCLUDE = 0x40;

		static constexpr uint8_t STOP_MASK = 0x80;
		static constexpr uint8_t ACTION_MASK = 0x70;
		static constexpr uint8_t COUNT_MASK = 0x0F;

		static constexpr uint8_t write(uint8_t count, bool stop = false)
		{
			return WRITE | (count & COUNT_MASK) | (stop ? STOP_MASK : 0x00);
		}
		static constexpr uint8_t read(uint8_t count, bool stop = false)
		{
			return READ | (count & COUNT_MASK) | (stop ? STOP_MASK : 0x00);
		}
		static constexpr bool is_write(uint8_t action)
		{
			return (action & ACTION_MASK) == WRITE;
		}
		static constexpr bool is_read(uint8_t action)
		{
			return (action & ACTION_MASK) == READ;
		}
		static constexpr bool is_stop(uint8_t action)
		{
			return action & STOP_MASK;
		}
		static constexpr uint8_t count(uint8_t action)
		{
			return action & COUNT_MASK;
		}
	}

	/// @cond notcodumented
	// Forward declaration of I2CDevice
	template<typename MANAGER> class I2CDevice;

	template<typename MANAGER> class AbstractI2CFuture
	{
	public:
		future::FutureStatus status() const
		{
			return status_;
		}

		future::FutureStatus await() const
		{
			while (true)
			{
				future::FutureStatus status = this->status();
				if (status != future::FutureStatus::NOT_READY)
					return status;
				time::yield();
			}
		}

		int error() const
		{
			future::FutureStatus status = await();
			switch (status)
			{
				case future::FutureStatus::READY:
				return 0;

				case future::FutureStatus::ERROR:
				return error_;

				default:
				// This should never happen
				return errors::EINVAL;
			}
		}

	protected:
		using DEVICE = I2CDevice<MANAGER>;
		using ABSTRACT_FUTURE = typename DEVICE::ABSTRACT_FUTURE;
		template<typename T> using PROXY = typename DEVICE::template PROXY<T>;

		AbstractI2CFuture() = default;

		// Check launch_commands() return and update own status if needed
		bool check_launch(int launch)
		{
			if (launch == 0) return true;
			error_ = launch;
			status_ = future::FutureStatus::ERROR;
			return false;
		}

		void finish()
		{
			// Future is finished
			if (status_ == future::FutureStatus::NOT_READY)
				status_ = future::FutureStatus::READY;
		}

		bool check_status(const ABSTRACT_FUTURE& future, future::FutureStatus status)
		{
			// First check that current future was executed successfully
			if (status != future::FutureStatus::READY)
			{
				error_ = future.error();
				status_ = status;
				return false;
			}
			return true;
		}

		static constexpr I2CLightCommand read(uint8_t read_count = 0, bool finish_future = false, bool stop = false)
		{
			return DEVICE::read(read_count, finish_future, stop);
		}

		static constexpr I2CLightCommand write(uint8_t write_count = 0, bool finish_future = false, bool stop = false)
		{
			return DEVICE::write(write_count, finish_future, stop);
		}

		int launch_commands(PROXY<ABSTRACT_FUTURE> proxy, utils::range<I2CLightCommand> commands)
		{
			return device_->launch_commands(proxy, commands);
		}

		// Global status for whole future group
		volatile future::FutureStatus status_ = future::FutureStatus::NOT_READY;
		volatile int error_ = 0;

		void set_device(DEVICE& device)
		{
			device_ = &device;
		}

	private:
		// The device that uses this future
		DEVICE* device_ = nullptr;
		friend DEVICE;
	};
	/// @endcond

	//TODO DOCS
	template<typename MANAGER> class I2CFuturesGroup : 
		public AbstractI2CFuture<MANAGER>, public future::FutureStatusListener<typename MANAGER::ABSTRACT_FUTURE>
	{
		using PARENT = AbstractI2CFuture<MANAGER>;
		using ABSTRACT_FUTURE = typename MANAGER::ABSTRACT_FUTURE;

	protected:
		I2CFuturesGroup(ABSTRACT_FUTURE** futures, uint8_t size) : futures_{futures}, size_{size} {}

		bool start(typename PARENT::DEVICE& device)
		{
			this->set_device(device);
			return next_future();
		}

	private:
		bool next_future()
		{
			if (index_ == size_)
			{
				// Future is finished
				this->finish();
				return false;
			}
			ABSTRACT_FUTURE& future = *futures_[index_++];
			// Check if future has read, write or both
			const bool stop = (index_ == size_);
			const bool read = future.get_future_value_size_();
			const bool write = future.get_storage_value_size_();
			int error = 0;
			if (read && write)
			{
				error = this->launch_commands(future, {PARENT::write(), PARENT::read(0, false, stop)});
			}
			else if (read)
			{
				error = this->launch_commands(future, {PARENT::read(0, false, stop)});
			}
			else if (write)
			{
				error = this->launch_commands(future, {PARENT::write(0, false, stop)});
			}
			else
			{
				error = errors::EILSEQ;
			}
			return this->check_launch(error);
		}

		void on_status_change(UNUSED const ABSTRACT_FUTURE& future, future::FutureStatus status) final
		{
			// First check that current future was executed successfully
			if (!this->check_status(future, status))
				return;
			next_future();
		}

		ABSTRACT_FUTURE** futures_;
		uint8_t size_;
		uint8_t index_ = 0;
	};

	// template<typename F> 
	template<typename MANAGER> class I2CSameFutureGroup : 
		public AbstractI2CFuture<MANAGER>, public future::FutureStatusListener<typename MANAGER::ABSTRACT_FUTURE>
	{
		using PARENT = AbstractI2CFuture<MANAGER>;
		using CONTENT = WriteContent<uint8_t, true>;
		using F = WriteRegisterFuture<MANAGER, uint8_t, true>;

	public:
		I2CSameFutureGroup(uint16_t address, uint8_t count) : address_{address}, count_{count} {}

	private:
		bool start(typename PARENT::DEVICE& device)
		{
			this->set_device(device);
			return next_future();
		}

		bool next_future()
		{
			if (!count_)
			{
				// Future is finished
				this->finish();
				return false;
			}
			uint8_t reg = next_byte();
			uint8_t val = next_byte();
			const bool stop = (count_ == 0);
			future_.reset_(CONTENT{reg, val});
			return this->check_launch(
				this->launch_commands(future_, {PARENT::write(0, false, stop)}));
		}

		// Get the next byte, from the flash
		uint8_t next_byte()
		{
			uint8_t data = 0;
			--count_;
			return flash::read_flash(address_++, data);
		}

		void on_status_change(const typename PARENT::ABSTRACT_FUTURE& future, future::FutureStatus status) final
		{
			// First check that current future was executed successfully
			if (!this->check_status(future, status))
				return;
			next_future();
		}

		// Address of flah mempry holding information about bytes to write
		uint16_t address_;
		uint8_t count_;
		// The future reused for all writes
		F future_{CONTENT{}, this};
	};

	//TODO template with list of sizes for read and write futures?
	template<typename MANAGER> class ComplexI2CFuturesGroup :
		public AbstractI2CFuture<MANAGER>, public future::FutureStatusListener<typename MANAGER::ABSTRACT_FUTURE>
	{
	protected:
		ComplexI2CFuturesGroup(uint16_t flash_config) : address_{flash_config} {}

		enum class ProcessAction : uint8_t
		{
			DONE,
			MARKER,
			INCLUDE,
			READ,
			WRITE
		};

		//TODO Best API to do the most in superclass instead of subclass (and avoid templates)
		ProcessAction process_action()
		{
			while ((action_ = next_byte()) == actions::LOOP)
			{
				// Store loop address for later use and skip to next action
				loop_ = address_;
			}
			if (action_ == actions::END)
			{
				// Future is finished
				this->finish();
				return ProcessAction::DONE;
			}
			if (action_ == actions::MARKER) return ProcessAction::MARKER;
			if (action_ == actions::INCLUDE) return ProcessAction::INCLUDE;
			if (actions::is_read(action_)) return ProcessAction::READ;
			if (actions::is_write(action_)) return ProcessAction::WRITE;

			// Error: unrecognized action code
			this->error_ = errors::EILSEQ;
			this->status_ = future::FutureStatus::READY;
			return ProcessAction::DONE;
		}

		void loop()
		{
			address_ = loop_;
		}

		// Get the next byte, from the flash
		uint8_t next_byte()
		{
			uint8_t data = 0;
			return flash::read_flash(address_++, data);
		}

		bool is_stop() const
		{
			return actions::is_stop(action_);
		}

		uint8_t count() const
		{
			return actions::count(action_);
		}

	private:
		// Information, read from flash, about futures to create and launch
		uint16_t address_;
		// Address to restart from in case of a loop
		uint16_t loop_ = 0;
		// Current action code
		uint8_t action_ = 0;
	};
}

#endif /* I2C_DEVICE_UTILITIES_H */
/// @endcond
