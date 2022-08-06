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

/// @cond api

#ifndef I2C_HANDLER_COMMON_HH
#define I2C_HANDLER_COMMON_HH

#include <stdint.h>
#include <fastarduino/i2c.h>
#include <fastarduino/streams.h>

namespace i2c
{
	// Type of commands in queue
	class I2CCommandType
	{
	public:
		constexpr I2CCommandType()
			: none{true}, write{false}, force_stop{false}, finish_future{false} {}
		constexpr I2CCommandType(const I2CCommandType&) = default;
		constexpr I2CCommandType& operator=(const I2CCommandType&) = default;

	private:
		constexpr I2CCommandType(bool write, bool force_stop, bool finish_future)
			: none{false}, write{write}, force_stop{force_stop}, finish_future{finish_future} {}

		// true if this is an empty command
		bool none : 1;
		// true if this is a write command, false for a read command
		bool write : 1;
		// true if a STOP condition must absolutely be forced at the end of this command
		bool force_stop : 1;
		// true if aasociated future is void and must eb forced finished after this command
		bool finish_future : 1;

		friend class I2CCommand;
		friend streams::ostream& operator<<(streams::ostream&, const I2CCommandType&);
		friend bool operator==(const I2CCommandType&, const I2CCommandType&);
		friend bool operator!=(const I2CCommandType&, const I2CCommandType&);
	};

	streams::ostream& operator<<(streams::ostream& out, const I2CCommandType& t)
	{
		if (t.none) return out << F("NONE") << streams::flush;
		out << (t.write ? F("WRITE") : F("READ"));
		if (t.force_stop)
			out << F("[STOP]");
		if (t.finish_future)
			out << F("[FINISH]");
		return out << streams::flush;
	}
	bool operator==(const I2CCommandType& a, const I2CCommandType& b)
	{
		return	(a.none == b.none)
			&&	(a.write == b.write)
			&&	(a.force_stop == b.force_stop)
			&&	(a.finish_future == b.finish_future);
	}
	bool operator!=(const I2CCommandType& a, const I2CCommandType& b)
	{
		return	(a.none != b.none)
			||	(a.write != b.write)
			||	(a.force_stop != b.force_stop)
			||	(a.finish_future != b.finish_future);
	}

	// Command in the queue
	class I2CCommand
	{
	public:
		constexpr I2CCommand() = default;
		constexpr I2CCommand(const I2CCommand&) = default;
		constexpr I2CCommand& operator=(const I2CCommand&) = default;

		static constexpr I2CCommand none()
		{
			return I2CCommand{};
		}

		static constexpr I2CCommand read(uint8_t target, bool force_stop, uint8_t future_id, bool finish_future)
		{
			return I2CCommand{I2CCommandType{false, force_stop, finish_future}, target, future_id};
		}
		static constexpr I2CCommand write(uint8_t target, bool force_stop, uint8_t future_id, bool finish_future)
		{
			return I2CCommand{I2CCommandType{true, force_stop, finish_future}, target, future_id};
		}

	private:
		constexpr I2CCommand(I2CCommandType type, uint8_t target, uint8_t future_id)
			: type{type}, target{target}, future_id{future_id} {}

		// Type of this command
		I2CCommandType type = I2CCommandType{};
		// Address of the target device (on 8 bits, already left-shifted)
		uint8_t target = 0;
		uint8_t future_id = 0;

		friend streams::ostream& operator<<(streams::ostream&, const I2CCommand&);
		friend bool operator==(const I2CCommand&, const I2CCommand&);
		friend bool operator!=(const I2CCommand&, const I2CCommand&);
	};

	streams::ostream& operator<<(streams::ostream& out, const I2CCommand& c)
	{
		out	<< '{' << c.type << ',' 
			<< streams::hex << c.target << ',' 
			<< streams::dec << c.future_id << '}' << streams::flush;
		return out;
	}
	bool operator==(const I2CCommand& a, const I2CCommand& b)
	{
		return (a.type == b.type) && (a.target == b.target) && (a.future_id == b.future_id);
	}
	bool operator!=(const I2CCommand& a, const I2CCommand& b)
	{
		return (a.type != b.type) || (a.target != b.target) || (a.future_id != b.future_id);
	}
}

#endif /* I2C_HANDLER_COMMON_HH */
/// @endcond
