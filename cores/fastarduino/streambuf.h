//   Copyright 2016-2017 Jean-Francois Poilpret
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
	 * The API provides protected "hooks" that get notified every time new content
	 * is successfully pushed to the buffer, or when the buffer is full while new
	 * content addition is attempted.
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
		template<uint8_t SIZE> ostreambuf(char (&buffer)[SIZE]) : QUEUE{buffer}, overflow_{false}
		{
		}

		/**
		 * Wait until all buffer content has been pulled by a consumer.
		 * This method clear the count of overflows that have occurred until now.
		 */
		void pubsync()
		{
			overflow_ = false;
			while (items()) time::yield();
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
		 * Return the underlying queue.
		 * Normally you will not need this method.
		 */
		QUEUE& queue()
		{
			return *this;
		}
		
	protected:
		/**
		 * Callback method called when new content has been added to the buffer.
		 * This can be overridden by a subclass to trigger interrupt-driven
		 * transmission of buffer data.
		 * Default implementation does nothing.
		 */
		virtual void on_put()
		{
		}

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
		 * Indicate if a buffer overflow has occurred since last time `pubsync()` or
		 * `reset_overflow()` was called. 
		 * @sa pubsync()
		 * @sa reset_overflow()
		 */
		inline bool overflow() const
		{
			return overflow_;
		}

		/**
		 * Reset the overflow flag.
		 * @sa overflow()
		 */
		inline void reset_overflow()
		{
			overflow_ = false;
		}

	private:
		bool overflow_;

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
		/**
		 * Special value returned by `sbumpc()` when buffer is empty.
		 */
		static const int EOF = -1;

		template<uint8_t SIZE> istreambuf(char (&buffer)[SIZE]) : QUEUE{buffer}
		{
		}

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

		//TODO move this method to istream
		/**
		 * Read one word from buffer, blocking until a space is read or @p max
		 * characters have been read already.
		 * @param str the character array that will receive the next word
		 * @param max the maximum number of characters that can be stored in 
		 * @p str including the ending `'\0'`
		 * @return the next word read from buffer
		 */
		char* scan(char* str, size_t max);

		/**
		 * Return the underlying queue.
		 * Normally you will not need this method.
		 */
		QUEUE& queue()
		{
			return *this;
		}
	};

	//TODO These methods should not exist or operate on istream directly
	/**
	 * Wait for @p in to have at least one character in buffer and get it.
	 * @param in the `istreambuf` to read one character from
	 * @return next character to be read from @p in
	 */
	char get(istreambuf& in);

	/**
	 * Wait for @p in to have at least @p size characters in buffer and get them.
	 * @param in the `istreambuf` to read characters from
	 * @param content the character array that will receive all characters read 
	 * from @p in
	 * @param size the number of characters to read from @p in
	 * @return @p content
	 */
	char* get(istreambuf& in, char* content, size_t size);

	/**
	 * Wait for @p in to have either at least @p size characters in buffer,
	 * or to reach character value @p end, then copy read string into @p str.
	 * @param in the `istreambuf` to read characters from
	 * @param str the character array that will receive all characters read 
	 * from @p in
	 * @param max the maximum number to read from @p in
	 * @param end the character marking the end of string to read
	 * @return the number of characters read from @p in and copied into @p str
	 */
	int gets(istreambuf& in, char* str, size_t max, char end = 0);
}

#endif /* STREAMBUF_H */
/// @endcond
