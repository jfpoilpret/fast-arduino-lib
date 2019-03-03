//   Copyright 2016-2018 Jean-Francois Poilpret
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
 * API to handle EEPROM access in read and write modes.
 */
#ifndef EEPROM_H
#define EEPROM_H

#include "boards/board_traits.h"
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "interrupts.h"
#include "utilities.h"
#include "queue.h"

/**
 * Register the necessary ISR (interrupt Service Routine) for eeprom::QueuedWriter 
 * to work properly.
 * @sa eeprom::QueuedWriter
 */
#define REGISTER_EEPROM_ISR() \
	REGISTER_ISR_METHOD_RETURN_(EE_READY_vect, eeprom::QueuedWriter, &eeprom::QueuedWriter::on_ready, bool)

/**
 * Register the necessary ISR (interrupt Service Routine) for eeprom::QueuedWriter 
 * to work properly, along with a callback method taht will be called everytime
 * all pending queued write operations are complete.
 * 
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function).
 * 
 * @sa eeprom::QueuedWriter
 */
#define REGISTER_EEPROM_ISR_METHOD(HANDLER, CALLBACK)                              \
	ISR(EE_READY_vect)                                                             \
	{                                                                              \
		using WRT_HANDLER = eeprom::QueuedWriter;                                  \
		using WRT_HOLDER = HANDLER_HOLDER_(WRT_HANDLER);                           \
		if (WRT_HOLDER::handler()->on_ready()) CALL_HANDLER_(HANDLER, CALLBACK)(); \
	}

/**
 * Register the necessary ISR (interrupt Service Routine) for eeprom::QueuedWriter 
 * to work properly, along with a callback method taht will be called everytime
 * all pending queued write operations are complete.
 * 
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered
 * 
 * @sa eeprom::QueuedWriter
 */
#define REGISTER_EEPROM_ISR_FUNCTION(CALLBACK)             \
	ISR(EE_READY_vect)                                     \
	{                                                      \
		using WRT_HANDLER = eeprom::QueuedWriter;          \
		using WRT_HOLDER = HANDLER_HOLDER_(WRT_HANDLER);   \
		if (WRT_HOLDER::handler()->on_ready()) CALLBACK(); \
	}

/**
 * Defines the API for accessing the EEPROM embedded in each AVR MCU.
 * The API offers blocking read, blocking write and queued (asynchronous) write.
 */
namespace eeprom
{
	/**
	 * Collection of static methods to read or write the AVR EEPROM.
	 * All API here is blocking, i.e. will not return until read or write is
	 * complete.
	 * 
	 * All API here exists in 2 flavors, differing in how the EEPROM cell address
	 * is passed :
	 * - as an absolute `uint16_t`  location, from `0` to max EEPROM size
	 * - as the address of a variable that was declared with `EEMEM` attribute
	 * 
	 * The second is generally more convenient as it allows you to:
	 * - not care about actual variable locations in EEPROM (if you have many
	 * distinct content to store, defined in different source files); location
	 * will be determined for you at compile-time
	 * - prepare initial values for EEPROM content, for which **make** will generate
	 * an `.eep` file that you can separately upload to EEPROM
	 * 
	 * The following snippet shows briefly how you can use the `EEMEM` flavor:
	 * @code
	 * // Declare EEPROM stored content
	 * static uint32_t eeprom_hash EEMEM = 0;
	 * ...
	 * uint32_t read_hash()
	 * {
	 *     uint32_t hash;
	 *     // Read uint32_t value located at "eeprom_hash" in EEPROM, and put it into hash variable
	 *     EEPROM::read(&eeprom_hash, hash);
	 *     return hash;
	 * }
	 * void write_hash(uint32_t hash)
	 * {
	 *     EEPROM::write(&eeprom_hash, hash);
	 * }
	 * @endcode  
	 */
	class EEPROM
	{
	public:
		/**
		 * Read value of type @p T stored in EEPROM at @p address.
		 * @note Whatever @p T type, this method never calls its assignment 
		 * operator but simply copies, byte per byte, content from EEPROM to internal 
		 * content of the type, no @p T code gets executed.
		 * 
		 * @tparam T the type of content to be read (determines the number of bytes to read)
		 * @param address the location to be read in EEPROM; @p address is typically
		 * obtained as `&variable` where `variable` is a global variable that was
		 * declared with `EEMEM` attribute.
		 * @param value a reference to the variable that will be assigned the content read
		 * from EEPROM
		 * @retval true if read was successful
		 * @retval false if read failed, i.e. if @p address is outside EEPROM bounds
		 * @sa read(uint16_t, T&)
		 */
		template<typename T> inline static bool read(const T* address, T& value)
		{
			return read((uint16_t) address, value);
		}

		/**
		 * Read value of type @p T stored in EEPROM at @p address.
		 * @note Whatever @p T type, this method never calls its assignment 
		 * operator but simply copies, byte per byte, content from EEPROM to internal 
		 * content of the type, no @p T code gets executed.
		 * 
		 * @tparam T the type of content to be read (determines the number of bytes to read)
		 * @param address the location to be read in EEPROM; @p address is provided as an 
		 * absolute location, from `0` to the maximum EEPROM size
		 * @param value a reference to the variable that will be assigned the content read
		 * from EEPROM
		 * @retval true if read was successful
		 * @retval false if read failed, i.e. if @p address is outside EEPROM bounds
		 * @sa read(const T*, T&)
		 */
		template<typename T> static bool read(uint16_t address, T& value)
		{
			if (!check(address, sizeof(T))) return false;
			uint8_t* v = (uint8_t*) &value;
			for (uint16_t i = 0; i < sizeof(T); ++i) blocked_read(address++, *v++);
			return true;
		}

		/**
		 * Read an array of @p count values of type @p T stored in EEPROM at @p address.
		 * @note Whatever @p T type, this method never calls its assignment 
		 * operator but simply copies, byte per byte, content from EEPROM to internal 
		 * content of the type, no @p T code gets executed.
		 * 
		 * @tparam T the type of content to be read (determines the number of bytes to read)
		 * @param address the location to be read in EEPROM; @p address is typically
		 * obtained as `variable` where `variable` is a global variable array that was
		 * declared with `EEMEM` attribute.
		 * @param value a pointer to the variable array that will be assigned the content read
		 * from EEPROM
		 * @param count the number of items of type @p T to be read
		 * @retval true if read was successful
		 * @retval false if read failed, i.e. if @p address is outside EEPROM bounds
		 * @sa read(uint16_t, T*, uint16_t)
		 */
		template<typename T> inline static bool read(const T* address, T* value, uint16_t count)
		{
			return read((uint16_t) address, value, count);
		}

		/**
		 * Read an array of @p count values of type @p T stored in EEPROM at @p address.
		 * @note Whatever @p T type, this method never calls its assignment 
		 * operator but simply copies, byte per byte, content from EEPROM to internal 
		 * content of the type, no @p T code gets executed.
		 * 
		 * @tparam T the type of content to be read (determines the number of bytes to read)
		 * @param address the location to be read in EEPROM; @p address is provided as an 
		 * absolute location, from `0` to the maximum EEPROM size
		 * @param value a pointer to the variable array that will be assigned the content read
		 * from EEPROM
		 * @param count the number of items of type @p T to be read
		 * @retval true if read was successful
		 * @retval false if read failed, i.e. if @p address is outside EEPROM bounds
		 * @sa read(const T*, T*, uint16_t)
		 */
		template<typename T> static bool read(uint16_t address, T* value, uint16_t count)
		{
			if (!check(address, count * sizeof(T))) return false;
			uint8_t* v = (uint8_t*) value;
			for (uint16_t i = 0; i < count * sizeof(T); ++i) blocked_read(address++, *v++);
			return true;
		}

		/**
		 * Read one byte stored in EEPROM at @p address.
		 * 
		 * @param address the location to be read in EEPROM; @p address is typically
		 * obtained as `&variable` where `variable` is a global variable that was
		 * declared with `EEMEM` attribute.
		 * @param value a reference to the variable that will be assigned the content read
		 * from EEPROM
		 * @retval true if read was successful
		 * @retval false if read failed, i.e. if @p address is outside EEPROM bounds
		 * @sa read(uint16_t, uint8_t&)
		 */
		inline static bool read(const uint8_t* address, uint8_t& value)
		{
			return read((uint16_t) address, value);
		}

		/**
		 * Read one byte stored in EEPROM at @p address.
		 * 
		 * @param address the location to be read in EEPROM; @p address is provided as an 
		 * absolute location, from `0` to the maximum EEPROM size
		 * @param value a reference to the variable that will be assigned the content read
		 * from EEPROM
		 * @retval true if read was successful
		 * @retval false if read failed, i.e. if @p address is outside EEPROM bounds
		 * @sa read(const uint8_t*, uint8_t&)
		 */
		inline static bool read(uint16_t address, uint8_t& value)
		{
			if (!check(address, 1))
			{
				value = 0xFF;
				return false;
			}
			blocked_read(address, value);
			return true;
		}

		/**
		 * Write the content of @p value of type @p T to the EEPROM at @p address.
		 * @note Whatever @p T type, this method never calls its assignment 
		 * operator but simply copies, byte per byte, content from local variable content
		 * to EEPROM, no @p T code gets executed.
		 * 
		 * @tparam T the type of content to be written (determines the number of bytes to write)
		 * @param address the location to be written in EEPROM; @p address is typically
		 * obtained as `&variable` where `variable` is a global variable that was
		 * declared with `EEMEM` attribute.
		 * @param value a reference to the variable which content will be copied
		 * to EEPROM
		 * @retval true if write was successful
		 * @retval false if write failed, i.e. if @p address is outside EEPROM bounds
		 * @sa write(uint16_t, const T&)
		 */
		template<typename T> inline static bool write(const T* address, const T& value)
		{
			return write((uint16_t) address, value);
		}

		/**
		 * Write the content of @p value of type @p T to the EEPROM at @p address.
		 * @note Whatever @p T type, this method never calls its assignment 
		 * operator but simply copies, byte per byte, content from local variable content
		 * to EEPROM, no @p T code gets executed.
		 * 
		 * @tparam T the type of content to be written (determines the number of bytes to write)
		 * @param address the location to be written in EEPROM; @p address is provided as an 
		 * absolute location, from `0` to the maximum EEPROM size
		 * @param value a reference to the variable which content will be copied
		 * to EEPROM
		 * @retval true if write was successful
		 * @retval false if write failed, i.e. if @p address is outside EEPROM bounds
		 * @sa write(uint16_t, const T&)
		 */
		template<typename T> static bool write(uint16_t address, const T& value)
		{
			if (!check(address, sizeof(T))) return false;
			uint8_t* v = (uint8_t*) &value;
			for (uint8_t i = 0; i < sizeof(T); ++i) blocked_write(address++, *v++);
			return true;
		}

		/**
		 * Write @p value, an array of @p count values of type @p T to the EEPROM at @p address.
		 * @note Whatever @p T type, this method never calls its assignment 
		 * operator but simply copies, byte per byte, content from local variable content
		 * to EEPROM, no @p T code gets executed.
		 * 
		 * @tparam T the type of content to be written (determines the number of bytes to write)
		 * @param address the location to be written in EEPROM; @p address is typically
		 * obtained as `variable` where `variable` is a global variable array that was
		 * declared with `EEMEM` attribute.
		 * @param value a pointer to the variable array which content will be copied
		 * to EEPROM
		 * @param count the number of items of type @p T to be written
		 * @retval true if write was successful
		 * @retval false if write failed, i.e. if @p address is outside EEPROM bounds
		 * @sa write(uint16_t, const T*, uint16_t)
		 */
		template<typename T> inline static bool write(const T* address, const T* value, uint16_t count)
		{
			return write((uint16_t) address, value, count);
		}

		/**
		 * Write @p value, an array of @p count values of type @p T to the EEPROM at @p address.
		 * @note Whatever @p T type, this method never calls its assignment 
		 * operator but simply copies, byte per byte, content from local variable content
		 * to EEPROM, no @p T code gets executed.
		 * 
		 * @tparam T the type of content to be written (determines the number of bytes to write)
		 * @param address the location to be written in EEPROM; @p address is provided as an 
		 * absolute location, from `0` to the maximum EEPROM size
		 * @param value a pointer to the variable array which content will be copied
		 * to EEPROM
		 * @param count the number of items of type @p T to be written
		 * @retval true if write was successful
		 * @retval false if write failed, i.e. if @p address is outside EEPROM bounds
		 * @sa write(const T*, const T*, uint16_t)
		 */
		template<typename T> static bool write(uint16_t address, const T* value, uint16_t count)
		{
			if (!check(address, count * sizeof(T))) return false;
			uint8_t* v = (uint8_t*) value;
			for (uint8_t i = 0; i < count * sizeof(T); ++i) blocked_write(address++, *v++);
			return true;
		}

		/**
		 * Write one byte to the EEPROM at @p address.
		 * 
		 * @param address the location to be written in EEPROM; @p address is typically
		 * obtained as `&variable` where `variable` is a global variable that was
		 * declared with `EEMEM` attribute.
		 * @param value the byte value that will be written to EEPROM
		 * @retval true if write was successful
		 * @retval false if write failed, i.e. if @p address is outside EEPROM bounds
		 * @sa write(uint16_t, uint8_t)
		 */
		inline static bool write(const uint8_t* address, uint8_t value)
		{
			return write((uint16_t) address, value);
		}

		/**
		 * Write one byte to the EEPROM at @p address.
		 * 
		 * @param address the location to be written in EEPROM; @p address is provided as an 
		 * absolute location, from `0` to the maximum EEPROM size
		 * @param value the byte value that will be written to EEPROM
		 * @retval true if write was successful
		 * @retval false if write failed, i.e. if @p address is outside EEPROM bounds
		 * @sa write(const uint8_t*, uint8_t)
		 */
		inline static bool write(uint16_t address, uint8_t value)
		{
			if (!check(address, 1)) return false;
			blocked_write(address, value);
			return true;
		}

		/**
		 * Erase the full EEPROM content.
		 * Note that "erasing" means setting all EEPROM cells to `0xFF`.
		 * The method will block until all EEPROM content has been erased.
		 */
		static void erase()
		{
			for (uint16_t address = 0; address < size(); ++address)
			{
				wait_until_ready();
				erase_address(address);
			}
		}

		/**
		 * Return the size (in bytes) of the embedded EEPROM.
		 */
		static constexpr uint16_t size()
		{
			return E2END + 1;
		}

		/**
		 * Block until the current EEPROM operation, whetever it is (e.g. read, write,
		 * erase), is complete.
		 * Normally, you would not need this method as any `EEPROM` method calls it 
		 * before starting its operation.
		 */
		inline static void wait_until_ready()
		{
			EECR_.loop_until_bit_clear(EEPE);
		}

	protected:
		/// @cond notdocumented
		inline static bool check(uint16_t address, uint16_t size)
		{
			return size && (address <= E2END) && (size <= (E2END + 1)) && ((address + size) <= (E2END + 1));
		}

		inline static void blocked_read(uint16_t address, uint8_t& value)
		{
			wait_until_ready();
			read_byte(address, value);
		}

		inline static void read_byte(uint16_t address, uint8_t& value)
		{
			EEAR_ = address;
			EECR_ = _BV(EERE);
			value = EEDR_;
		}

		inline static void blocked_write(uint16_t address, uint8_t value)
		{
			wait_until_ready();
			write_byte(address, value);
		}

		// In order to optimize write time we read current byte first, then compare it with new value
		// Then we choose between erase, write and erase+write based on comparison
		// This approach is detailed in ATmel note AVR103: Using the EEPROM Programming Modes
		// http://www.atmel.com/images/doc2578.pdf
		inline static void write_byte(uint16_t address, uint8_t value)
		{
			EEAR_ = address;
			EECR_ = _BV(EERE);
			uint8_t old_value = EEDR_;
			uint8_t diff = old_value ^ value;
			if (diff & value)
			{
				// Some bits need to be erased (ie set to 1)
				if (value == 0xFF)
					// Erase only
					EECR_ = EEPM0;
				else
					// Combine Erase/Write operation
					EECR_ = 0;
			}
			else
			{
				// No bit to be erased
				if (diff)
					// Some bits to be programmed (set to 0): Write only
					EECR_ = EEPM1;
				else
					// old value == new value => do nothing
					return;
			}
			EEDR_ = value;
			synchronized
			{
				EECR_ |= _BV(EEMPE);
				EECR_ |= _BV(EEPE);
			}
		}

		inline static bool erase_address(uint16_t address)
		{
			EEAR_ = address;
			EECR_ = _BV(EERE);
			uint8_t value = EEDR;
			// Some bits need to be erased (ie set to 1)
			if (value == 0xFF) return false;
			EECR_ = EEPM0;
			EEDR_ = 0xFF;
			synchronized
			{
				EECR_ |= _BV(EEMPE);
				EECR_ |= _BV(EEPE);
			}
			return true;
		}
		/// @endcond

	private:
		using REG8 = board_traits::REG8;
		using REG16 = board_traits::REG16;
		static constexpr const REG16 EEAR_{EEAR};
		static constexpr const REG8 EECR_{EECR};
		static constexpr const REG8 EEDR_{EEDR};
	};

	/**
	 * API that allows asynchronous writing to EEPROM; this can be useful when you
	 * have large amount of data to write but cannot afford to wait until all bytes
	 * have been written.
	 * The class uses a ring buffer which array must be provided at construction time.
	 * 
	 * In order for QueueWriter to function properly, you must register a proper ISR.
	 * FastArduino provides 3 possible ISR registrations:
	 * - REGISTER_EEPROM_ISR() basic ISR, ensures that all queued writes get written
	 * asynchronously
	 * - REGISTER_EEPROM_ISR_METHOD() enhanced ISR, first ensures that all queued
	 * writes get handled and when the last byte is written, calls a back a method
	 * of a handler class
	 * - REGISTER_EEPROM_ISR_FUNCTION() enhanced ISR, first ensures that all queued
	 * writes get handled and when the last byte is written, calls a back a function
	 * 
	 * Basically it has the same `write()` and `erase()` methods as `EEPROM` class,
	 * except:
	 * - its methods are not `static`
	 * - methods return immediately without waiting for the operation to be finished
	 * - methods may return `false` if the operation overflows the ring buffer
	 */
	class QueuedWriter : private EEPROM
	{
	public:
		/**
		 * Construct a QueuedWriter from a given @p buffer array.
		 * 
		 * @tparam SIZE the size of @p buffer; this size limits the amount of writes
		 * that can be queued and the content size of each write
		 * @param buffer the buffer that will be used by this QueuedWriter
		 */
		template<uint16_t SIZE>
		QueuedWriter(uint8_t (&buffer)[SIZE]) : buffer_{buffer}, current_{}, erase_{false}, done_{true}
		{
			interrupt::register_handler(*this);
		}

		/**
		 * Write the content of @p value of type @p T to the EEPROM at @p address.
		 * @note Whatever @p T type, this method never calls its assignment 
		 * operator but simply copies, byte per byte, content from local variable content
		 * to EEPROM, no @p T code gets executed.
		 * 
		 * @tparam T the type of content to be written (determines the number of bytes to write)
		 * @param address the location to be written in EEPROM; @p address is typically
		 * obtained as `&variable` where `variable` is a global variable that was
		 * declared with `EEMEM` attribute.
		 * @param value a reference to the variable which content will be copied
		 * to EEPROM
		 * @retval true if write was successful
		 * @retval false if write failed, i.e. if @p address is outside EEPROM bounds
		 * or if the ring buffer would overflow
		 * @sa write(uint16_t, const T&) 
		 */
		template<typename T> inline bool write(const T* address, const T& value)
		{
			return write((uint16_t) address, value);
		}

		/**
		 * Write the content of @p value of type @p T to the EEPROM at @p address.
		 * @note NOTE: whatever @p T type, this method never calls its assignment 
		 * operator but simply copies, byte per byte, content from local variable content
		 * to EEPROM, no @p T code gets executed.
		 * 
		 * @tparam T the type of content to be written (determines the number of bytes to write)
		 * @param address the location to be written in EEPROM; @p address is provided as an 
		 * absolute location, from `0` to the maximum EEPROM size
		 * @param value a reference to the variable which content will be copied
		 * to EEPROM
		 * @retval true if write was successful
		 * @retval false if write failed, i.e. if @p address is outside EEPROM bounds
		 * or if the ring buffer would overflow
		 * @sa write(const T*, const T&)
		 */
		template<typename T> bool write(uint16_t address, const T& value)
		{
			if (!check(address, sizeof(T))) return false;
			synchronized return write_data(address, (uint8_t*) &value, sizeof(T));
		}

		/**
		 * Write @p value, an array of @p count values of type @p T to the EEPROM at @p address.
		 * @note NOTE: whatever @p T type, this method never calls its assignment 
		 * operator but simply copies, byte per byte, content from local variable content
		 * to EEPROM, no @p T code gets executed.
		 * 
		 * @tparam T the type of content to be written (determines the number of bytes to write)
		 * @param address the location to be written in EEPROM; @p address is typically
		 * obtained as `variable` where `variable` is a global variable array that was
		 * declared with `EEMEM` attribute.
		 * @param value a pointer to the variable array which content will be copied
		 * to EEPROM
		 * @param count the number of items of type @p T to be written
		 * @retval true if write was successful
		 * @retval false if write failed, i.e. if @p address is outside EEPROM bounds
		 * or if the ring buffer would overflow
		 * @sa write(uint16_t, const T*, uint16_t) or if the ring buffer would overflow
		 */
		template<typename T> inline bool write(const T* address, const T* value, uint16_t count)
		{
			return write((uint16_t) address, value, count);
		}

		/**
		 * Write @p value, an array of @p count values of type @p T to the EEPROM at @p address.
		 * @note NOTE: whatever @p T type, this method never calls its assignment 
		 * operator but simply copies, byte per byte, content from local variable content
		 * to EEPROM, no @p T code gets executed.
		 * 
		 * @tparam T the type of content to be written (determines the number of bytes to write)
		 * @param address the location to be written in EEPROM; @p address is provided as an 
		 * absolute location, from `0` to the maximum EEPROM size
		 * @param value a pointer to the variable array which content will be copied
		 * to EEPROM
		 * @param count the number of items of type @p T to be written
		 * @retval true if write was successful
		 * @retval false if write failed, i.e. if @p address is outside EEPROM bounds
		 * or if the ring buffer would overflow
		 * @sa write(const T*, const T*, uint16_t)
		 */
		template<typename T> bool write(uint16_t address, const T* value, uint16_t count)
		{
			if (!check(address, count * sizeof(T))) return false;
			synchronized return write_data(address, (uint8_t*) value, count * sizeof(T));
		}

		/**
		 * Write one byte to the EEPROM at @p address.
		 * 
		 * @param address the location to be written in EEPROM; @p address is typically
		 * obtained as `&variable` where `variable` is a global variable that was
		 * declared with `EEMEM` attribute.
		 * @param value the byte value that will be written to EEPROM
		 * @retval true if write was successful
		 * @retval false if write failed, i.e. if @p address is outside EEPROM bounds
		 * or if the ring buffer would overflow
		 * @sa write(uint16_t, uint8_t)
		 */
		inline bool write(const uint8_t* address, uint8_t value)
		{
			return write((uint16_t) address, value);
		}

		/**
		 * Write one byte to the EEPROM at @p address.
		 * 
		 * @param address the location to be written in EEPROM; @p address is provided as an 
		 * absolute location, from `0` to the maximum EEPROM size
		 * @param value the byte value that will be written to EEPROM
		 * @retval true if write was successful
		 * @retval false if write failed, i.e. if @p address is outside EEPROM bounds
		 * or if the ring buffer would overflow
		 * @sa write(const uint8_t*, uint8_t)
		 */
		bool write(uint16_t address, uint8_t value)
		{
			if (!check(address, 1)) return false;
			synchronized return write_data(address, &value, 1);
		}

		/**
		 * Erase the full EEPROM content.
		 * Note that "erasing" means setting all EEPROM cells to `0xFF`.
		 * The method will first remove any pending writes from the ring buffer,
		 * wait for any currently on-going 1-byte write operation to completem,
		 * then it will start asynchronous erase operation.
		 */
		void erase()
		{
			// First remove all pending writes
			synchronized
			{
				buffer_.clear_();
				current_.size = 0;
			}
			// Wait until current byte write is finished
			wait_until_done();
			synchronized
			{
				// Start erase
				erase_ = true;
				done_ = false;
				current_.address = 0;
				current_.size = size();
				// Start transmission if not done yet
				EECR_ = _BV(EERIE);
			}
		}

		/**
		 * Block until all pending operations (queued in the ring buffer) are
		 * complete.
		 * This can be useful if your program is about to exit but you want to
		 * first ensure that any EEPROM write operations are finished before exit
		 * (otherwise, the EEPROM content may not be as expected).
		 */
		void wait_until_done() const
		{
			while (!done_)
				;
		}

		/**
		 * Tell if there is no queued, nor on-going write operation.
		 */
		bool is_done() const
		{
			return done_;
		}

	private:
		static const uint16_t ITEM_SIZE = 3;

		bool on_ready()
		{
			if (erase_)
			{
				if (current_.size)
					erase_next();
				else
				{
					// All erases are finished
					erase_ = false;
					// Mark all EEPROM work as finished if no write is pending in the queue
					if (buffer_.empty_())
					{
						done_ = true;
						EECR_ = 0;
					}
				}
			}
			else if (current_.size)
				// There is one item being currently written, write next byte
				write_next();
			else if (!buffer_.empty_())
			{
				// Current item is finished writing but there is another item to be written in the queue
				// Get new item and start transmission of first byte
				current_ = next_item();
				write_next();
			}
			else
			{
				// All writes are finished
				done_ = true;
				EECR_ = 0;
			}
			return done_;
		}
		
		void write_next()
		{
			uint8_t value;
			buffer_.pull_(value);
			EEPROM::write_byte(current_.address++, value);
			--current_.size;
			EECR_ |= _BV(EERIE);
		}

		void erase_next()
		{
			EEPROM::erase_address(current_.address++);
			--current_.size;
			EECR_ |= _BV(EERIE);
		}

		bool write_data(uint16_t address, uint8_t* value, uint16_t size)
		{
			// First check if there is enough space in buffer_ for this queued write
			if ((buffer_.free_() < size + ITEM_SIZE) || !size) return false;
			done_ = false;
			// Add new queued write to buffer
			buffer_.push_(WriteItem::value1(address, size));
			buffer_.push_(WriteItem::value2(address, size));
			buffer_.push_(WriteItem::value3(address, size));
			for (uint16_t i = 0; i < size; ++i) buffer_.push_(*value++);

			// Start transmission if not done yet
			EECR_ = _BV(EERIE);
			return true;
		}

		struct WriteItem
		{
			WriteItem() : address{0}, size{0}
			{
			}
			WriteItem(uint8_t value1, uint8_t value2, uint8_t value3)
				: address{uint16_t(value1 << 4 | value2 >> 4)}, size{uint16_t(utils::is_zero(
																	(value2 & 0x0F) << 8 | value3, E2END + 1))}
			{
			}
			inline static uint8_t value1(uint16_t address, uint16_t size UNUSED)
			{
				return address >> 4;
			}
			inline static uint8_t value2(uint16_t address, uint16_t size)
			{
				return address << 4 | size >> 8;
			}
			inline static uint8_t value3(uint16_t address UNUSED, uint16_t size)
			{
				return size;
			}

			uint16_t address;
			uint16_t size;
		};

		WriteItem next_item()
		{
			uint8_t value1, value2, value3;
			buffer_.pull_(value1);
			buffer_.pull_(value2);
			buffer_.pull_(value3);
			return WriteItem{value1, value2, value3};
		}

		using REG8 = board_traits::REG8;
		using REG16 = board_traits::REG16;
		static constexpr const REG16 EEAR_{EEAR};
		static constexpr const REG8 EECR_{EECR};
		static constexpr const REG8 EEDR_{EEDR};

		containers::Queue<uint8_t, uint8_t> buffer_;
		WriteItem current_;
		volatile bool erase_;
		volatile bool done_;

		friend void ::EE_READY_vect(void);
	};
}

#endif /* EEPROM_H */
/// @endcond
