//   Copyright 2016-2023 Jean-Francois Poilpret
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

#include "flash.h"
#include "interrupts.h"
#include "queue.h"

/**
 * Register the necessary callbacks that will be notified when a `streams::ostreambuf`
 * is put new content (character or string). This is used by hardware and software
 * UATX and UART.
 * 
 * Each handler registered here will be notified until one mentions it has handled
 * the notification.
 * 
 * @warning this macro must be called only once, with all interested handlers classes;
 * calling it more than once will lead to errors at link time.
 * @note you do not need to call this macro if you do not use streams::ostreambuf
 * in your program.
 * 
 * @param HANDLER1 a class which registered instance will be notified, through its
 * `bool on_put(streams::ostreambuf&)` method when any ostreambuffer has new content
 * put into it.
 * @param ... other classes similar to HANDLER1.
 * 
 * @sa REGISTER_OSTREAMBUF_NO_LISTENERS()
 * @sa interrupt::register_handler
 */
#define REGISTER_OSTREAMBUF_LISTENERS(HANDLER1, ...)								\
	void streams::ostreambuf_on_put_dispatch(ostreambuf& obuf)						\
	{																				\
		streams::dispatch_handler::ostreambuf_on_put<HANDLER1, ##__VA_ARGS__>(obuf);\
	}

/**
 * Register no callback at all to `streams::ostreambuf`.
 * You normally do not need this macro, except if you:
 * - use streams::ostreambuf
 * - but you do not use UATX, or UART
 * - you do not need to be called back when content is put to your ostreambuf instances
 * 
 * @sa REGISTER_OSTREAMBUF_LISTENERS()
 */
#define REGISTER_OSTREAMBUF_NO_LISTENERS()							\
	void streams::ostreambuf_on_put_dispatch(ostreambuf&) {}

/**
 * This macro shall be used in a class containing a private callback method
 * `bool on_put(streams::ostreambuf&)`, registered by `REGISTER_OSTREAMBUF_LISTENERS`.
 * It declares the class where it is used as a friend of all necessary functions
 * so that the private callback method can be called properly.
 */
#define DECL_OSTREAMBUF_LISTENERS_FRIEND         \
	friend struct streams::dispatch_handler;

namespace streams
{
	/// @cond notdocumented
	class ostreambuf;
	extern void ostreambuf_on_put_dispatch(ostreambuf&);
	/// @endcond 

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

		/// @cond notdocumented
		template<uint8_t SIZE>
		explicit ostreambuf(char (&buffer)[SIZE]) : QUEUE{buffer, true} {}
		/// @endcond

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

	private:
		void on_put()
		{
			ostreambuf_on_put_dispatch(*this);
		}

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

		/// @cond notdocumented
		template<uint8_t SIZE> explicit istreambuf(char (&buffer)[SIZE]) : QUEUE{buffer, false} {}
		/// @endcond

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

	/// @cond notdocumented
	struct dispatch_handler
	{
		template<bool DUMMY_> static bool ostreambuf_on_put_helper(ostreambuf& obuf UNUSED)
		{
			return false;
		}

		template<bool DUMMY_, typename HANDLER1_, typename... HANDLERS_> 
		static bool ostreambuf_on_put_helper(ostreambuf& obuf)
		{
			bool result = interrupt::HandlerHolder<HANDLER1_>::handler()->on_put(obuf);
			// handle other handlers if needed
			return result || ostreambuf_on_put_helper<DUMMY_, HANDLERS_...>(obuf);
		}

		template<typename... HANDLERS_> static void ostreambuf_on_put(ostreambuf& obuf)
		{
			// Ask each registered listener tohandle obuf on_put() if concerned
			ostreambuf_on_put_helper<false, HANDLERS_...>(obuf);
		}
	};
	/// @endcond 
}

#endif /* STREAMBUF_H */
/// @endcond
