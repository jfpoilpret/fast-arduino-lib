#ifndef QUEUE_HH
#define	QUEUE_HH

#include "utilities.hh"
#include "time.hh"

template<typename T>
class Queue
{
public:
	template<uint8_t SIZE>
	Queue(T buffer[SIZE]): _buffer{buffer}, _mask{(uint8_t)(SIZE - 1)}, _head{0}, _tail{0}
	{
		static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
	}
	
	//FIXME don't need this method, make ctor itself a template!
//	template<uint8_t SIZE>
//	static Queue<T> create(T buffer[SIZE]);
	
	bool push(const T& item);
	bool pull(T& item);
	
	uint8_t items() const __attribute__((always_inline))
	{
		ClearInterrupt clint;
		return (_tail - _head) & _mask;
	}
	uint8_t free() const __attribute__((always_inline))
	{
		ClearInterrupt clint;
		return (_head - _tail - 1) & _mask;
	}
	
private:
	Queue(T* buffer, uint8_t size): _buffer{buffer}, _mask{(uint8_t)(size - 1)}, _head{0}, _tail{0} {}
	T* const _buffer;
	const uint8_t _mask;
	volatile uint8_t _head;
	volatile uint8_t _tail;
};

//template<typename T>
//template<uint8_t SIZE>
//Queue<T> Queue<T>::create(T buffer[SIZE])
//{
//	static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
//	return Queue<T>(buffer, SIZE);
//}

template<typename T>
bool Queue<T>::push(const T& item)
{
	ClearInterrupt clint;
	if ((_head - _tail - 1) & _mask)
	{
		_buffer[_tail] = item;
		_tail = (_tail + 1) & _mask;
		return true;
	}
	return false;
}

template<typename T>
bool Queue<T>::pull(T& item)
{
	ClearInterrupt clint;
	if (_tail != _head)
	{
		item = _buffer[_head];
		_head = (_head + 1) & _mask;
		return true;
	}
	return false;
}

// Utility method that waits until a Queue has an item available
template<typename T>
T pull(Queue<T>& queue)
{
	T item;
	while (!queue.pull(item))
		Time::yield();
	return item;
}

#endif	/* QUEUE_HH */
