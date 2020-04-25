/*
 * This program is just for personal experiments here on AVR features and C++ stuff.
 * This is a Proof of Concept about Futures and Promises, to be used later by
 * async I2C API (and possibly other API too).
 * It just uses an Arduino UNO with the following connections:
 * - TODO
 */


#include <string.h>
#include <fastarduino/errors.h>
#include <fastarduino/move.h>
#include <fastarduino/time.h>

#include <fastarduino/uart.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/flash.h>

// Register vector for UART (used for debug)
REGISTER_UATX_ISR(0)

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

// OPEN POINTS
// - how to handle errors inside FM
// - how to avoid reuse of inactive ids? (high risk of updating another Future!)
// - Future template specialization for void

// Forward declarations
class AbstractFuture;
template<typename T> class Future;

// Do we need to make it a singleton?
class AbstractFutureManager
{
public:
	static AbstractFutureManager& instance()
	{
		return *instance_;
	}

	// Called by a future value producer
	template<typename T> bool register_future(Future<T>& future)
	{
		synchronized return register_future_(future);
	}
	template<typename T> bool register_future_(Future<T>& future);

	// Called by future value providers
	// 2 first methods can set value by chunks
	bool set_future_value(uint8_t id, uint8_t chunk) const
	{
		synchronized return set_future_value_(id, chunk);
	}
	bool set_future_value(uint8_t id, const uint8_t* chunk, uint8_t size) const
	{
		synchronized return set_future_value_(id, chunk, size);
	}
	template<typename T> bool set_future_value(uint8_t id, const T& value) const
	{
		synchronized return set_future_value_(id, value);
	}
	bool set_future_error(uint8_t id, int error) const
	{
		synchronized return set_future_error_(id, error);
	}

	bool set_future_value_(uint8_t id, uint8_t chunk) const;
	bool set_future_value_(uint8_t id, const uint8_t* chunk, uint8_t size) const;
	template<typename T> bool set_future_value_(uint8_t id, const T& value) const;
	bool set_future_error_(uint8_t id, int error) const;

protected:
	explicit AbstractFutureManager(AbstractFuture** futures, uint8_t size)
		: size_{size}, futures_{futures}
	{
		for (uint8_t i = 0; i < size_; ++i)
			futures_[i] = nullptr;
		synchronized AbstractFutureManager::instance_ = this;
	}

	AbstractFutureManager(const AbstractFutureManager&) = delete;
	AbstractFutureManager& operator=(const AbstractFutureManager&) = delete;
	~AbstractFutureManager()
	{
		synchronized AbstractFutureManager::instance_ = nullptr;
	}

private:
	static AbstractFutureManager* instance_;

	bool update_future(uint8_t id, AbstractFuture* old_address, AbstractFuture* new_address)
	{
		synchronized return update_future_(id, old_address, new_address);
	}
	// Called by Future themselves (on construction, destruction, assignment)
	bool update_future_(uint8_t id, AbstractFuture* old_address, AbstractFuture* new_address)
	{
		// Check id is plausible and address matches
		if ((id == 0) || (id > size_) || (futures_[id - 1] != old_address))
			return false;
		futures_[id - 1] = new_address;
		return true;
	}

	const uint8_t size_;
	AbstractFuture** futures_;

	friend class AbstractFuture;
};

// Actual FutureManager: it just adds static storage to AbstractFutureManager
template<uint8_t SIZE>
class FutureManager : public AbstractFutureManager
{
public:
	FutureManager() : AbstractFutureManager{buffer_, SIZE}, buffer_{} {}

private:
	AbstractFuture* buffer_[SIZE];
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
	uint8_t id() const
	{
		return id_;
	}

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

	int error()
	{
		switch (await())
		{
			case FutureStatus::ERROR:
			set_invalid();
			return error_;

			case FutureStatus::READY:
			return 0;

			case FutureStatus::INVALID:
			default:
			set_invalid();
			return errors::EINVAL;
		}
	}

protected:
	// Constructor used by FutureManager
	AbstractFuture(uint8_t* data, uint8_t size) : data_{data}, size_{size} {}
	~AbstractFuture()
	{
		// Notify FutureManager about destruction
		AbstractFutureManager::instance().update_future(id_, this, nullptr);
	}

	//FIXME data_ is incorrect because it shall point to the next byte to write!
	AbstractFuture(AbstractFuture&& that)
		: id_{that.id_}, status_{that.status_}, error_{that.error_}, data_{that.data_}, size_{that.size_}
	{
		// Notify FutureManager about Future move
		if (!AbstractFutureManager::instance().update_future(id_, &that, this))
			status_ = FutureStatus::INVALID;
		// Make rhs Future invalid
		that.status_ = FutureStatus::INVALID;
	}
	AbstractFuture& operator=(AbstractFuture&& that)
	{
		if (this == &that) return *this;
		synchronized
		{
			// In case this Future is valid, it must be invalidated with FutureManager
			AbstractFutureManager::instance().update_future_(id_, this, nullptr);
			id_ = that.id_;
			status_ = that.status_;
			error_ = that.error_;
			//FIXME data_ is incorrect because it shall point to the next byte to write!
			data_ = that.data_;
			size_ = that.size_;
			// Notify FutureManager about Future move
			if (!AbstractFutureManager::instance().update_future_(id_, &that, this))
				status_ = FutureStatus::INVALID;
			// Make rhs Future invalid
			that.status_ = FutureStatus::INVALID;
		}
		return *this;
	}

	AbstractFuture(const AbstractFuture&) = delete;
	AbstractFuture& operator=(const AbstractFuture&) = delete;

	// Called by Future<T>::get() and Future<T>::error()
	void set_invalid()
	{
		synchronized
		{
			// Notify FutureManager to release this Future
			AbstractFutureManager::instance().update_future_(id_, this, nullptr);
			status_ = FutureStatus::INVALID;
		}
	}

private:
	// The following methods are called by FutureManager to fill the Future value (or error)
	bool set_chunk_(uint8_t chunk)
	{
		// Check this future is waiting for data
		if (status_ != FutureStatus::NOT_READY)
			return false;
		// Update Future value chunk
		*data_++ = chunk;
		// Is that the last chunk?
		if (--size_ == 0)
			status_ = FutureStatus::READY;
		return true;
	}
	bool set_chunk_(const uint8_t* chunk, uint8_t size)
	{
		// Check this future is waiting for data
		if (status_ != FutureStatus::NOT_READY)
			return false;
		// Check size does not go beyond expected size
		if (size > size_)
		{
			// Store error
			set_error_(errors::EMSGSIZE);
			return false;
		}
		memcpy(data_, chunk, size);
		// Is that the last chunk?
		size_ -= size;
		if (size_ == 0)
			status_ = FutureStatus::READY;
		return true;
	}
	bool set_error_(int error)
	{
		// Check this future is waiting for data
		if (error == 0 || status_ != FutureStatus::NOT_READY)
			return false;
		error_ = error;
		status_ = FutureStatus::ERROR;
		return true;
	}

	uint8_t id_ = 0;
	volatile FutureStatus status_ = FutureStatus::INVALID;
	int error_ = 0;
	uint8_t* data_ = nullptr;
	uint8_t size_ = 0;

	friend class AbstractFutureManager;
};

bool AbstractFutureManager::set_future_value_(uint8_t id, uint8_t chunk) const
{
	if ((id == 0) || (id > size_))
		return false;
	AbstractFuture* future = futures_[id - 1];
	if (future == nullptr)
		return false;
	return future->set_chunk_(chunk);
}
bool AbstractFutureManager::set_future_value_(uint8_t id, const uint8_t* chunk, uint8_t size) const
{
	if ((id == 0) || (id > size_))
		return false;
	AbstractFuture* future = futures_[id - 1];
	if (future == nullptr)
		return false;
	return future->set_chunk_(chunk, size);
}
template<typename T> bool AbstractFutureManager::set_future_value_(uint8_t id, const T& value) const
{
	if ((id == 0) || (id > size_))
		return false;
	AbstractFuture* future = futures_[id - 1];
	if (future == nullptr)
		return false;
	return future->set_chunk_(reinterpret_cast<const uint8_t*>(&value), sizeof(T));
}
bool AbstractFutureManager::set_future_error_(uint8_t id, int error) const
{
	if ((id == 0) || (id > size_))
		return false;
	AbstractFuture* future = futures_[id - 1];
	if (future == nullptr)
		return false;
	return future->set_error_(error);
}

// Future supports only types strictly smaller than 256 bytes
template<typename T>
class Future : public AbstractFuture
{
	static_assert(sizeof(T) <= UINT8_MAX, "T must be strictly smaller than 256 bytes");

public:
	Future() : AbstractFuture{buffer_, sizeof(T)} {}
	~Future() = default;

	Future(Future<T>&& that)
	{
		synchronized memcpy(buffer_, that.buffer_, sizeof(T));
	}
	Future<T>& operator=(Future<T>&& that)
	{
		if (this == &that) return *this;
		synchronized memcpy(buffer_, that.buffer_, sizeof(T));
		return *this;
	}

	Future(const Future<T>&) = delete;
	Future& operator=(const Future<T>&) = delete;

	// The following method is blocking until this Future is ready
	bool get(T& result)
	{
		if (await() != FutureStatus::READY)
			return false;
		result = result_;
		set_invalid();
		return true;
	}

private:
	union
	{
		T result_;
		uint8_t buffer_[sizeof(T)];
	};
};

template<typename T> bool AbstractFutureManager::register_future_(Future<T>& future)
{
	//TODO possible optimization if we maintain a count of free ids
	for (uint8_t i = 0; i < size_; ++i)
	{
		if (futures_[i] == nullptr)
		{
			update_future_(future.id_, &future, nullptr);
			future.id_ = static_cast<uint8_t>(i + 1);
			future.status_ = FutureStatus::NOT_READY;
			futures_[i] = &future;
			return true;
		}
	}
	return false;
}

// Static definitions (must be in cpp file)
//==========================================
AbstractFutureManager* AbstractFutureManager::instance_ = nullptr;


// Example starts here
//=====================

// Add utility ostream manipulator for FutureStatus
static const flash::FlashStorage* convert(FutureStatus s)
{
	switch (s)
	{
		case FutureStatus::INVALID:
		return F("INVALID");

		case FutureStatus::NOT_READY:
		return F("NOT_READY");

		case FutureStatus::READY:
		return F("READY");

		case FutureStatus::ERROR:
		return F("ERROR");
	}
}

streams::ostream& operator<<(streams::ostream& out, FutureStatus s)
{
	return out << convert(s);
}

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

static constexpr uint8_t MAX_FUTURES = 64;

using namespace streams;

template<typename T>
void trace_future(ostream& out, const Future<T>& future)
{
	out << F("Future id = ") << dec << future.id() << F(", status = ") << future.status() << endl;
}

//TODO Prepare tests from simplest to most complex:
// - without interrupt, check simple Future can be set with value, chunks, or error
// - check Future status on move and after resolved
int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Enable interrupts at startup time
	sei();

	// Initialize debugging output
	serial::hard::UATX<board::USART::USART0> uart{output_buffer};
	uart.begin(115200);
	ostream out = uart.out();
	out << boolalpha << showbase;

	out << F("Before FutureManager instantiation") << endl;
	FutureManager<MAX_FUTURES> manager{};

	out << F("TEST #1 simple Future lifecycle") << endl;
	out << F("#1.1 instantiate future") << endl;
	Future<uint16_t> future1;
	trace_future(out, future1);
	out << F("#1.2 register_future()") << endl;
	bool ok = manager.register_future(future1);
	out << F("result => ") << ok << endl;
	trace_future(out, future1);
	if (ok)
	{
		out << F("#1.3 set_future_error()") << endl;
		ok = manager.set_future_error(future1.id(), 0x1111);
		out << F("result => ") << ok << endl;
		trace_future(out, future1);
		int error = future1.error();
		out << F("error() = ") << hex << error << endl;
		trace_future(out, future1);
	}

	out << F("#1.4 reuse future") << endl;
	ok = manager.register_future(future1);
	out << F("result => ") << ok << endl;
	trace_future(out, future1);
	if (ok)
	{
		out << F("#1.5 set_future_value()") << endl;
		ok = manager.set_future_value(future1.id(), 0x8000);
		out << F("result => ") << ok << endl;
		trace_future(out, future1);
		int error = future1.error();
		out << F("error() = ") << dec << error << endl;
		trace_future(out, future1);
		uint16_t value = 0;
		ok = future1.get(value);
		out << F("get() = ") << ok << F(", value = ") << hex << value << endl;
		trace_future(out, future1);
		error = future1.error();
		out << F("error() = ") << dec << error << endl;
		trace_future(out, future1);
	}

	//TODO check set value by chunks


	//TODO check further updates do not do anything (and do not crash either!)
	//TODO check status across lifecyle

	// time::delay_ms(1000);
	// out << F("TEST #2 Future construction/destruction/move...") << endl;
	// //TODO

	// time::delay_ms(1000);
	// out << F("TEST #3 Future updated by ISR...") << endl;
	//TODO

}
