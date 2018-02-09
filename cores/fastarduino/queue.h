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
 * Utility API to handle ring-buffer queue containers.
 */
#ifndef QUEUE_HH
#define QUEUE_HH

#include "utilities.h"
#include "time.h"

namespace containers
{
	//TODO try to avoid SIZE contraint to be a power of 2, or at least give the choice
	// the developer
	//TODO imprvoe code size by creating a non-template base class with all common stuff
	//TODO need to add some "peeking" API or iterator API, or some kind of deep-copy to another queue?
	/**
	 * Queue of type @p T_ items.
	 * This is a FIFO (*first in first out*) queue, built upon a ring buffer of 
	 * fixed size, passed at construction time.
	 * 
	 * This queue offers only a few operations:
	 * - push an item at "the end" of the queue
	 * - pull an item from "the beginning" of the queue
	 * - clear the whole queue
	 * - get various information without changing the queue, including peeking
	 * one or several items from the beginning of the queue without removing them
	 * 
	 * All operations exist in two flavors:
	 * - **synchronized**: you should use this flavor whenever the caller cannot 
	 * guarantee no interruption will occur during the operation, i.e. when the caller
	 * is neither part of an ISR, nor embedded itself in a `synchronized` block.
	 * - **not synchronized**: you should use this flavor when the caller can guarantee
	 * that no interruption will occur during the operation, e.g. when called from
	 * an ISR or from within a `synchronized` block; these methods bear the same
	 * name as their **synchronized** counterparts, with an additional trailing
	 * **_** underscore.
	 * 
	 * @tparam T_ the type of items in this queue
	 * @tparam TREF_ the constant reference type of items in this queue; this is
	 * `const T_&` by default, but this may be changed, for small size types
	 * (one or two bytes long) to `T_` itself, in order to avoid additional code
	 * to handle reference extraction of an item. This type is used by `push()`
	 * methods.
	 */
	template<typename T_, typename TREF_ = const T_&> class Queue
	{
	public:
		/** The type of items in this queue. */
		using T = T_;

		/** The constant reference type of items in this queue. */
		using TREF = TREF_;

		/**
		 * Create a new queue, based on the provided @p buffer array.
		 * The queue size is determined by the size of `buffer`.
		 * @tparam SIZE the number of @p T items that `buffer` can hold; for
		 * performance optimization, this must be a power of 2; if not, compilation
		 * will fail.
		 * @param buffer the buffer used by this queue to store its items
		 */
		template<uint8_t SIZE>
		Queue(T (&buffer)[SIZE]) : buffer_{buffer}, mask_{(uint8_t)(SIZE - 1)}, head_{0}, tail_{0}
		{
			static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
		}

		/**
		 * Push @p item to the end of this queue, provided there is still available
		 * space in its ring buffer.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `push()` instead.
		 * 
		 * @param item a constant reference to the item to be pushed to thsi queue
		 * @retval true if @p item could be pushed
		 * @retval false if this queue is full and thus @p item could not be pushed
		 * 
		 * @sa push()
		 * @sa pull_()
		 * @sa free_()
		 */
		bool push_(TREF item);

		/**
		 * Pull an item from the beginning of this queue, if not empty, and copy
		 * it into @p item. The item is removed from the queue.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `pull()` instead.
		 * 
		 * @param item a reference to the item variable that will be assigned the
		 * first element of this queue
		 * @retval true if the queue is not empty and thus an item has been copied
		 * to @p item
		 * @retval false if this queue is empty and thus @p item has not changed
		 * 
		 * @sa pull()
		 * @sa push_()
		 * @sa empty_()
		 */
		bool pull_(T& item);

		/**
		 * Peek an item from the beginning of this queue, if not empty, and copy
		 * it into @p item. The queue is NOT modified, no item is removed from the queue.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `peek()` instead.
		 * 
		 * @param item a reference to the item variable that will be assigned the
		 * first element of this queue
		 * @retval true if the queue is not empty and thus an item has been copied
		 * to @p item
		 * @retval false if this queue is empty and thus @p item has not changed
		 * 
		 * @sa peek()
		 * @sa pull_()
		 * @sa push_()
		 * @sa empty_()
		 */
		bool peek_(T& item) const;

		/**
		 * Peek up to @p size items from the beginning of this queue, if not empty, 
		 * and copy these into @p buffer array. The queue is NOT modified, no item 
		 * is removed from the queue.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `peek()` instead.
		 * 
		 * @param buffer a pointer to an array of @p size items of type @p T,
		 * that will be assigned the first @p size elements of this queue
		 * @param size the maximum number of items to peek from this queue and
		 * copy to @p buffer; @p buffer size must be at least @p size
		 * @return the number of elements copied from the queue into @p buffer;
		 * this may be `0` if the queue is empty, or any number lower or equal to
		 * @p size; this will be @p size if the queue has at least @p size elements
		 * 
		 * @sa peek()
		 * @sa pull_()
		 * @sa push_()
		 * @sa items_()
		 */
		uint8_t peek_(T* buffer, uint8_t size) const;

		/**
		 * Peek up to @p SIZE items from the beginning of this queue, if not empty, 
		 * and copy these into @p buffer array. The queue is NOT modified, no item 
		 * is removed from the queue.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `peek()` instead.
		 * 
		 * @tparam SIZE the number of items that @p buffer can hold; this is also
		 * the maximum number of items to peek from this queue and
		 * copy to @p buffer
		 * @param buffer an array of @p SIZE items of type @p T,
		 * that will be assigned the first @p SIZE elements of this queue
		 * @return the number of elements copied from the queue into @p buffer;
		 * this may be `0` if the queue is empty, or any number lower or equal to
		 * @p SIZE; this will be @p SIZE if the queue has at least @p SIZE elements
		 * 
		 * @sa peek()
		 * @sa pull_()
		 * @sa push_()
		 * @sa items_()
		 */
		template<uint8_t SIZE> uint8_t peek_(T (&buffer)[SIZE]) const;

		/**
		 * Get the maximum size of this queue.
		 * This is the maximum number of items that can be present at the same time
		 * in this queue.
		 */
		inline uint8_t size() const
		{
			return mask_;
		}

		/**
		 * Tell if this queue is currently empty.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `empty()` instead.
		 * @sa empty()
		 * @sa free_()
		 */
		inline bool empty_() const
		{
			return (tail_ == head_);
		}

		/**
		 * Tell the current number of items currently present in this queue.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `items()` instead.
		 * @sa items()
		 * @sa size()
		 */
		inline uint8_t items_() const
		{
			return (tail_ - head_) & mask_;
		}

		/**
		 * Tell the current number of available locations for items to be pushed 
		 * to this queue.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `free()` instead.
		 * @sa free()
		 * @sa empty_()
		 */
		inline uint8_t free_() const
		{
			return (head_ - tail_ - 1) & mask_;
		}

		/**
		 * Completely clear this queue. All present items, if any, are lost.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `clear()` instead.
		 * @sa clear()
		 * @sa empty_()
		 */
		inline void clear_()
		{
			head_ = tail_ = 0;
		}

		/**
		 * Push @p item to the end of this queue, provided there is still available
		 * space in its ring buffer.
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `push_()` instead.
		 * 
		 * @param item a constant reference to the item to be pushed to thsi queue
		 * @retval true if @p item could be pushed
		 * @retval false if this queue is full and thus @p item could not be pushed
		 * 
		 * @sa push_()
		 * @sa pull()
		 * @sa free()
		 */
		inline bool push(TREF item)
		{
			synchronized return push_(item);
		}

		/**
		 * Pull an item from the beginning of this queue, if not empty, and copy
		 * it into @p item. The item is removed from the queue.
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `pull_()` instead.
		 * 
		 * @param item a reference to the item variable that will be assigned the
		 * first element of this queue
		 * @retval true if the queue is not empty and thus an item has been copied
		 * to @p item
		 * @retval false if this queue is empty and thus @p item has not changed
		 * 
		 * @sa pull_()
		 * @sa push()
		 * @sa empty()
		 */
		inline bool pull(T& item)
		{
			synchronized return pull_(item);
		}

		/**
		 * Peek an item from the beginning of this queue, if not empty, and copy
		 * it into @p item. The queue is NOT modified, no item is removed from the queue.
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `peek_()` instead.
		 * 
		 * @param item a reference to the item variable that will be assigned the
		 * first element of this queue
		 * @retval true if the queue is not empty and thus an item has been copied
		 * to @p item
		 * @retval false if this queue is empty and thus @p item has not changed
		 * 
		 * @sa peek_()
		 * @sa pull()
		 * @sa push()
		 * @sa empty()
		 */
		inline bool peek(T& item) const
		{
			synchronized return peek_(item);
		}

		/**
		 * Peek up to @p size items from the beginning of this queue, if not empty, 
		 * and copy these into @p buffer array. The queue is NOT modified, no item 
		 * is removed from the queue.
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `peek_()` instead.
		 * 
		 * @param buffer a pointer to an array of @p size items of type @p T,
		 * that will be assigned the first @p size elements of this queue
		 * @param size the maximum number of items to peek from this queue and
		 * copy to @p buffer; @p buffer size must be at least @p size
		 * @return the number of elements copied from the queue into @p buffer;
		 * this may be `0` if the queue is empty, or any number lower or equal to
		 * @p size; this will be @p size if the queue has at least @p size elements
		 * 
		 * @sa peek_()
		 * @sa pull()
		 * @sa push()
		 * @sa items()
		 */
		inline uint8_t peek(T* buffer, uint8_t size) const
		{
			synchronized return peek_(buffer, size);
		}

		/**
		 * Peek up to @p SIZE items from the beginning of this queue, if not empty, 
		 * and copy these into @p buffer array. The queue is NOT modified, no item 
		 * is removed from the queue.
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `peek_()` instead.
		 * 
		 * @tparam SIZE the number of items that @p buffer can hold; this is also
		 * the maximum number of items to peek from this queue and
		 * copy to @p buffer
		 * @param buffer an array of @p SIZE items of type @p T,
		 * that will be assigned the first @p SIZE elements of this queue
		 * @return the number of elements copied from the queue into @p buffer;
		 * this may be `0` if the queue is empty, or any number lower or equal to
		 * @p SIZE; this will be @p SIZE if the queue has at least @p SIZE elements
		 * 
		 * @sa peek_()
		 * @sa pull()
		 * @sa push()
		 * @sa items()
		 */
		template<uint8_t SIZE> inline uint8_t peek(T (&buffer)[SIZE]) const
		{
			synchronized return peek_(buffer);
		}

		/**
		 * Tell if this queue is currently empty.
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `empty_()` instead.
		 */
		inline bool empty() const
		{
			synchronized return empty_();
		}

		/**
		 * Tell the current number of items currently present in this queue.
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `items_()` instead.
		 * @sa items_()
		 */
		inline uint8_t items() const
		{
			synchronized return items_();
		}

		/**
		 * Tell the current number of available locations for items to be pushed 
		 * to this queue.
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `free_()` instead.
		 * @sa free_()
		 * @sa empty()
		 */
		inline uint8_t free() const
		{
			synchronized return free_();
		}

		/**
		 * Completely clear this queue. All present items, if any, are lost.
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `free()` instead.
		 * @sa clear_()
		 * @sa empty()
		 */
		inline void clear()
		{
			synchronized clear_();
		}

	private:
		T* const buffer_;
		const uint8_t mask_;
		volatile uint8_t head_;
		volatile uint8_t tail_;
	};

	template<typename T, typename TREF> bool Queue<T, TREF>::peek_(T& item) const
	{
		if (tail_ == head_) return false;
		item = buffer_[head_];
		return true;
	}

	template<typename T, typename TREF> uint8_t Queue<T, TREF>::peek_(T* buffer, uint8_t size) const
	{
		uint8_t actual = (tail_ - head_) & mask_;
		if (size > actual) size = actual;
		//TODO optimize copy (index calculation is simple if split in 2 parts)
		for (uint8_t i = 0; i < size; ++i) buffer[i] = buffer_[(head_ + i) & mask_];
		return size;
	}

	template<typename T, typename TREF> template<uint8_t SIZE> uint8_t Queue<T, TREF>::peek_(T (&buffer)[SIZE]) const
	{
		return peek_(&buffer, SIZE);
	}

	template<typename T, typename TREF> bool Queue<T, TREF>::push_(TREF item)
	{
		if ((head_ - tail_ - 1) & mask_)
		{
			buffer_[tail_] = item;
			tail_ = (tail_ + 1) & mask_;
			return true;
		}
		return false;
	}

	template<typename T, typename TREF> bool Queue<T, TREF>::pull_(T& item)
	{
		if (tail_ != head_)
		{
			item = buffer_[head_];
			head_ = (head_ + 1) & mask_;
			return true;
		}
		return false;
	}

	// Utility method that waits until a Queue has an item available
	template<typename T, typename TREF> T pull(Queue<T, TREF>& queue)
	{
		T item;
		while (!queue.pull(item)) time::yield();
		return item;
	}

	template<typename T, typename TREF> T peek(Queue<T, TREF>& queue)
	{
		T item;
		while (!queue.peek(item)) time::yield();
		return item;
	}
}

#endif /* QUEUE_HH */
/// @endcond
