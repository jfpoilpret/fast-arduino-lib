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

#include "i2c.h"

namespace i2c
{
	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, Status s)
	{
		switch (s)
		{
			case Status::OK:
			out << F("OK");
			break;
			
			case Status::START_TRANSMITTED:
			out << F("START_TRANSMITTED");
			break;
			
			case Status::REPEAT_START_TRANSMITTED:
			out << F("REPEAT_START_TRANSMITTED");
			break;
			
			case Status::SLA_W_TRANSMITTED_ACK:
			out << F("SLA_W_TRANSMITTED_ACK");
			break;
			
			case Status::SLA_W_TRANSMITTED_NACK:
			out << F("SLA_W_TRANSMITTED_NACK");
			break;
			
			case Status::DATA_TRANSMITTED_ACK:
			out << F("DATA_TRANSMITTED_ACK");
			break;
			
			case Status::DATA_TRANSMITTED_NACK:
			out << F("DATA_TRANSMITTED_NACK");
			break;
			
			case Status::ARBITRATION_LOST:
			out << F("ARBITRATION_LOST");
			break;
			
			case Status::SLA_R_TRANSMITTED_ACK:
			out << F("SLA_R_TRANSMITTED_ACK");
			break;
			
			case Status::SLA_R_TRANSMITTED_NACK:
			out << F("SLA_R_TRANSMITTED_NACK");
			break;
			
			case Status::DATA_RECEIVED_ACK:
			out << F("DATA_RECEIVED_ACK");
			break;
			
			case Status::DATA_RECEIVED_NACK:
			out << F("DATA_RECEIVED_NACK");
			break;
			
			default:
			out << F("UNKNOWN[") << streams::hex << uint8_t(s) << ']';
			break;
		}
		return out << streams::flush;
	}
	/// @endcond
}
