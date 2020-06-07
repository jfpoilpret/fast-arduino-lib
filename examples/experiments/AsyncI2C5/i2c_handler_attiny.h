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

#ifndef I2C_HANDLER_ATTINY_HH
#define I2C_HANDLER_ATTINY_HH

#include <util/delay_basic.h>

#include <fastarduino/array.h>
#include <fastarduino/i2c.h>
#include <fastarduino/future.h>
#include <fastarduino/queue.h>
#include <fastarduino/time.h>
#include <fastarduino/interrupts.h>
#include <fastarduino/bits.h>
#include <fastarduino/utilities.h>

#include "i2c_handler_common.h"

#define REGISTER_I2C_ISR(MODE)

namespace i2c
{
	template<I2CMode MODE_> class I2CHandler : public AbstractI2CHandler<MODE_>
	{
		using SUPER = AbstractI2CHandler<MODE_>;

	public:
		I2CHandler(	I2CErrorPolicy error_policy = I2CErrorPolicy::CLEAR_ALL_COMMANDS,
					I2C_DEBUG_HOOK hook = nullptr)
			:	SUPER{error_policy, hook}
		{
			// // set SDA/SCL default directions
			// SUPER::TRAIT::DDR &= bits::CBV8(SUPER::TRAIT::BIT_SDA);
			// // SUPER::TRAIT::PORT |= bits::BV8(SUPER::TRAIT::BIT_SDA);
			// SUPER::TRAIT::DDR |= bits::BV8(SUPER::TRAIT::BIT_SCL);
			// SUPER::TRAIT::PORT |= bits::BV8(SUPER::TRAIT::BIT_SCL);

			// set SDA/SCL default directions
			SUPER::TRAIT::PORT |= bits::BV8(SUPER::TRAIT::BIT_SDA);
			SUPER::TRAIT::PORT |= bits::BV8(SUPER::TRAIT::BIT_SCL);
			SUPER::TRAIT::DDR |= bits::BV8(SUPER::TRAIT::BIT_SDA);
			SUPER::TRAIT::DDR |= bits::BV8(SUPER::TRAIT::BIT_SCL);
		}

		void begin()
		{
			synchronized begin_();
		}
		void end()
		{
			synchronized end_();
		}

		void begin_()
		{
			// 1. Force 1 to data
			USIDR_ = UINT8_MAX;
			// 2. Enable TWI
			// Set USI I2C mode, enable software clock strobe (USITC)
			USICR_ = bits::BV8(USIWM1, USICS1, USICLK);
			// Clear all interrupt flags
			USISR_ = bits::BV8(USISIF, USIOIF, USIPF, USIDC);
			// 3. Set SDA as output
			SDA_OUTPUT();
		}
		void end_()
		{
			// Disable TWI
			USICR_ = 0;
			// Set SDA back to INPUT
			SDA_INPUT();
		}

		bool ensure_num_commands_(UNUSED uint8_t num_commands)
		{
			return true;
		}

	private:
		static constexpr const typename SUPER::REG8 USIDR_{USIDR};
		static constexpr const typename SUPER::REG8 USISR_{USISR};
		static constexpr const typename SUPER::REG8 USICR_{USICR};

		// Constant values for USISR
		// For byte transfer, we set counter to 0 (16 ticks => 8 clock cycles)
		static constexpr const uint8_t USISR_DATA = bits::BV8(USISIF, USIOIF, USIPF, USIDC);
		// For acknowledge bit, we start counter at 0E (2 ticks: 1 raising and 1 falling edge)
		static constexpr const uint8_t USISR_ACK = USISR_DATA | (0x0E << USICNT0);

		// Timing constants for current mode (as per I2C specifications)
		static constexpr const uint8_t T_HD_STA = utils::calculate_delay1_count(MODE_ == I2CMode::STANDARD ? 4.0 : 0.6);
		static constexpr const uint8_t T_LOW = utils::calculate_delay1_count(MODE_ == I2CMode::STANDARD ? 4.7 : 1.3);
		static constexpr const uint8_t T_HIGH = utils::calculate_delay1_count(MODE_ == I2CMode::STANDARD ? 4.0 : 0.6);
		static constexpr const uint8_t T_SU_STA = utils::calculate_delay1_count(MODE_ == I2CMode::STANDARD ? 4.7 : 0.6);
		static constexpr const uint8_t T_SU_STO = utils::calculate_delay1_count(MODE_ == I2CMode::STANDARD ? 4.0 : 0.6);
		static constexpr const uint8_t T_BUF = utils::calculate_delay1_count(MODE_ == I2CMode::STANDARD ? 4.7 : 1.3);

		void SCL_HIGH()
		{
			SUPER::TRAIT::PORT |= bits::BV8(SUPER::TRAIT::BIT_SCL);
			SUPER::TRAIT::PIN.loop_until_bit_set(SUPER::TRAIT::BIT_SCL);
		}

		void SCL_LOW()
		{
			SUPER::TRAIT::PORT &= bits::CBV8(SUPER::TRAIT::BIT_SCL);
		}

		void SDA_HIGH()
		{
			SUPER::TRAIT::PORT |= bits::BV8(SUPER::TRAIT::BIT_SDA);
		}

		void SDA_LOW()
		{
			SUPER::TRAIT::PORT &= bits::CBV8(SUPER::TRAIT::BIT_SDA);
		}

		void SDA_INPUT()
		{
			SUPER::TRAIT::DDR &= bits::CBV8(SUPER::TRAIT::BIT_SDA);
			// SUPER::TRAIT::PORT |= bits::BV8(SUPER::TRAIT::BIT_SDA);
		}

		void SDA_OUTPUT()
		{
			SUPER::TRAIT::DDR |= bits::BV8(SUPER::TRAIT::BIT_SDA);
		}

		void last_command_pushed_()
		{
			// Check if previously executed command already did a STOP
			if ((!this->command_.type.force_stop) && (!stopped_already_) && (!clear_commands_))
				exec_stop_();
			this->command_ = I2CCommand::none();
			clear_commands_ = false;
			stopped_already_ = false;
		}

		void start_impl_()
		{
			// Ensure SCL is HIGH
			SCL_HIGH();
			// Wait for Tsu-sta
			_delay_loop_1(T_SU_STA);
			// Now we can generate start condition
			// Force SDA low for Thd-sta
			SDA_LOW();
			_delay_loop_1(T_HD_STA);
			// Pull SCL low
			SCL_LOW();
			//			_delay_loop_1(T_LOW());
			// Release SDA (force high)
			SDA_HIGH();
			//TODO check START transmission with USISIF flag?
			bool ok = USISR_ & bits::BV8(USISIF);
			// this->status_ = (ok ? this->expected_status_ : Status::ARBITRATION_LOST);
			this->status_ = this->expected_status_;
		}

		void send_byte_impl(uint8_t data)
		{
			// Set SCL low
			SCL_LOW();
			// Transfer address byte
			USIDR_ = data;
			transfer(USISR_DATA);
			// For acknowledge, first set SDA as input
			SDA_INPUT();
			bool ok = ((transfer(USISR_ACK) & 0x01U) == 0);
			// The expected status is one Status _ACK values
			// When not OK, it shall be changed to the _NACK matching value
			// This can be done by simply adding 0x08 to the ACK value.
			this->status_ = this->expected_status_ + (ok ? 0 : 0x08);
		}

		uint8_t receive_impl(bool last_byte)
		{
			SDA_INPUT();
			uint8_t data = transfer(USISR_DATA);
			// Send ACK (or NACK if last byte)
			USIDR_ = (last_byte ? UINT8_MAX : 0x00);
			transfer(USISR_ACK);
			return data;
		}

		void stop_impl()
		{
			// Pull SDA low
			SDA_LOW();
			// Release SCL
			SCL_HIGH();
			_delay_loop_1(T_SU_STO);
			// Release SDA
			SDA_HIGH();
			_delay_loop_1(T_BUF);
		}

		uint8_t transfer(uint8_t USISR_count)
		{
			//Rework according to AVR310
			// Init counter (8 bits or 1 bit for acknowledge)
			USISR_ = USISR_count;
			do
			{
				_delay_loop_1(T_LOW);
				// clock strobe (SCL raising edge)
				// USICR_ |= bits::BV8(USITC);
				USICR_ = bits::BV8(USIWM1, USICS1, USICLK, USITC);
				SUPER::TRAIT::PIN.loop_until_bit_set(SUPER::TRAIT::BIT_SCL);
				_delay_loop_1(T_HIGH);
				// clock strobe (SCL falling edge)
				// USICR_ |= bits::BV8(USITC);
				USICR_ = bits::BV8(USIWM1, USICS1, USICLK, USITC);
			}
			while ((USISR_ & bits::BV8(USIOIF)) == 0);
			_delay_loop_1(T_LOW);
			// Read data
			uint8_t data = USIDR_;
			USIDR_ = UINT8_MAX;
			// Release SDA
			SDA_OUTPUT();
			return data;
		}

		// Push one byte of a command to the queue, and possibly initiate a new transmission right away
		bool push_command(const I2CCommand& command)
		{
			synchronized return push_command_(command);
		}

		bool push_command_(const I2CCommand& command)
		{
			// Check command is not empty
			if (command.type.none) return true;
			if (clear_commands_) return false;
			// Execute command immediately from start to optional stop
			// Check if start or repeat start (depends on previously executed command)
			if (this->command_.type.none || this->command_.type.force_stop)
				exec_start_();
			else
				exec_repeat_start_();
			this->command_ = command;
			if (!handle_no_error()) return false;

			if (command.type.write)
			{
				// Send device address
				exec_send_slaw_();
				if (!handle_no_error()) return false;
				// Send content
				while (future::AbstractFutureManager::instance().get_storage_value_size_(command.future_id) > 0)
				{
					exec_send_data_();
					if (!handle_no_error()) return false;
				}
			}
			else
			{
				// Send device address
				exec_send_slar_();
				if (!handle_no_error()) return false;
				// Receive content
				while (future::AbstractFutureManager::instance().get_future_value_size_(command.future_id) > 0)
				{
					exec_receive_data_();
					if (!handle_no_error()) return false;
				}
			}

			// Check if we must force finish the future
			if (command.type.finish_future)
				future::AbstractFutureManager::instance().set_future_finish_(command.future_id);
			// Check if we must force a STOP
			if (command.type.force_stop)
				exec_stop_();
			return true;
		}

		// Low-level methods to handle the bus in an asynchronous way
		void exec_start_()
		{
			this->call_hook(DebugStatus::START);
			this->expected_status_ = Status::START_TRANSMITTED;
			start_impl_();
		}
		void exec_repeat_start_()
		{
			this->call_hook(DebugStatus::REPEAT_START);
			this->expected_status_ = Status::REPEAT_START_TRANSMITTED;
			start_impl_();
		}
		void exec_send_slar_()
		{
			this->call_hook(DebugStatus::SLAR, this->command_.target);
			// Read device address from queue
			this->expected_status_ = Status::SLA_R_TRANSMITTED_ACK;
			send_byte_impl(this->command_.target | 0x01U);
		}
		void exec_send_slaw_()
		{
			this->call_hook(DebugStatus::SLAW, this->command_.target);
			// Read device address from queue
			this->expected_status_ = Status::SLA_W_TRANSMITTED_ACK;
			send_byte_impl(this->command_.target);
		}
		void exec_send_data_()
		{
			// Determine next data byte
			uint8_t data = 0;
			bool ok = future::AbstractFutureManager::instance().get_storage_value_(this->command_.future_id, data);
			this->call_hook(DebugStatus::SEND, data);
			// This should only happen if there are 2 concurrent consumers for that Future
			if (!ok)
				future::AbstractFutureManager::instance().set_future_error_(this->command_.future_id, errors::EILSEQ);
			this->call_hook(ok ? DebugStatus::SEND_OK : DebugStatus::SEND_ERROR);
			this->expected_status_ = Status::DATA_TRANSMITTED_ACK;
			send_byte_impl(data);
		}
		void exec_receive_data_()
		{
			// Is this the last byte to receive?
			uint8_t data;
			if (future::AbstractFutureManager::instance().get_future_value_size_(this->command_.future_id) == 1)
			{
				this->call_hook(DebugStatus::RECV_LAST);
				// Send NACK for the last data byte we want
				this->expected_status_ = Status::DATA_RECEIVED_NACK;
				data = receive_impl(true);
			}
			else
			{
				this->call_hook(DebugStatus::RECV);
				// Send ACK for data byte if not the last one we want
				this->expected_status_ = Status::DATA_RECEIVED_ACK;
				data = receive_impl(false);
			}
			// Ensure status is set properly
			this->status_ = this->expected_status_;
			// Fill future
			bool ok = future::AbstractFutureManager::instance().set_future_value_(this->command_.future_id, data);
			// This should only happen in case there are 2 concurrent providers for this future
			//FIXME handle error!
			if (!ok)
				future::AbstractFutureManager::instance().set_future_error_(this->command_.future_id, errors::EILSEQ);
			this->call_hook(ok ? DebugStatus::RECV_OK : DebugStatus::RECV_ERROR, data);
		}
		void exec_stop_(bool error = false)
		{
			this->call_hook(DebugStatus::STOP);
			stop_impl();
			if (!error)
				this->expected_status_ = 0;
			this->command_ = I2CCommand::none();
			// If so then delay 4.0us + 4.7us (100KHz) or 0.6us + 1.3us (400KHz)
			// (ATMEGA328P datasheet 29.7 Tsu;sto + Tbuf)
			_delay_loop_1(DELAY_AFTER_STOP);
			stopped_already_ = true;
		}

		bool handle_no_error()
		{
			if (this->check_no_error()) return true;

			switch (this->error_policy_)
			{
				case I2CErrorPolicy::CLEAR_ALL_COMMANDS:
				// Clear all pending transactions from queue
				case I2CErrorPolicy::CLEAR_TRANSACTION_COMMANDS:
				// Clear command belonging to the same transaction (i.e. same future)
				// ie forbid any new command until last command (add new flag for that)
				clear_commands_ = true;
				break;
			}
			// In case of an error, immediately send a STOP condition
			exec_stop_(true);
			return false;
		}

		static constexpr const float STANDARD_DELAY_AFTER_STOP_US = 4.0 + 4.7;
		static constexpr const float FAST_DELAY_AFTER_STOP_US = 0.6 + 1.3;
		static constexpr const float DELAY_AFTER_STOP_US =
			(MODE_ == I2CMode::STANDARD ? STANDARD_DELAY_AFTER_STOP_US : FAST_DELAY_AFTER_STOP_US);
		static constexpr const uint8_t DELAY_AFTER_STOP = utils::calculate_delay1_count(DELAY_AFTER_STOP_US);

		bool clear_commands_ = false;
		bool stopped_already_ = false;

		template<I2CMode> friend class AbstractDevice;
	};
}

#endif /* I2C_HANDLER_ATTINY_HH */
/// @endcond
