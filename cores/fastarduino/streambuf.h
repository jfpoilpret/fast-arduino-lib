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

/**
 * @file
 * C++-like std::iostream facilities.
 */
#ifndef STREAMBUF_H
#define STREAMBUF_H

#include "queue.h"
#include "flash.h"

namespace streams
{
	/**
	 * Output API based on a ring buffer.
	 * Provides general methods to push characters or strings to the buffer;
	 * the buffer is supposed to be consumed by another class (e.g. `serial::hard::UATX`).
	 * The API provides a protected "hook" (`virtual on_put()`) that get notified 
	 * every time new content is successfully pushed to the buffer, or when the 
	 * buffer is full while new content addition is attempted.
	 * 
	 * @param buffer the original ring buffer containing all pushed content; once
	 * passed to the constructor, it should never be used directly as it will be
	 * consumed by a `containers::Queue`.
	 */
	class ostreambuf : private containers::Queue<char, char>
	{
	private:
		using QUEUE = Queue<char, char>;

	public:
		ostreambuf(const ostreambuf&) = delete;
		ostreambuf& operator=(const ostreambuf&) = delete;
		
		template<uint8_t SIZE>
		explicit ostreambuf(char (&buffer)[SIZE]) : QUEUE{buffer, true} {}

		/**
		 * Wait until all buffer content has been pulled by a consumer.
		 * This method clear the count of overflows that have occurred until now.
		 */
		void pubsync()
		{
			overflow_ = false;
			while (!empty()) time::yield();
		}

		/**
		 * Append a character to the buffer.
		 * If the buffer is full, then `overflow()` flag will be set.
		 * @param c the character to append
		 * @sa overflow()
		 */
		void sputc(char c)
		{
			put_(c, true);
		}

		/**
		 * Append several characters to the buffer.
		 * If the buffer is full, then `overflow()` flag will be set.
		 * Once all characters have been appended, `on_put()` will be called,
		 * even if an overflow has occurred.
		 * 
		 * @param content the array of characters to be appended
		 * @param size the number of characters in @p content to append
		 * @sa on_put()
		 * @sa overflow()
		 */
		void sputn(const char* content, size_t size)
		{
			while (size--) put_(*content++, false);
			on_put();
		}

		/**
		 * Append a string to the buffer.
		 * The `'\0'` end character of @p str is not transmitted.
		 * If the buffer is full, then `overflow()` flag will be set.
		 * Once all string content has been appended, `on_put()` will be called,
		 * even if an overflow has occurred.
		 * 
		 * @param str the '\0' ended string (standard C-string) to append
		 * @sa on_put()
		 * @sa overflow()
		 */
		void sputn(const char* str)
		{
			while (*str) put_(*str++, false);
			on_put();
		}

		/**
		 * Append a string, stored on flash memory, to the buffer.
		 * The `'\0'` end character of @p str is not transmitted.
		 * If the buffer is full, then `overflow()` flag will be set.
		 * Once all string content has been appended, `on_put()` will be called,
		 * even if an overflow has occurred.
		 * Example:
		 * @code
		 * output.sputn(F("Hello, World!\n"));
		 * @endcode
		 * 
		 * @param str the '\0' ended string (standard C-string), stored on flash,
		 * to append
		 * @sa F()
		 * @sa on_put()
		 * @sa overflow()
		 */
		void sputn(const flash::FlashStorage* str)
		{
			uint16_t address = (uint16_t) str;
			while (char value = pgm_read_byte(address++)) put_(value, false);
			on_put();
		}

		/**
		 * Indicate if a buffer overflow has occurred since last time `pubsync()` or
		 * `reset_overflow()` was called. 
		 * @sa pubsync()
		 * @sa reset_overflow()
		 */
		bool overflow() const
		{
			return overflow_;
		}

		/**
		 * Return the underlying queue.
		 * Normally you will not need this method.
		 */
		QUEUE& queue()
		{
			return *this;
		}

	protected:
		/**
		 * Append a character to the buffer.
		 * If the buffer is full, then `overflow()` flag will be set.
		 * @param c the character to append
		 * @param call_on_put `true` if `on_put()` should be called after @p c has
		 * been appended, `false` otherwise; when directly calling this method,
		 * you should keep the default value.
		 * @sa on_put()
		 * @sa overflow()
		 */
		void put_(char c, bool call_on_put = true)
		{
			if (!push(c)) overflow_ = true;
			if (call_on_put) on_put();
		}

		/**
		 * Reset the overflow flag.
		 * @sa overflow()
		 */
		void reset_overflow()
		{
			overflow_ = false;
		}

		//TODO DOC
		virtual void on_put() {}

	private:
		bool overflow_ = false;

		friend class ios_base;
		friend class ostream;
	};

	/**
	 * Input API based on a ring buffer.
	 * Provides general methods to pull characters or strings from the buffer;
	 * the buffer content is supposed to be produced by another class (e.g. 
	 * `serial::hard::UARX`).
	 * 
	 * @param buffer the original ring buffer containing all pushed content; once
	 * passed to the constructor, it should never be used directly as it will be
	 * consumed by a `containers::Queue`.
	 */
	class istreambuf : private containers::Queue<char, char>
	{
	private:
		using QUEUE = Queue<char, char>;

	public:
		istreambuf(const istreambuf&) = delete;
		istreambuf& operator=(const istreambuf&) = delete;
		
		/**
		 * Special value returned by `sbumpc()` when buffer is empty.
		 */
		static const int EOF = -1;

		template<uint8_t SIZE> explicit istreambuf(char (&buffer)[SIZE]) : QUEUE{buffer, false} {}

		/**
		 * @return number of available characters in buffer
		 */
		int in_avail() const
		{
			return items();
		}

		/**
		 * @return next character to be read from buffer, or `EOF` if buffer is
		 * empty
		 */
		int sbumpc()
		{
			char value;
			if (pull(value)) return value;
			return EOF;
		}

		/**
		 * @return next character to be read from buffer (or `EOF` if buffer is
		 * empty) but does not remove it from the buffer.
		 */
		int sgetc()
		{
			char value;
			if (peek(value)) return value;
			return EOF;
		}

		/**
		 * Return the underlying queue.
		 * Normally you will not need this method.
		 */
		QUEUE& queue()
		{
			return *this;
		}
	};
}

#endif /* STREAMBUF_H */
/// @endcond
