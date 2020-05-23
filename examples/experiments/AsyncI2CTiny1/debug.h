/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * This one is a proof of concept on I2C asynchronous handling, to be later integrated
 * to FastArduino library.
 * For tests, I just use a DS1307 connected through I2C (SDA/SCL) to an Arduino UNO.
 */

#ifndef DEBUG_HH
#define DEBUG_HH

#include <fastarduino/streams.h>
#include "i2c_handler.h"

#if defined(DEBUG_STEPS) || defined(DEBUG_SEND_OK) || defined(DEBUG_SEND_ERR) || defined(DEBUG_RECV_OK) || defined(DEBUG_RECV_ERR)
// Add utility ostream manipulator for FutureStatus
static const flash::FlashStorage* convert(i2c::DebugStatus s)
{
	switch (s)
	{
		case i2c::DebugStatus::START:
		return F("START");

		case i2c::DebugStatus::REPEAT_START:
		return F("REPEAT_START");

		case i2c::DebugStatus::SLAW:
		return F("SLAW");

		case i2c::DebugStatus::SLAR:
		return F("SLAR");

		case i2c::DebugStatus::SEND:
		return F("SEND");

		case i2c::DebugStatus::RECV:
		return F("RECV");

		case i2c::DebugStatus::RECV_LAST:
		return F("RECV_LAST");

		case i2c::DebugStatus::STOP:
		return F("STOP");

		case i2c::DebugStatus::SEND_OK:
		return F("SEND_OK");

		case i2c::DebugStatus::SEND_ERROR:
		return F("SEND_ERROR");

		case i2c::DebugStatus::RECV_OK:
		return F("RECV_OK");

		case i2c::DebugStatus::RECV_ERROR:
		return F("RECV_ERROR");
	}
}

streams::ostream& operator<<(streams::ostream& out, i2c::DebugStatus s)
{
	return out << convert(s);
}

#ifdef ARDUINO_UNO
static constexpr uint8_t MAX_DEBUG = 128;
#else
static constexpr uint8_t MAX_DEBUG = 64;
#endif

static i2c::DebugStatus debug_status[MAX_DEBUG];
static uint8_t debug_data[MAX_DEBUG];
static uint8_t debug_index = 0;

static void trace_states(streams::ostream& out)
{
	for (uint8_t i = 0; i < debug_index; ++i)
		out << debug_status[i] << ' ' << streams::hex << debug_data[i] << streams::endl;
	if (debug_index >= MAX_DEBUG)
		out << F("##### DEBUG OVERFLOW #####") << streams::endl;
	debug_index = 0;
}

static void debug_hook(i2c::DebugStatus status, uint8_t data)
{
	if (debug_index >= MAX_DEBUG) return;
	switch (status)
	{
		case i2c::DebugStatus::START:
		case i2c::DebugStatus::REPEAT_START:
		case i2c::DebugStatus::STOP:
		case i2c::DebugStatus::SLAW:
		case i2c::DebugStatus::SLAR:
		case i2c::DebugStatus::SEND:
		case i2c::DebugStatus::RECV:
		case i2c::DebugStatus::RECV_LAST:
		#ifdef DEBUG_STEPS
		debug_status[debug_index] = status;
		debug_data[debug_index++] = data;
		#endif
		break;

		case i2c::DebugStatus::SEND_OK:
		#ifdef DEBUG_SEND_OK
		debug_status[debug_index] = status;
		debug_data[debug_index++] = data;
		#endif
		break;

		case i2c::DebugStatus::SEND_ERROR:
		#ifdef DEBUG_SEND_ERR
		debug_status[debug_index] = status;
		debug_data[debug_index++] = data;
		#endif
		break;

		case i2c::DebugStatus::RECV_OK:
		#ifdef DEBUG_RECV_OK
		debug_status[debug_index] = status;
		debug_data[debug_index++] = data;
		#endif
		break;

		case i2c::DebugStatus::RECV_ERROR:
		#ifdef DEBUG_RECV_ERR
		debug_status[debug_index] = status;
		debug_data[debug_index++] = data;
		#endif
		break;
	}
}
#else
static i2c::I2C_DEBUG_HOOK debug_hook = nullptr;
#endif

#endif /* DEBUG_HH */
