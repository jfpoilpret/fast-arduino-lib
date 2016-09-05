#ifndef ARRAY_HH
#define	ARRAY_HH

/*
// Circular array of fixed size
template<typename T>
class RingArray
{
public:
	template<uint8_t SIZE>
	RingArray(T (&buffer)[SIZE]): _buffer{buffer}, _mask{(uint8_t)(SIZE - 1)}, _head{0}, _tail{0}
	{
		static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
	}

	template<uint8_t SIZE>
	RingArray<T> clone(T (&buffer)[SIZE])
	{
		RingArray<T> copy{buffer};
		//TODO copy buffer content
		return copy;
	}

	bool insert(const T& item);
	bool remove(T& item);
	
	T& operator [] (uint8_t index);
	const T& operator [] (uint8_t index) const;
	
	uint8_t items() const INLINE
	{
		return (_tail - _head) & _mask;
	}
	uint8_t free() const INLINE
	{
		return (_head - _tail - 1) & _mask;
	}
	
protected:
	//TODO can we ultimately remove this factory API? (now that template ctor works...)
	RingArray(T* buffer, uint8_t size): _buffer{buffer}, _mask{(uint8_t)(size - 1)}, _head{0}, _tail{0} {}

private:
	T* const _buffer;
	const uint8_t _mask;
	uint8_t _head;
	uint8_t _tail;
};

template<typename T>
bool Queue<T>::peek(T& item) const
{
	ClearInterrupt clint;
	if (_tail == _head) return false;
	item = _buffer[_head];
	return true;
}

template<typename T>
uint8_t Queue<T>::peek(T* buffer, uint8_t size) const
{
	ClearInterrupt clint;
	uint8_t actual = (_tail - _head) & _mask;
	if (size > actual) size = actual;
	//TODO copy 
}

template<uint8_t SIZE>
template<typename T>
uint8_t Queue<T>::peek(T (&buffer)[SIZE]) const
{
	return peek(&buffer, SIZE);
}

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
*/

#endif	/* ARRAY_HH */
