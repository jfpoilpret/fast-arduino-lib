#ifndef QUEUE_HH
#define	QUEUE_HH

template<typename T>
class Queue
{
public:
	template<uint8_t SIZE>
	static Queue<T> create(T buffer[SIZE]);
	
	bool push(const T& item);
	bool pull(T& item);
	T pull();
	
	uint8_t available() const;
	uint8_t room() const;
	
private:
	Queue(T* buffer, uint8_t size): _buffer{buffer}, _size{size}, _head{0}, _tail{0} {}
	T* const _buffer;
	const uint8_t _size;
	uint8_t _head;
	uint8_t _tail;
};

#endif	/* QUEUE_HH */

