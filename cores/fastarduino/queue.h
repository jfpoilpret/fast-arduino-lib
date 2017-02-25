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

#ifndef QUEUE_HH
#define	QUEUE_HH

#include "utilities.h"
#include "time.h"

//TODO imprvoe code size by creating a non-template base class with all common stuff
//TODO need to add some "peeking" API or iterator API, or some kind of deep-copy to another queue?
template<typename T, typename TREF = const T&>
class Queue
{
public:
	template<uint8_t SIZE>
	Queue(T (&buffer)[SIZE]): _buffer{buffer}, _mask{(uint8_t)(SIZE - 1)}, _head{0}, _tail{0}
	{
		static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
	}

	// Those methods are not interrupt-safe hence must be called only from an ISR
	bool _push(TREF item);
	bool _pull(T& item);
	bool _peek(T& item) const;
	uint8_t _peek(T* buffer, uint8_t size) const;
	template<uint8_t SIZE>
	uint8_t _peek(T (&buffer)[SIZE]) const;
	inline bool _empty() const
	{
		return (_tail == _head);
	}
	inline uint8_t _items() const
	{
		return (_tail - _head) & _mask;
	}
	inline uint8_t _free() const
	{
		return (_head - _tail - 1) & _mask;
	}
	inline void _clear()
	{
		_head = _tail = 0;
	}

	// Those methods are interrupt-safe hence can be called outside any ISR
	inline bool push(TREF item)
	{
		synchronized return _push(item);
	}
	inline bool pull(T& item)
	{
		synchronized return _pull(item);
	}
	inline bool peek(T& item) const
	{
		synchronized return _peek(item);
	}
	inline uint8_t peek(T* buffer, uint8_t size) const
	{
		synchronized return _peek(buffer, size);
	}
	template<uint8_t SIZE>
	inline uint8_t peek(T (&buffer)[SIZE]) const
	{
		synchronized return _peek(buffer);
	}
	inline bool empty() const
	{
		synchronized return _empty();
	}
	inline uint8_t items() const
	{
		synchronized return _items();
	}
	inline uint8_t free() const
	{
		synchronized return _free();
	}
	inline void clear()
	{
		synchronized _clear();
	}
	
private:
	T* const _buffer;
	const uint8_t _mask;
	volatile uint8_t _head;
	volatile uint8_t _tail;
};

template<typename T, typename TREF>
bool Queue<T, TREF>::_peek(T& item) const
{
	if (_tail == _head) return false;
	item = _buffer[_head];
	return true;
}

template<typename T, typename TREF>
uint8_t Queue<T, TREF>::_peek(T* buffer, uint8_t size) const
{
	uint8_t actual = (_tail - _head) & _mask;
	if (size > actual) size = actual;
	//TODO optimize copy (index calculation is simple if split in 2 parts)
	for (uint8_t i = 0; i < size; ++i)
		buffer[i] = _buffer[(_head + i) & _mask];
	return size;
}

template<typename T, typename TREF>
template<uint8_t SIZE>
uint8_t Queue<T, TREF>::_peek(T (&buffer)[SIZE]) const
{
	return _peek(&buffer, SIZE);
}

template<typename T, typename TREF>
bool Queue<T, TREF>::_push(TREF item)
{
	if ((_head - _tail - 1) & _mask)
	{
		_buffer[_tail] = item;
		_tail = (_tail + 1) & _mask;
		return true;
	}
	return false;
}

template<typename T, typename TREF>
bool Queue<T, TREF>::_pull(T& item)
{
	if (_tail != _head)
	{
		item = _buffer[_head];
		_head = (_head + 1) & _mask;
		return true;
	}
	return false;
}

// Utility method that waits until a Queue has an item available
template<typename T, typename TREF>
T pull(Queue<T, TREF>& queue)
{
	T item;
	while (!queue.pull(item))
		Time::yield();
	return item;
}

template<typename T, typename TREF>
T peek(Queue<T, TREF>& queue)
{
	T item;
	while (!queue.peek(item))
		Time::yield();
	return item;
}

#endif	/* QUEUE_HH */
