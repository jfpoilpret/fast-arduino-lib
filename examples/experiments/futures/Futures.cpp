/*
 * This program is just for personal experiments here on AVR features and C++ stuff.
 * This is a Proof of Concept about Futures and Promises, to be used later by
 * async I2C API (and possibly other API too).
 * It just uses an Arduino UNO with the following connections:
 * - TODO
 */


// Define vectors we need in the example
#include <fastarduino/time.h>

// MAIN IDEA:
// - A Future holds a buffer for future value (any type)
// - Each Future is identified by a unique ID
// - A Future is either:
//		- Invalid: it is not linked to anything and is unusable; this happens
//		in several circumstances: default construction, instance move, value (or 
//		error) set and already read once
//		- Not ready: its value has not been obtained yet
//		- Ready: its value has been fully set and not yet read by anyone
//		- Error: an error occurred in the provider, hence no value will ever be 
//		held by this Future, the actual error has not yet been read by anyone
// - A FutureManager centralizes lifetime of all Futures
// - The FutureManager holds pointers to each valid Future
// - Number of Futures is statically defined at build time
// - Futures notify their lifetime to FM (moved, deleted, inactive)
// - Futures ID are used as an index into an internal FM table
// - Value providers must know the ID in order to fill up values (or errors) of
//	a Future, through FM (only FM knows exactly where each Future stands)

//TODO Everything!

// Forward declarations
class AbstractFuture;
template<typename T> class Future;

// Do we need to make it a singleton?
class AbstractFutureManager
{
public:
	template<uint8_t SIZE>
	AbstractFutureManager(AbstractFuture (&futures)[SIZE])
		: size_{SIZE}, futures_ {&futures}
	{
		for (uint8_t i = 0; i < SIZE; ++i)
			futures_[i] = 0;
	}

	template<typename T> bool new_future(Future<T>& future);
	// void status() const;
	// void status(uint8_t id) const;

private:
	// Called by Future themselves (on construction, destruction, assignment)
	void release_future(uint8_t id, AbstractFuture* address);
	void update_future(uint8_t id, AbstractFuture* address);

	// Called by future value providers
	// 2 first methods can set value by chunks
	void set_future_value(uint8_t id, uint8_t chunk) const;
	void set_future_value(uint8_t id, const uint8_t* chunk, uint8_t size) const;
	template<typename T> void set_future_value(uint8_t id, const T& value) const;

	const uint8_t size_;
	AbstractFuture* const futures_;

	//TODO friends declaration
	friend class AbstractFuture;
};

enum class FutureStatus : uint8_t
{
	INVALID = 0,
	NOT_READY,
	READY,
	ERROR
};

class AbstractFuture
{
public:
	FutureStatus status() const
	{
		return status_;
	}

	// The following methods are blocking until this Future is ready
	FutureStatus await() const
	{
		while (status_ == FutureStatus::NOT_READY)
			time::yield();
		return status_;
	}

protected:
	AbstractFuture() {}
	// Constructor used by FutureManager
	AbstractFuture(uint8_t id, uint8_t* data) {}
	~AbstractFuture()
	{
		//TODO notify FutureManager about destruction
	}

	AbstractFuture(AbstractFuture&& that)
	{
		//TODO notify FutureManager about move
		//TODO invalidate rhs
	}
	AbstractFuture& operator=(AbstractFuture&& that)
	{
		//TODO notify FutureManager about move
		//TODO invalidate rhs
	}

	AbstractFuture(const AbstractFuture&) = delete;
	AbstractFuture& operator=(const AbstractFuture&) = delete;

private:
	uint8_t id_ = 0;
	volatile FutureStatus status_ = FutureStatus::INVALID;
	uint8_t index_ = 0;
	uint8_t* data_ = nullptr;
};

// Future supports only types strictly smaller than 256 bytes
template<typename T>
class Future
{
	static_assert("T must be strictly smaller than 256 bytes", sizeof T <= UINT8_MAX);

public:
	Future() {}
	~Future()
	{
		//TODO notify FutureManager about destruction
	}

	Future(Future&& that)
	{
		//TODO notify FutureManager about move
		//TODO invalidate rhs
	}
	Future& operator=(Future&& that)
	{
		//TODO notify FutureManager about move
		//TODO invalidate rhs
	}

	Future(const Future&) = delete;
	Future& operator=(const Future&) = delete;

	FutureStatus status() const
	{
		return status_;
	}

	// The following methods are blocking until this Future is ready
	FutureStatus await() const
	{
		while (status_ == FutureStatus::NOT_READY)
			time::yield();
		return status_;
	}
	int error();
	T result();

private:
	uint8_t id_;
	volatile FutureStatus status_ = FutureStatus::NOT_READY;
	uint8_t index_ = 0;
	union
	{
		T result_;
		uint8_t buffer[sizeof T];
		int error_;
	};
};

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Enable interrupts at startup time
	sei();

	while (true)
	{
	}
}
