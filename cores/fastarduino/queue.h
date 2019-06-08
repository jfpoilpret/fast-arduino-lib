//   Copyright 2016-2019 Jean-Francois Poilpret
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

#include <math.h>
#include "utilities.h"
#include "time.h"

namespace containers
{
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
		 * @tparam SIZE the number of @p T items that `buffer` can hold; note that,
		 * for optimization reasons, only `SIZE - 1` items can be held in the buffer.
		 * @param buffer the buffer used by this queue to store its items
		 */
		template<uint8_t SIZE> explicit Queue(T (&buffer)[SIZE]) : buffer_{buffer}, size_{SIZE}, head_{0}, tail_{0} {}

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
		uint8_t size() const
		{
			return size_ - 1;
		}

		/**
		 * Tell if this queue is currently empty.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `empty()` instead.
		 * @sa empty()
		 * @sa free_()
		 * @sa full_()
		 */
		bool empty_() const
		{
			return tail_ == head_;
		}

		/**
		 * Tell if this queue is currently full.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `full()` instead.
		 * @sa full()
		 * @sa free_()
		 */
		bool full_() const
		{
			return (tail_ + 1) == (head_ ? head_ : size_);
		}

		/**
		 * Tell the current number of items currently present in this queue.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `items()` instead.
		 * @sa items()
		 * @sa size()
		 */
		uint8_t items_() const
		{
			// - must be 0 when head_ = tail_
			// - must be size_ - 1 max, when tail_ = head_ - 1
			return tail_ - head_ + (tail_ >= head_ ? 0 : size_);
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
		uint8_t free_() const
		{
			// - must be 0 when tail_ = head_ - 1
			// - must be size_ - 1, when head_ == tail_
			// We simply return size_ - 1 - items_() but we optimize it a bit
			return head_ - tail_ - 1 + (tail_ >= head_ ? size_ : 0);
		}

		/**
		 * Completely clear this queue. All present items, if any, are lost.
		 * This method is not synchronized, hence you must ensure it is called from
		 * an interrupt-safe context; otherwise, you should use the synchronized flavor
		 * `clear()` instead.
		 * @sa clear()
		 * @sa empty_()
		 */
		void clear_()
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
		bool push(TREF item)
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
		bool pull(T& item)
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
		bool peek(T& item) const
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
		uint8_t peek(T* buffer, uint8_t size) const
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
		template<uint8_t SIZE> uint8_t peek(T (&buffer)[SIZE]) const
		{
			synchronized return peek_(buffer);
		}

		/**
		 * Tell if this queue is currently empty.
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `empty_()` instead.
		 */
		bool empty() const
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
		uint8_t items() const
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
		uint8_t free() const
		{
			synchronized return free_();
		}

		/**
		 * Tell if this queue is currently full.
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `full_()` instead.
		 * @sa full_()
		 * @sa free()
		 */
		bool full() const
		{
			synchronized return full_();
		}

		/**
		 * Completely clear this queue. All present items, if any, are lost.
		 * This method is synchronized, hence you can call it from an
		 * an interrupt-unsafe context; if you are sure you are in an interrupt-safe,
		 * you should use the not synchronized flavor `free()` instead.
		 * @sa clear_()
		 * @sa empty()
		 */
		void clear()
		{
			synchronized clear_();
		}

	private:
		T* const buffer_;
		const uint8_t size_;
		volatile uint8_t head_;
		volatile uint8_t tail_;
	};

	/// @cond notdocumented
	template<typename T, typename TREF> bool Queue<T, TREF>::peek_(T& item) const
	{
		if (tail_ == head_) return false;
		item = buffer_[head_];
		return true;
	}

	template<typename T, typename TREF> uint8_t Queue<T, TREF>::peek_(T* buffer, uint8_t size) const
	{
		size = min(size, items());
		if (size)
		{
			// Split peek in 2 parts if needed
			if (head_ < tail_)
			{
				const T* source = &buffer_[head_];
				for (uint8_t i = 0; i < size; ++i) *buffer++ = *source++;
			}
			else
			{
				uint8_t part_size = size_ - head_;
				const T* source = &buffer_[head_];
				for (uint8_t i = 0; i < part_size; ++i) *buffer++ = *source++;
				part_size = size - part_size;
				source = buffer_;
				for (uint8_t i = 0; i < part_size; ++i) *buffer++ = *source++;
			}
		}
		return size;
	}

	template<typename T, typename TREF> template<uint8_t SIZE> uint8_t Queue<T, TREF>::peek_(T (&buffer)[SIZE]) const
	{
		return peek_(&buffer, SIZE);
	}

	template<typename T, typename TREF> bool Queue<T, TREF>::push_(TREF item)
	{
		if (full_()) return false;
		buffer_[tail_] = item;
		++tail_;
		if (tail_ == size_) tail_ = 0;
		return true;
	}

	template<typename T, typename TREF> bool Queue<T, TREF>::pull_(T& item)
	{
		if (empty_()) return false;
		item = buffer_[head_];
		++head_;
		if (head_ == size_) head_ = 0;
		return true;
	}
	/// @endcond

	/**
	 * Pull an item from the beginning of @p queue. 
	 * The item is removed from the queue.
	 * This method wait until one item is available in @p queue.
	 * 
	 * @return the first item pulled from @p queue
	 * 
	 * @sa Queue::pull()
	 * @sa time::yield()
	 */
	template<typename T, typename TREF> T pull(Queue<T, TREF>& queue)
	{
		T item;
		while (!queue.pull(item)) time::yield();
		return item;
	}

	/**
	 * Peek an item from the beginning of @p queue.
	 * The queue is NOT modified, no item is removed from the queue.
	 * This method wait until one item is available in @p queue.
	 * 
	 * @return the first item peeked from @p queue
	 * 
	 * @sa Queue::peek()
	 * @sa time::yield()
	 */
	template<typename T, typename TREF> T peek(Queue<T, TREF>& queue)
	{
		T item;
		while (!queue.peek(item)) time::yield();
		return item;
	}
}

#endif /* QUEUE_HH */
/// @endcond
