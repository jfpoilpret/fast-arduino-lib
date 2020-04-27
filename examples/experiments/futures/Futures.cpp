/*
 * This program is just for personal experiments here on AVR features and C++ stuff.
 * This is a Proof of Concept about Futures and Promises, to be used later by
 * async I2C API (and possibly other API too).
 * It just uses an Arduino UNO with the following connections:
 * - D2 (EXT0) connected to a push button, itself connected to GND
 * - D3 (EXT1) connected to a push button, itself connected to GND
 */


#include <string.h>
#include <fastarduino/errors.h>
#include <fastarduino/move.h>
#include <fastarduino/time.h>

#include <fastarduino/tests/assertions.h>
#include <fastarduino/uart.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/flash.h>
#include <fastarduino/int.h>
#include <fastarduino/gpio.h>

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
// - it is possible to subclass Futures to add last minute transformation on get()

// OPEN POINTS
//TODO - optimization/concurrence-safety of AbstractFuture/Future moves!
//TODO - invalid Futures should always get id_ set to 0!
//TODO - how to avoid reuse of inactive ids? (high risk of updating another Future!)

//TODO - limit friends as much as possible
//TODO - Future template specialization for void, for refs?
//TODO - Generalize concept to support ValueHolders (opposite of futures)

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

	uint8_t available_futures() const
	{
		synchronized return available_futures_();
	}

	uint8_t available_futures_() const
	{
		uint8_t free = 0;
		for (uint8_t i = 0; i < size_; ++i)
			if (futures_[i] == nullptr)
				++free;
		return free;
	}

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
	//TODO does this function work also with a T (useful when T is small and is a constant eg -1 as int)
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

	AbstractFuture* find_future(uint8_t id) const
	{
		if ((id == 0) || (id > size_))
			return nullptr;
		return futures_[id - 1];
	}
	bool update_future(uint8_t id, AbstractFuture* old_address, AbstractFuture* new_address)
	{
		synchronized return update_future_(id, old_address, new_address);
	}
	// Called by Future themselves (on construction, destruction, assignment)
	bool update_future_(uint8_t id, AbstractFuture* old_address, AbstractFuture* new_address)
	{
		// Check id is plausible and address matches
		if (find_future(id) != old_address)
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
			status_ = FutureStatus::INVALID;
			return error_;

			case FutureStatus::READY:
			return 0;

			case FutureStatus::INVALID:
			default:
			status_ = FutureStatus::INVALID;
			return errors::EINVAL;
		}
	}

protected:
	// Constructor used by FutureManager
	AbstractFuture(uint8_t* data, uint8_t size) : data_{data}, current_{data}, size_{size} {}
	~AbstractFuture()
	{
		// Notify FutureManager about destruction
		AbstractFutureManager::instance().update_future(id_, this, nullptr);
	}

	AbstractFuture(AbstractFuture&& that)
		:	id_{that.id_}, status_{that.status_}, error_{that.error_}, 
			data_{that.data_}, current_{that.current_}, size_{that.size_}
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
			data_ = that.data_;
			current_ = that.current_;
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

private:
	// The following methods are called by FutureManager to fill the Future value (or error)
	bool set_chunk_(uint8_t chunk)
	{
		// Check this future is waiting for data
		if (status_ != FutureStatus::NOT_READY)
			return false;
		// Update Future value chunk
		*current_++ = chunk;
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
		memcpy(current_, chunk, size);
		current_ += size;
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
	uint8_t* current_ = nullptr;
	uint8_t size_ = 0;

	friend class AbstractFutureManager;
	template<typename T> friend class Future;
};

bool AbstractFutureManager::set_future_value_(uint8_t id, uint8_t chunk) const
{
	AbstractFuture* future = find_future(id);
	if (future == nullptr)
		return false;
	return future->set_chunk_(chunk);
}
bool AbstractFutureManager::set_future_value_(uint8_t id, const uint8_t* chunk, uint8_t size) const
{
	AbstractFuture* future = find_future(id);
	if (future == nullptr)
		return false;
	return future->set_chunk_(chunk, size);
}
template<typename T> bool AbstractFutureManager::set_future_value_(uint8_t id, const T& value) const
{
	AbstractFuture* future = find_future(id);
	if (future == nullptr)
		return false;
	return future->set_chunk_(reinterpret_cast<const uint8_t*>(&value), sizeof(T));
}
bool AbstractFutureManager::set_future_error_(uint8_t id, int error) const
{
	AbstractFuture* future = find_future(id);
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

	Future(Future<T>&& that) : AbstractFuture{std::move(that)}
	{
		move(std::move(that));
	}
	//TODO maybe AbstractFuture::operator=() is not mandatory and can be fully removed?
	Future<T>& operator=(Future<T>&& that)
	{
		if (this == &that) return *this;
		(AbstractFuture&) *this = std::move(that);
		move(std::move(that));
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
		status_ = FutureStatus::INVALID;
		return true;
	}

private:
	//TODO better put this method in AbstractFuture?
	void move(Future<T>&& that)
	{
		synchronized
		{
			// Reset size_ if data is not being constructed currently
			if (status_ != FutureStatus::NOT_READY)
				size_ = sizeof(T);
			memcpy(buffer_, that.buffer_, sizeof(T));
			data_ = buffer_;
			current_ = buffer_ + (that.current_ - that.data_);
		}
	}

	union
	{
		T result_;
		uint8_t buffer_[sizeof(T)];
	};
};

template<typename T> bool AbstractFutureManager::register_future_(Future<T>& future)
{
	//FIXME we should refuse to re-register an already registered future, (except if INVALID?)!!!
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

using EXT0 = gpio::FastPinType<board::EXT_PIN<board::ExternalInterruptPin::D2_PD2_EXT0>()>;
using EXT1 = gpio::FastPinType<board::EXT_PIN<board::ExternalInterruptPin::D3_PD3_EXT1>()>;

struct ButtonValue
{
	ButtonValue(uint8_t button = 0, uint16_t count = 0) : button{button}, count{count} {}
	uint8_t button;
	uint16_t count;
};
static uint8_t future_id = 0;

void button_pushed()
{
	static uint16_t count0 = 0;
	static uint16_t count1 = 0;

	// if EXTx pushed, countx++, set value of future to (x, countx)
	if (!EXT0::value())
		AbstractFutureManager::instance().set_future_value_(future_id, ButtonValue{0, ++count0});
	else if (!EXT1::value())
		AbstractFutureManager::instance().set_future_value_(future_id, ButtonValue{1, ++count1});
}

// Register ISR for EXT0/1
REGISTER_INT_ISR_FUNCTION(0, board::ExternalInterruptPin::D2_PD2_EXT0, button_pushed)
REGISTER_INT_ISR_FUNCTION(1, board::ExternalInterruptPin::D3_PD3_EXT1, button_pushed)

// Future subclass for checking that it works too!
class MyFuture : public Future<uint16_t>
{
public:
	MyFuture() : Future<uint16_t>{} {}
	~MyFuture() = default;

	MyFuture(MyFuture&& that) : Future<uint16_t>{std::move(that)}
	{
	}
	MyFuture& operator=(MyFuture&& that)
	{
		if (this == &that) return *this;
		(Future<uint16_t>&) *this = std::move(that);
		return *this;
	}

	MyFuture(const MyFuture&) = delete;
	MyFuture& operator=(const MyFuture&) = delete;

	// The following method is blocking until this Future is ready
	bool get(uint16_t& result)
	{
		uint16_t temp = 0;
		if (this->Future<uint16_t>::get(temp))
		{
			result = temp * 10;
			return true;
		}
		return false;
	}
};

#define ASSERT_STATUS(OUT, STATUS, FUTURE) assert(OUT, F("" #FUTURE ".status()"), FutureStatus:: STATUS, FUTURE.status())
#define ASSERT_ERROR(OUT, ERROR, FUTURE) assert(OUT, F("" #FUTURE ".error()"), ERROR, FUTURE.error())
template<typename T1, typename T2> 
void assert_value(ostream& out, const flash::FlashStorage* name1, const flash::FlashStorage* name2, 
	Future<T1>& future, const T2& expected)
{
	T1 actual{};
	assert(out, name1, future.get(actual));
	assert(out, name2, expected, actual);
}
#define ASSERT_VALUE(OUT, VALUE, FUTURE) assert_value(OUT, F("" #FUTURE ".get()"), F("" #FUTURE ".get() value"), FUTURE, VALUE)

template<typename T>
void trace_future(ostream& out, const Future<T>& future)
{
	out << F("Future id = ") << dec << future.id() << F(", status = ") << future.status() << endl;
}

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
	assert(out, F("available futures"), MAX_FUTURES, manager.available_futures());
	out << F("Available futures = ") << dec << manager.available_futures() << endl;

	out << F("TEST #1 simple Future lifecycle: normal error case") << endl;
	// Check normal error context
	out << F("#1.1 instantiate future") << endl;
	Future<uint16_t> future1;
	ASSERT_STATUS(out, INVALID, future1);
	out << F("#1.2 register_future()") << endl;
	ASSERT(out, manager.register_future(future1));
	ASSERT_STATUS(out, NOT_READY, future1);
	assert(out, F("available futures"), MAX_FUTURES - 1, manager.available_futures());
	{
		out << F("#1.3 set_future_error()") << endl;
		ASSERT(out, manager.set_future_error(future1.id(), 0x1111));
		ASSERT_STATUS(out, ERROR, future1);
		ASSERT_ERROR(out, 0x1111, future1);
		ASSERT_STATUS(out, INVALID, future1);
	}
	out << endl;

	// Check full data set
	out << F("TEST #2 simple Future lifecycle: new Future and full value set") << endl;
	out << F("#2.1 instantiate future") << endl;
	Future<uint16_t> future2;
	ASSERT_STATUS(out, INVALID, future2);
	out << F("#2.2 register_future()") << endl;
	ASSERT(out, manager.register_future(future2));
	ASSERT_STATUS(out, NOT_READY, future2);
	{
		out << F("#2.3 set_future_value()") << endl;
		ASSERT(out, manager.set_future_value(future2.id(), 0x8000));
		ASSERT_STATUS(out, READY, future2);
		ASSERT_ERROR(out, 0, future2);
		ASSERT_STATUS(out, READY, future2);
		ASSERT_VALUE(out, 0x8000u, future2);
		ASSERT_STATUS(out, INVALID, future2);
		ASSERT_ERROR(out, errors::EINVAL, future2);
		ASSERT_STATUS(out, INVALID, future2);
	}
	out << endl;

	// Check set value by chunks
	out << F("TEST #3 simple Future lifecycle: new Future and partial value set") << endl;
	out << F("#3.1 instantiate future") << endl;
	Future<uint16_t> future3;
	ASSERT_STATUS(out, INVALID, future3);
	out << F("#3.2 register future") << endl;
	ASSERT(out, manager.register_future(future3));
	ASSERT_STATUS(out, NOT_READY, future3);
	{
		out << F("#3.3 set_future_value() chunk1") << endl;
		ASSERT(out, manager.set_future_value(future3.id(), uint8_t(0x11)));
		ASSERT_STATUS(out, NOT_READY, future3);
		out << F("#3.4 set_future_value() chunk2") << endl;
		ASSERT(out, manager.set_future_value(future3.id(), uint8_t(0x22)));
		ASSERT_STATUS(out, READY, future3);
		ASSERT_VALUE(out, 0x2211u, future3);
		ASSERT_STATUS(out, INVALID, future3);
	}
	out << endl;

	// Check set value by data pointer once
	out << F("TEST #4 simple Future lifecycle: new Future and full value pointer set") << endl;
	out << F("#4.1 instantiate future") << endl;
	Future<uint16_t> future4;
	ASSERT_STATUS(out, INVALID, future4);
	out << F("#4.2 register future") << endl;
	ASSERT(out, manager.register_future(future4));
	ASSERT_STATUS(out, NOT_READY, future4);
	{
		out << F("#4.3 set_future_value() from ptr") << endl;
		const uint16_t constant = 0x4433;
		ASSERT(out, manager.set_future_value(future4.id(), (const uint8_t*) &constant, sizeof(constant)));
		ASSERT_STATUS(out, READY, future4);
		ASSERT_VALUE(out, 0x4433u, future4);
		ASSERT_STATUS(out, INVALID, future4);
	}
	out << endl;

	// Check set value by data pointer twice
	out << F("TEST #5 simple Future lifecycle: new Future and part value pointer set") << endl;
	out << F("#5.1 instantiate future") << endl;
	Future<uint16_t> future5;
	ASSERT_STATUS(out, INVALID, future5);
	out << F("#5.2 register future") << endl;
	ASSERT(out, manager.register_future(future5));
	ASSERT_STATUS(out, NOT_READY, future5);
	{
		out << F("#5.3 set_future_value() from ptr (1 byte)") << endl;
		const uint16_t constant = 0x5566;
		ASSERT(out, manager.set_future_value(future5.id(), (const uint8_t*) &constant, 1));
		ASSERT_STATUS(out, NOT_READY, future5);
		out << F("#5.4 set_future_value() from ptr (2nd byte)") << endl;
		ASSERT(out, manager.set_future_value(future5.id(), ((const uint8_t*) &constant) + 1, 1));
		ASSERT_STATUS(out, READY, future5);
		ASSERT_VALUE(out, 0x5566u, future5);
		ASSERT_STATUS(out, INVALID, future5);
	}
	out << endl;

	// Check further updates do not do anything (and do not crash either!)
	//TODO also perform this check before get() is called READY -> READY or READY -> ERROR?
	out << F("TEST #6 simple Future lifecycle: check no more updates possible after first set complete") << endl;
	out << F("#6.1 instantiate future") << endl;
	Future<uint16_t> future6;
	ASSERT_STATUS(out, INVALID, future6);
	out << F("#6.2 register future") << endl;
	ASSERT(out, manager.register_future(future6));
	ASSERT_STATUS(out, NOT_READY, future6);
	{
		out << F("#6.3 set_future_value() from full value") << endl;
		ASSERT(out, manager.set_future_value(future6.id(), 0x8899));
		ASSERT_STATUS(out, READY, future6);
		out << F("#6.4 set_future_value() additional byte") << endl;
		ASSERT(out, !manager.set_future_value(future6.id(), uint8_t(0xAA)));
		ASSERT_STATUS(out, READY, future6);
		ASSERT_VALUE(out, 0x8899u, future6);
		ASSERT_STATUS(out, INVALID, future6);
		out << F("#6.5 set_future_value() after get() additional byte") << endl;
		ASSERT(out, !manager.set_future_value(future6.id(), uint8_t(0xBB)));
		ASSERT_STATUS(out, INVALID, future6);
	}
	out << endl;

	// Check reuse of a future in various states
	out << F("TEST #7 check Future status after move constructor") << endl;
	out << F("#7.1 instantiate future") << endl;
	Future<uint16_t> future7;
	ASSERT_STATUS(out, INVALID, future7);
	out << F("#7.2 register future") << endl;
	ASSERT(out, manager.register_future(future7));
	ASSERT_STATUS(out, NOT_READY, future7);
	{
		out << F("#7.3 check status (NOT_READY, INVALID) -> (INVALID, NOT_READY)") << endl;
		Future<uint16_t> future8 = std::move(future7);
		ASSERT_STATUS(out, INVALID, future7);
		ASSERT_STATUS(out, NOT_READY, future8);

		out << F("#7.4 check status (READY, INVALID) -> (INVALID, READY)") << endl;
		ASSERT(out, manager.set_future_value(future8.id(), 0xFFFFu));
		Future<uint16_t> future9 = std::move(future8);
		ASSERT_STATUS(out, INVALID, future8);
		ASSERT_STATUS(out, READY, future9);
		ASSERT_VALUE(out, 0xFFFFu, future9);

		out << F("#7.5 check status (ERROR, INVALID) -> (INVALID, ERROR)") << endl;
		Future<uint16_t> future10;
		ASSERT(out, manager.register_future(future10));
		ASSERT(out, manager.set_future_error(future10.id(), -10000));
		Future<uint16_t> future11 = std::move(future10);
		ASSERT_STATUS(out, INVALID, future10);
		ASSERT_STATUS(out, ERROR, future11);
		ASSERT_ERROR(out, -10000, future11);

		out << F("#7.6 check status (INVALID, INVALID) -> (INVALID, INVALID)") << endl;
		Future<uint16_t> future12;
		Future<uint16_t> future13 = std::move(future12);
		ASSERT_STATUS(out, INVALID, future12);
		ASSERT_STATUS(out, INVALID, future13);

		out << F("#7.7 check status (partial NOT_READY, INVALID) -> (INVALID, partial NOT_READY)") << endl;
		Future<uint16_t> future14;
		ASSERT(out, manager.register_future(future14));
		ASSERT(out, manager.set_future_value(future14.id(), uint8_t(0xBB)));
		Future<uint16_t> future15 = std::move(future14);
		ASSERT_STATUS(out, INVALID, future14);
		ASSERT_STATUS(out, NOT_READY, future15);
		ASSERT(out, manager.set_future_value(future15.id(), uint8_t(0xCC)));
		out << F("#7.8 Complete set value") << endl;
		ASSERT_STATUS(out, READY, future15);
		ASSERT_ERROR(out, 0, future15);
		ASSERT_VALUE(out, 0xCCBBu, future15);
	}
	out << endl;

	// Check reuse of a future in various states
	out << F("TEST #8 check Future status after move assignment") << endl;
	out << F("#8.1 instantiate futures") << endl;
	Future<uint16_t> future17, future18, future19, future20, future21, future22, future23, future24, future25;
	out << F("#8.2 register future") << endl;
	ASSERT(out, manager.register_future(future17));
	ASSERT_STATUS(out, NOT_READY, future17);
	{
		out << F("#8.3 check status (NOT_READY, INVALID) -> (INVALID, NOT_READY)") << endl;
		future18 = std::move(future17);
		ASSERT_STATUS(out, INVALID, future17);
		ASSERT_STATUS(out, NOT_READY, future18);

		out << F("#8.4 check status (READY, INVALID) -> (INVALID, READY)") << endl;
		ASSERT(out, manager.set_future_value(future18.id(), 0xFFFFu));
		future19 = std::move(future18);
		ASSERT_STATUS(out, INVALID, future18);
		ASSERT_STATUS(out, READY, future19);
		ASSERT_VALUE(out, 0xFFFFu, future19);

		out << F("#8.5 check status (ERROR, INVALID) -> (INVALID, ERROR)") << endl;
		ASSERT(out, manager.register_future(future20));
		ASSERT(out, manager.set_future_error(future20.id(), -10000));
		future21 = std::move(future20);
		ASSERT_STATUS(out, INVALID, future20);
		ASSERT_STATUS(out, ERROR, future21);
		ASSERT_ERROR(out, -10000, future21);

		out << F("#8.6 check status (INVALID, INVALID) -> (INVALID, INVALID)") << endl;
		future23 = std::move(future22);
		ASSERT_STATUS(out, INVALID, future22);
		ASSERT_STATUS(out, INVALID, future23);

		out << F("#8.7 check status (partial NOT_READY, INVALID) -> (INVALID, partial NOT_READY)") << endl;
		ASSERT(out, manager.register_future(future24));
		ASSERT(out, manager.set_future_value(future24.id(), uint8_t(0xBB)));
		future25 = std::move(future24);
		ASSERT_STATUS(out, INVALID, future24);
		ASSERT_STATUS(out, NOT_READY, future25);
		out << F("#8.8 after complete set value, status shall be READY") << endl;
		ASSERT(out, manager.set_future_value(future25.id(), uint8_t(0xCC)));
		ASSERT_STATUS(out, READY, future25);
		ASSERT_ERROR(out, 0, future25);
		ASSERT_VALUE(out, 0xCCBBu, future25);
	}
	out << endl;

	// Check Future subclassing
	out << F("TEST #9 Future subclassing...") << endl;
	out << F("#9.1 instantiate future") << endl;
	MyFuture my_future;
	ASSERT_STATUS(out, INVALID, my_future);
	out << F("#9.2 register_future()") << endl;
	ASSERT(out, manager.register_future(my_future));
	ASSERT_STATUS(out, NOT_READY, my_future);
	out << F("#9.3 set_future_value()") << endl;
	ASSERT(out, manager.set_future_value(my_future.id(), 123));
	ASSERT_STATUS(out, READY, my_future);
	out << F("#9.4 get()") << endl;
	uint16_t actual = 0;
	ASSERT(out, my_future.get(actual));
	assert(out, F("myfuture.get() value"), 1230u, actual);
	ASSERT_STATUS(out, INVALID, my_future);
	out << endl;

	time::delay_ms(1000);
	out << F("TEST #10 Future updated by ISR...") << endl;
	EXT0::set_mode(gpio::PinMode::INPUT_PULLUP);
	EXT1::set_mode(gpio::PinMode::INPUT_PULLUP);
	interrupt::INTSignal<board::ExternalInterruptPin::D2_PD2_EXT0> signal0{interrupt::InterruptTrigger::FALLING_EDGE};
	interrupt::INTSignal<board::ExternalInterruptPin::D3_PD3_EXT1> signal1{interrupt::InterruptTrigger::FALLING_EDGE};
	signal0.enable();
	signal1.enable();
	while (true)
	{
		Future<ButtonValue> future;
		manager.register_future(future);
		future_id = future.id();
		ButtonValue value;
		out << F("Press button 0 or 1 to see the future result") << endl;
		switch (future.await())
		{
			case FutureStatus::READY:
			future.get(value);
			out << F("Button EXT") << dec << value.button << F(", count = ") << value.count << endl;
			break;

			case FutureStatus::ERROR:
			out << F("Error ") << dec << future.error() << F(" received!") << endl;
			break;

			default:
			out << F("Unexpected status ") << future.status() << endl;
			break;
		}
	}

}
