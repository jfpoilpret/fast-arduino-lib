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

#include "array.h"
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
		WriteContent() = default;
		WriteContent(uint8_t reg, const T& value)
			:	register_{reg}, value_{BIG_ENDIAN ? utils::change_endianness(value) : value} {}

	private:
		uint8_t register_{};
		T value_{}; 
	};
	/// @endcond

	// Future to read a register
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

		void reset_()
		{
			PARENT::reset_(REGISTER);
		}
	};

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

		void reset_(const T& input = T{})
		{
			PARENT::reset_(WriteContent{REGISTER, input});
		}
	};

	/// @cond notdocumented
	// Forward declaration of I2CDevice
	template<typename MANAGER> class I2CDevice;
	/// @endcond

	template<typename MANAGER> class I2CFutureHelper
	{
		static_assert(I2CManager_trait<MANAGER>::IS_I2CMANAGER, "MANAGER must be an I2C Manager");

	protected:
		using DEVICE = I2CDevice<MANAGER>;
		using ABSTRACT_FUTURE = typename MANAGER::ABSTRACT_FUTURE;
		template<typename T> using PROXY = typename MANAGER::template PROXY<T>;

		I2CFutureHelper() = default;

		// Check launch_commands() return and update own status if needed
		bool check_error(int error, ABSTRACT_FUTURE& target)
		{
			if (error == 0) return true;
			target.set_future_error_(error);
			return false;
		}

		bool check_status(const ABSTRACT_FUTURE& source, future::FutureStatus status, ABSTRACT_FUTURE& target)
		{
			// First check that current future was executed successfully
			if (status != future::FutureStatus::READY)
			{
				target.set_future_error_(source.error());
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

		void set_device(DEVICE& device)
		{
			device_ = &device;
		}

		DEVICE& device()
		{
			return *device_;
		}

	private:
		// The device that uses this future
		DEVICE* device_ = nullptr;
		friend DEVICE;
	};

	//TODO DOCS
	template<typename MANAGER> class AbstractI2CFuturesGroup : 
		public future::AbstractFuturesGroup<typename MANAGER::ABSTRACT_FUTURE>,
		public I2CFutureHelper<MANAGER>
	{
		using HELPER = I2CFutureHelper<MANAGER>;
		using GROUP = future::AbstractFuturesGroup<typename MANAGER::ABSTRACT_FUTURE>;

	protected:
		using ABSTRACT_FUTURE = typename MANAGER::ABSTRACT_FUTURE;
		using STATUS_LISTENER = typename GROUP::STATUS_LISTENER;

		AbstractI2CFuturesGroup(STATUS_LISTENER* status_listener = nullptr) : GROUP{status_listener} {}

		// Check launch_commands() return and update own status if needed
		bool check_error(int error)
		{
			return HELPER::check_error(error, *this);
		}

		bool check_status(const ABSTRACT_FUTURE& source, future::FutureStatus status)
		{
			return HELPER::check_status(source, status, *this);
		}
	};

	//TODO DOCS
	// This future groups several futures and executes them (on attached I2C device) all in sequence
	// status() and error() are automatically updated
	// Subclass should add get() method and return whatever is useful to caller (obtained from other futures)
	template<typename MANAGER> class I2CFuturesGroup : 
		public AbstractI2CFuturesGroup<MANAGER>
	{
		using PARENT = AbstractI2CFuturesGroup<MANAGER>;
		using MANAGER_TRAIT = I2CManager_trait<MANAGER>;
		using STATUS_LISTENER = typename PARENT::STATUS_LISTENER;
		using ABSTRACT_FUTURE = typename PARENT::ABSTRACT_FUTURE;

	protected:
		I2CFuturesGroup(
			ABSTRACT_FUTURE** futures, uint8_t size, STATUS_LISTENER* status_listener = nullptr)
			: PARENT{status_listener}, futures_{futures}, size_{size} {}

		bool start(typename PARENT::DEVICE& device)
		{
			PARENT::set_device(device);
			if (MANAGER_TRAIT::IS_ASYNC)
			{
				return next_future();
			}
			else
			{
				// In sync mode, we replace recursive calls (generated by on_status_change()) by a loop
				while (index_ != size_)
				{
					if (!next_future()) return false;
				}
				return true;
			}
		}

		void on_status_change(const ABSTRACT_FUTURE& future, future::FutureStatus status) final
		{
			PARENT::on_status_change(future, status);
			// In sync mode, we must avoid recursive calls generated by on_status_change()!
			if (MANAGER_TRAIT::IS_ASYNC)
			{
				// First check that current future was executed successfully
				if (status == future::FutureStatus::READY)
					next_future();
			}
		}

	private:
		bool next_future()
		{
			if (index_ == size_)
				// Future is finished
				return false;

			ABSTRACT_FUTURE& future = *futures_[index_++];
			// Check if future has read, write or both
			const bool stop = (index_ == size_);
			const bool read = future.get_future_value_size_();
			const bool write = future.get_storage_value_size_();
			int error = 0;
			if (read && write)
			{
				error = PARENT::launch_commands(future, {PARENT::write(), PARENT::read(0, false, stop)});
			}
			else if (read)
			{
				error = PARENT::launch_commands(future, {PARENT::read(0, false, stop)});
			}
			else if (write)
			{
				error = PARENT::launch_commands(future, {PARENT::write(0, false, stop)});
			}
			else
			{
				//FIXME we consider that any other future is an I2CFuturesgroup, which might not always be correct!
				I2CFuturesGroup& group = static_cast<I2CFuturesGroup&>(future);
				if (!group.start(PARENT::device()))
					error = errors::EILSEQ;
			}
			return PARENT::check_error(error);
		}

		ABSTRACT_FUTURE** futures_;
		uint8_t size_;
		uint8_t index_ = 0;
	};

	//TODO DOCS
	// This future executes several times the same future but with distinct input (I2C write) everytime
	// template<typename F> 
	template<typename MANAGER> class I2CSameFutureGroup : public AbstractI2CFuturesGroup<MANAGER>
	{
		using PARENT = AbstractI2CFuturesGroup<MANAGER>;
		using MANAGER_TRAIT = I2CManager_trait<MANAGER>;
		using STATUS_LISTENER = typename PARENT::STATUS_LISTENER;
		using ABSTRACT_FUTURE = typename PARENT::ABSTRACT_FUTURE;
		using F = WriteRegisterFuture<MANAGER, uint8_t, true>;
		using CONTENT = WriteContent<uint8_t, true>;
		static constexpr uint8_t FUTURE_SIZE = F::IN_SIZE;

	public:
		I2CSameFutureGroup(uint16_t address, uint8_t size, STATUS_LISTENER* status_listener = nullptr)
			: PARENT{status_listener}, address_{address}, size_{size}
		{
			PARENT::init({&future_}, size / FUTURE_SIZE);
		}

	//TODO set protected (or private?) and set friend DEVICE
	// protected:
		bool start(typename PARENT::DEVICE& device)
		{
			PARENT::set_device(device);
			if (MANAGER_TRAIT::IS_ASYNC)
			{
				return next_future();
			}
			else
			{
				// In sync mode, we replace recursive calls (generated by on_status_change()) by a loop
				while (size_)
				{
					if (!next_future()) return false;
				}
				return true;
			}
		}

	private:
		bool next_future()
		{
			if (size_ == 0)
				// Future is finished already
				return false;

			uint8_t reg = next_byte();
			uint8_t val = next_byte();
			const bool stop = (size_ == 0);
			future_.reset_(CONTENT{reg, val});
			return PARENT::check_error(
				PARENT::launch_commands(future_, {PARENT::write(0, false, stop)}));
		}

		// Get the next byte, from the flash
		uint8_t next_byte()
		{
			uint8_t data = 0;
			--size_;
			return flash::read_flash(address_++, data);
		}

		void on_status_change(const ABSTRACT_FUTURE& future, future::FutureStatus status) final
		{
			PARENT::on_status_change(future, status);
			// In sync mode, we must avoid recursive calls generated by on_status_change()!
			if (MANAGER_TRAIT::IS_ASYNC)
			{
				// First check that current future was executed successfully
				if (status == future::FutureStatus::READY)
					next_future();
			}
		}

		// Address of flah mempry holding information about bytes to write
		uint16_t address_;
		uint8_t size_;
		// The future reused for all writes
		F future_{0, 0};
	};

#ifdef EXPERIMENTAL_API
	/**
	 * This namespace contains action codes for use in flash memory configuration arrays
	 * used by ComplexI2CFuturesGroup.
	 * 
	 * For all I2C futures using flash read-only date to write to device, the following
	 * convention is used:
	 * 1. The whole array is broken down into individual lines, each representing an action
	 * 2. Each action is starting with an action byte
	 * 3. Each action byte will have additional bytes depending on its actual action
	 * 4. The last line uses action 0x00 (end of stream)
	 * 
	 * Action bytes are defined as follows:
	 * - 0x00	end of stream marker (Future is complete)
	 * - 0x1H	write (1+H) bytes to the device; the action is followed by the register index and H bytes to write
	 * - 0x9H	same as 0x1H, but this is the last command for this future (force stop is needed)
	 * - 0x2H	read H bytes from device register, after writing the register index; 
	 * 			the action is followed by the register index
	 * - 0xAH	same as 0x2H, but this is the last command for this future (force stop is needed)
	 * - 0x30	special marker, used by the Future code to do something special at this point; this is followed by
	 * 			one byte that can be any free value indicating to code what shall be done
	 * - 0x31	special marker to meorize the current flash address for looping back to it
	 * 			when calling `ComplexI2CFuturesGroup.loop()` method.
	 * - 0x40	special marker, to include another Future into this Future; this is followed by one byte
	 * 			that can be any free value used to idnicate which Future shall be used
	 * - other codes are kept for future (and Future) use
	 * 
	 * @sa ComplexI2CFuturesGroup
	 */
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

	//FIXME wont't work in SYNC mode (too many recursion calls through future listeners)!!!
	template<typename MANAGER> class ComplexI2CFuturesGroup : public AbstractI2CFuturesGroup<MANAGER>
	{
		using PARENT = AbstractI2CFuturesGroup<MANAGER>;
		using MANAGER_TRAIT = I2CManager_trait<MANAGER>;
		using STATUS_LISTENER = typename PARENT::STATUS_LISTENER;
		using ABSTRACT_FUTURE = typename PARENT::ABSTRACT_FUTURE;

	protected:
		ComplexI2CFuturesGroup(uint16_t flash_config, STATUS_LISTENER* status_listener = nullptr)
			: PARENT{status_listener}, address_{flash_config} {}

		enum class ProcessAction : uint8_t
		{
			DONE,
			MARKER,
			INCLUDE,
			READ,
			WRITE
		};

		//TODO Find better API to do the most in superclass instead of subclass (and avoid templates)
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
				PARENT::set_future_finish_();
				return ProcessAction::DONE;
			}
			if (action_ == actions::MARKER) return ProcessAction::MARKER;
			if (action_ == actions::INCLUDE) return ProcessAction::INCLUDE;
			if (actions::is_read(action_)) return ProcessAction::READ;
			if (actions::is_write(action_)) return ProcessAction::WRITE;

			// Error: unrecognized action code
			PARENT::check_error(errors::EILSEQ);
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
#endif
}

#endif /* I2C_DEVICE_UTILITIES_H */
/// @endcond
