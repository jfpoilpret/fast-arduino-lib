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

// includes for the example program itself
#include <fastarduino/tests/assertions.h>
#include <fastarduino/uart.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/flash.h>
#include <fastarduino/int.h>
#include <fastarduino/gpio.h>

// Register vector for UART (used for debug)
REGISTER_UATX_ISR(0)

// MAIN CONCEPT:
// - A Future holds a buffer for future value (any type, even void, i.e. no value)
// - A Future may also hold a storage value (constant, any type) with same lifetime as the Future
// - Each Future is identified by a unique ID
// - A Future is either:
//		- Invalid: it is not linked to anything and is unusable; this happens
//		in several circumstances: default construction, instance move, value (or 
//		error) set and already read once
//		- Not ready: its value has not been obtained yet
//		- Ready: its value has been fully set and not yet read by anyone
//		- Error: an error occurred in the provider, hence no value will ever be 
//		held by this Future, the actual error has not yet been read by anyone
// - Once invalid, a Future becomes useless, unless re-assigned with newly 
//	constructed Future
// - A FutureManager centralizes lifetime of all Futures
// - The FutureManager holds pointers to each valid Future
// - Maximum number of Futures is statically defined at build time
// - Futures notify their lifetime to FM (moved, deleted, inactive)
// - Futures ID are used as an index into an internal FM table, 0 means "not registered"
// - Value providers must know the ID in order to fill up values (or errors) of
//	a Future, through FM (only FM knows exactly where each Future stands)
// - Storage value consumers must know the ID in order to get storage value of
//	a Future, through FM (only FM knows exactly where each Future stands)
// - it is possible to subclass a Future to add last minute transformation on get()
// - FutureManager tries to limit potential conflicts when assigning an ID during
// Future registration, by searching for an available ID AFTER the last ID removed;
// This may not be sufficient: it is possible (although a well-written program should 
// never do that) that a NOT_READY Future gets destructed and its value provider tries to 
// fill its value, since the provider only gets the ID, if the same ID has been assigned 
// to a new Future, a conflict may occur and possibly lead to a crash.

// Forward declarations
class AbstractFuture;
template<typename OUT, typename IN> class Future;

class AbstractFutureManager
{
public:
	static AbstractFutureManager& instance()
	{
		return *instance_;
	}

	// Called by a future value producer
	template<typename OUT, typename IN> bool register_future(Future<OUT, IN>& future)
	{
		synchronized return register_future_(future);
	}
	template<typename OUT, typename IN> bool register_future_(Future<OUT, IN>& future);

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

	// Called by Future output providers and input consumers
	// This method is useful to make READY a Future with void output
	bool set_future_finish(uint8_t id) const
	{
		synchronized return set_future_finish_(id);
	}
	// Following methods can set value by chunks or as whole
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
	// Mark Future in ERROR
	bool set_future_error(uint8_t id, int error) const
	{
		synchronized return set_future_error_(id, error);
	}

	// Following methods read a Future storage value by chunks
	bool get_storage_value(uint8_t id, uint8_t& chunk) const
	{
		synchronized return get_storage_value_(id, chunk);
	}
	bool get_storage_value(uint8_t id, uint8_t* chunk, uint8_t size) const
	{
		synchronized return get_storage_value_(id, chunk, size);
	}

	// Same methods as above but not synchronized (called from an ISR exclusively)
	bool set_future_finish_(uint8_t id) const;
	bool set_future_value_(uint8_t id, uint8_t chunk) const;
	bool set_future_value_(uint8_t id, const uint8_t* chunk, uint8_t size) const;
	template<typename T> bool set_future_value_(uint8_t id, const T& value) const;
	bool set_future_error_(uint8_t id, int error) const;

	bool get_storage_value_(uint8_t id, uint8_t& chunk) const;
	bool get_storage_value_(uint8_t id, uint8_t* chunk, uint8_t size) const;

protected:
	AbstractFutureManager(AbstractFuture** futures, uint8_t size)
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

	bool register_at_index_(AbstractFuture& future, uint8_t index);

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
		if (new_address == nullptr)
			last_removed_id_ = id;
		return true;
	}

	const uint8_t size_;
	AbstractFuture** futures_;
	uint8_t last_removed_id_ = 0;

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
			return errors::EINVAL;
		}
	}

protected:
	// Constructor used by FutureManager
	AbstractFuture(uint8_t* output_data, uint8_t output_size, uint8_t* input_data, uint8_t input_size)
		:	output_data_{output_data}, output_current_{output_data}, output_size_{output_size},
			input_data_{input_data}, input_current_{input_data}, input_size_{input_size} {}
	~AbstractFuture()
	{
		// Notify FutureManager about destruction
		AbstractFutureManager::instance().update_future(id_, this, nullptr);
	}

	AbstractFuture(const AbstractFuture&) = delete;
	AbstractFuture& operator=(const AbstractFuture&) = delete;
	AbstractFuture(AbstractFuture&&) = delete;
	AbstractFuture& operator=(AbstractFuture&&) = delete;

	void invalidate()
	{
		status_ = FutureStatus::INVALID;
	}

	// This method is called by subclass in their move constructor and assignment operator
	void move_(AbstractFuture&& that, uint8_t full_output_size, uint8_t full_input_size)
	{
		// In case this Future is valid, it must be first invalidated with FutureManager
		AbstractFutureManager::instance().update_future_(id_, this, nullptr);

		// Now copy all attributes from rhs (output_data_ was already initialized when this was constructed)
		id_ = that.id_;
		status_ = that.status_;
		error_ = that.error_;
		output_size_ = that.output_size_;
		input_size_ -= that.input_size_;
		// Calculate data pointer attribute for next set value calls
		output_current_ = output_data_ + full_output_size - output_size_;
		input_current_ = input_data_ + full_input_size - input_size_;

		// Notify FutureManager about Future move
		if (!AbstractFutureManager::instance().update_future_(id_, &that, this))
			status_ = FutureStatus::INVALID;

		// Make rhs Future invalid
		that.id_ = 0;
		that.status_ = FutureStatus::INVALID;
	}

private:
	// The following methods are called by FutureManager to fill the Future value (or error)
	bool set_finish_()
	{
		// Check this future is waiting for data
		if (status_ != FutureStatus::NOT_READY)
			return false;
		if (output_size_ == 0)
			status_ = FutureStatus::READY;
		return true;
	}
	bool set_chunk_(uint8_t chunk)
	{
		// Check this future is waiting for data
		if (status_ != FutureStatus::NOT_READY)
			return false;
		// Update Future value chunk
		*output_current_++ = chunk;
		// Is that the last chunk?
		if (--output_size_ == 0)
			status_ = FutureStatus::READY;
		return true;
	}
	bool set_chunk_(const uint8_t* chunk, uint8_t size)
	{
		// Check this future is waiting for data
		if (status_ != FutureStatus::NOT_READY)
			return false;
		// Check size does not go beyond expected size
		if (size > output_size_)
		{
			// Store error
			set_error_(errors::EMSGSIZE);
			return false;
		}
		memcpy(output_current_, chunk, size);
		output_current_ += size;
		// Is that the last chunk?
		output_size_ -= size;
		if (output_size_ == 0)
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

	// The following methods are called by FutureManager to get the read-only value held by this Future
	bool get_chunk_(uint8_t& chunk)
	{
		// Check all bytes have not been transferred yet
		if (!input_size_)
			return false;
		chunk = *input_current_++;
		--input_size_;
		return true;
	}
	bool get_chunk_(uint8_t* chunk, uint8_t size)
	{
		// Check size does not go beyond transferrable size
		if (size > input_size_)
			return false;
		memcpy(chunk, input_current_, size);
		input_current_ += size;
		input_size_ -= size;
		return true;
	}

	uint8_t id_ = 0;
	volatile FutureStatus status_ = FutureStatus::INVALID;
	int error_ = 0;

	uint8_t* output_data_ = nullptr;
	uint8_t* output_current_ = nullptr;
	uint8_t output_size_ = 0;
	
	uint8_t* input_data_ = nullptr;
	uint8_t* input_current_ = nullptr;
	uint8_t input_size_ = 0;

	friend class AbstractFutureManager;
};

bool AbstractFutureManager::set_future_finish_(uint8_t id) const
{
	AbstractFuture* future = find_future(id);
	if (future == nullptr)
		return false;
	return future->set_finish_();
}
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

bool AbstractFutureManager::get_storage_value_(uint8_t id, uint8_t& chunk) const
{
	AbstractFuture* future = find_future(id);
	if (future == nullptr)
		return false;
	return future->get_chunk_(chunk);
}
bool AbstractFutureManager::get_storage_value_(uint8_t id, uint8_t* chunk, uint8_t size) const
{
	AbstractFuture* future = find_future(id);
	if (future == nullptr)
		return false;
	return future->get_chunk_(chunk, size);
}

// Future supports only types strictly smaller than 256 bytes
template<typename OUT = void, typename IN = void>
class Future : public AbstractFuture
{
	static_assert(sizeof(OUT) <= UINT8_MAX, "OUT type must be strictly smaller than 256 bytes");
	static_assert(sizeof(IN) <= UINT8_MAX, "IN type must be strictly smaller than 256 bytes");

public:
	Future(const IN& input = IN{})
		: AbstractFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN)}, input_{input} {}
	~Future() = default;

	Future(Future<OUT, IN>&& that) : AbstractFuture{output_buffer_, sizeof(OUT), input_buffer_, sizeof(IN)}
	{
		move(std::move(that));
	}
	Future<OUT, IN>& operator=(Future<OUT, IN>&& that)
	{
		if (this == &that) return *this;
		move(std::move(that));
		return *this;
	}

	Future(const Future<OUT, IN>&) = delete;
	Future<OUT, IN>& operator=(const Future<OUT, IN>&) = delete;

	// The following method is blocking until this Future is ready
	bool get(OUT& result)
	{
		if (await() != FutureStatus::READY)
			return false;
		result = output_;
		invalidate();
		return true;
	}

private:
	void move(Future<OUT, IN>&& that)
	{
		synchronized
		{
			memcpy(output_buffer_, that.output_buffer_, sizeof(OUT));
			memcpy(input_buffer_, that.input_buffer_, sizeof(IN));
			move_(std::move(that), sizeof(OUT), sizeof(IN));
		}
	}

	union
	{
		OUT output_;
		uint8_t output_buffer_[sizeof(OUT)];
	};
	union
	{
		const IN input_;
		uint8_t input_buffer_[sizeof(IN)];
	};
};

// Template specialization for void types
template<typename OUT>
class Future<OUT, void> : public AbstractFuture
{
	static_assert(sizeof(OUT) <= UINT8_MAX, "OUT type must be strictly smaller than 256 bytes");

public:
	Future() : AbstractFuture{output_buffer_, sizeof(OUT), nullptr, 0} {}
	~Future() = default;

	Future(Future<OUT, void>&& that) : AbstractFuture{output_buffer_, sizeof(OUT), nullptr, 0}
	{
		move(std::move(that));
	}
	Future<OUT, void>& operator=(Future<OUT, void>&& that)
	{
		if (this == &that) return *this;
		move(std::move(that));
		return *this;
	}

	Future(const Future<OUT, void>&) = delete;
	Future<OUT, void>& operator=(const Future<OUT, void>&) = delete;

	// The following method is blocking until this Future is ready
	bool get(OUT& result)
	{
		if (await() != FutureStatus::READY)
			return false;
		result = output_;
		invalidate();
		return true;
	}

private:
	void move(Future<OUT, void>&& that)
	{
		synchronized
		{
			memcpy(output_buffer_, that.output_buffer_, sizeof(OUT));
			move_(std::move(that), sizeof(OUT), 0);
		}
	}

	union
	{
		OUT output_;
		uint8_t output_buffer_[sizeof(OUT)];
	};
};

template<typename IN>
class Future<void, IN> : public AbstractFuture
{
	static_assert(sizeof(IN) <= UINT8_MAX, "IN type must be strictly smaller than 256 bytes");

public:
	Future(const IN& input = IN{})
		: AbstractFuture{nullptr, 0, input_buffer_, sizeof(IN)}, input_{input} {}
	~Future() = default;

	Future(Future<void, IN>&& that) : AbstractFuture{nullptr, 0, input_buffer_, sizeof(IN)}
	{
		move(std::move(that));
	}
	Future<void, IN>& operator=(Future<void, IN>&& that)
	{
		if (this == &that) return *this;
		move(std::move(that));
		return *this;
	}

	Future(const Future<void, IN>&) = delete;
	Future<void, IN>& operator=(const Future<void, IN>&) = delete;

	// The following method is blocking until this Future is ready
	bool get()
	{
		if (await() != FutureStatus::READY)
			return false;
		invalidate();
		return true;
	}

private:
	void move(Future<void, IN>&& that)
	{
		synchronized
		{
			memcpy(input_buffer_, that.input_buffer_, sizeof(IN));
			move_(std::move(that), 0, sizeof(IN));
		}
	}

	union
	{
		const IN input_;
		uint8_t input_buffer_[sizeof(IN)];
	};
};

template<>
class Future<void, void> : public AbstractFuture
{
public:
	Future() : AbstractFuture{nullptr, 0,nullptr, 0} {}
	~Future() = default;

	Future(Future<void, void>&& that) : AbstractFuture{nullptr, 0, nullptr, 0}
	{
		synchronized move_(std::move(that), 0, 0);
	}
	Future<void, void>& operator=(Future<void, void>&& that)
	{
		if (this == &that) return *this;
		synchronized move_(std::move(that), 0, 0);
		return *this;
	}

	Future(const Future<void, void>&) = delete;
	Future<void, void>& operator=(const Future<void, void>&) = delete;

	// The following method is blocking until this Future is ready
	bool get()
	{
		if (await() != FutureStatus::READY)
			return false;
		invalidate();
		return true;
	}
};

template<typename OUT, typename IN>
bool AbstractFutureManager::register_future_(Future<OUT, IN>& future)
{
	// You cannot register an already registered future
	if (future.id() != 0)
		return false;
	// Optimization: we start search AFTER the last removed id
	for (uint8_t i = last_removed_id_; i < size_; ++i)
		if (register_at_index_(future, i))
			return true;
	for (uint8_t i = 0; i <= last_removed_id_; ++i)
		if (register_at_index_(future, i))
			return true;
	return false;
}

bool AbstractFutureManager::register_at_index_(AbstractFuture& future, uint8_t index)
{
	if (futures_[index] != nullptr)
		return false;
	update_future_(future.id_, &future, nullptr);
	future.id_ = static_cast<uint8_t>(index + 1);
	future.status_ = FutureStatus::NOT_READY;
	futures_[index] = &future;
	return true;
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

#define ECHO_ID(OUT, FUTURE) OUT << F("" #FUTURE ".id() = ") << FUTURE.id() << endl

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

	// Check normal error context
	out << F("TEST #1 simple Future lifecycle: normal error case") << endl;
	out << F("#1.1 instantiate future") << endl;
	Future<uint16_t> future1;
	ASSERT_STATUS(out, INVALID, future1);
	out << F("#1.2 register_future()") << endl;
	ASSERT(out, manager.register_future(future1));
	ECHO_ID(out, future1);
	ASSERT_STATUS(out, NOT_READY, future1);
	assert(out, F("available futures"), MAX_FUTURES - 1, manager.available_futures());
	out << F("#1.3 set_future_error()") << endl;
	ASSERT(out, manager.set_future_error(future1.id(), 0x1111));
	ASSERT_STATUS(out, ERROR, future1);
	ASSERT_ERROR(out, 0x1111, future1);
	ASSERT_STATUS(out, INVALID, future1);
	out << endl;

	// Check full data set
	out << F("TEST #2 simple Future lifecycle: new Future and full value set") << endl;
	out << F("#2.1 instantiate future") << endl;
	Future<uint16_t> future2;
	ASSERT_STATUS(out, INVALID, future2);
	out << F("#2.2 register_future()") << endl;
	ASSERT(out, manager.register_future(future2));
	ECHO_ID(out, future2);
	ASSERT_STATUS(out, NOT_READY, future2);
	out << F("#2.3 set_future_value()") << endl;
	ASSERT(out, manager.set_future_value(future2.id(), 0x8000));
	ASSERT_STATUS(out, READY, future2);
	ASSERT_ERROR(out, 0, future2);
	ASSERT_STATUS(out, READY, future2);
	ASSERT_VALUE(out, 0x8000u, future2);
	ASSERT_STATUS(out, INVALID, future2);
	ASSERT_ERROR(out, errors::EINVAL, future2);
	ASSERT_STATUS(out, INVALID, future2);
	out << endl;

	// Check set value by chunks
	out << F("TEST #3 simple Future lifecycle: new Future and partial value set") << endl;
	out << F("#3.1 instantiate future") << endl;
	Future<uint16_t> future3;
	ASSERT_STATUS(out, INVALID, future3);
	out << F("#3.2 register future") << endl;
	ASSERT(out, manager.register_future(future3));
	ECHO_ID(out, future3);
	ASSERT_STATUS(out, NOT_READY, future3);
	out << F("#3.3 set_future_value() chunk1") << endl;
	ASSERT(out, manager.set_future_value(future3.id(), uint8_t(0x11)));
	ASSERT_STATUS(out, NOT_READY, future3);
	out << F("#3.4 set_future_value() chunk2") << endl;
	ASSERT(out, manager.set_future_value(future3.id(), uint8_t(0x22)));
	ASSERT_STATUS(out, READY, future3);
	ASSERT_VALUE(out, 0x2211u, future3);
	ASSERT_STATUS(out, INVALID, future3);
	out << endl;

	// Check set value by data pointer once
	out << F("TEST #4 simple Future lifecycle: new Future and full value pointer set") << endl;
	out << F("#4.1 instantiate future") << endl;
	Future<uint16_t> future4;
	ASSERT_STATUS(out, INVALID, future4);
	out << F("#4.2 register future") << endl;
	ASSERT(out, manager.register_future(future4));
	ECHO_ID(out, future4);
	ASSERT_STATUS(out, NOT_READY, future4);
	out << F("#4.3 set_future_value() from ptr") << endl;
	const uint16_t constant1 = 0x4433;
	ASSERT(out, manager.set_future_value(future4.id(), (const uint8_t*) &constant1, sizeof(constant1)));
	ASSERT_STATUS(out, READY, future4);
	ASSERT_VALUE(out, 0x4433u, future4);
	ASSERT_STATUS(out, INVALID, future4);
	out << endl;

	// Check set value by data pointer twice
	out << F("TEST #5 simple Future lifecycle: new Future and part value pointer set") << endl;
	out << F("#5.1 instantiate future") << endl;
	Future<uint16_t> future5;
	ASSERT_STATUS(out, INVALID, future5);
	out << F("#5.2 register future") << endl;
	ASSERT(out, manager.register_future(future5));
	ECHO_ID(out, future5);
	ASSERT_STATUS(out, NOT_READY, future5);
	out << F("#5.3 set_future_value() from ptr (1 byte)") << endl;
	const uint16_t constant2 = 0x5566;
	ASSERT(out, manager.set_future_value(future5.id(), (const uint8_t*) &constant2, 1));
	ASSERT_STATUS(out, NOT_READY, future5);
	out << F("#5.4 set_future_value() from ptr (2nd byte)") << endl;
	ASSERT(out, manager.set_future_value(future5.id(), ((const uint8_t*) &constant2) + 1, 1));
	ASSERT_STATUS(out, READY, future5);
	ASSERT_VALUE(out, 0x5566u, future5);
	ASSERT_STATUS(out, INVALID, future5);
	out << endl;

	// Check further updates do not do anything (and do not crash either!)
	out << F("TEST #6 simple Future lifecycle: check no more updates possible after first set complete") << endl;
	out << F("#6.1 instantiate future") << endl;
	Future<uint16_t> future6;
	ASSERT_STATUS(out, INVALID, future6);
	out << F("#6.2 register future") << endl;
	ASSERT(out, manager.register_future(future6));
	ECHO_ID(out, future6);
	ASSERT_STATUS(out, NOT_READY, future6);
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
	out << endl;

	// Check reuse of a future in various states
	out << F("TEST #7 check Future status after move constructor") << endl;
	out << F("#7.1 instantiate future") << endl;
	Future<uint16_t> future7;
	ASSERT_STATUS(out, INVALID, future7);
	out << F("#7.2 register future") << endl;
	ASSERT(out, manager.register_future(future7));
	ECHO_ID(out, future7);
	ASSERT_STATUS(out, NOT_READY, future7);
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
	ECHO_ID(out, future10);
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
	ECHO_ID(out, future14);
	ASSERT(out, manager.set_future_value(future14.id(), uint8_t(0xBB)));
	Future<uint16_t> future15 = std::move(future14);
	ASSERT_STATUS(out, INVALID, future14);
	ASSERT_STATUS(out, NOT_READY, future15);
	ASSERT(out, manager.set_future_value(future15.id(), uint8_t(0xCC)));
	out << F("#7.8 Complete set value") << endl;
	ASSERT_STATUS(out, READY, future15);
	ASSERT_ERROR(out, 0, future15);
	ASSERT_VALUE(out, 0xCCBBu, future15);
	out << endl;

	// Check reuse of a future in various states
	out << F("TEST #8 check Future status after move assignment") << endl;
	out << F("#8.1 instantiate futures") << endl;
	Future<uint16_t> future17, future18, future19, future20, future21, future22, future23, future24, future25;
	out << F("#8.2 register future") << endl;
	ASSERT(out, manager.register_future(future17));
	ECHO_ID(out, future17);
	ASSERT_STATUS(out, NOT_READY, future17);
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
	ECHO_ID(out, future20);
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
	ECHO_ID(out, future24);
	ASSERT(out, manager.set_future_value(future24.id(), uint8_t(0xBB)));
	future25 = std::move(future24);
	ASSERT_STATUS(out, INVALID, future24);
	ASSERT_STATUS(out, NOT_READY, future25);
	out << F("#8.8 after complete set value, status shall be READY") << endl;
	ASSERT(out, manager.set_future_value(future25.id(), uint8_t(0xCC)));
	ASSERT_STATUS(out, READY, future25);
	ASSERT_ERROR(out, 0, future25);
	ASSERT_VALUE(out, 0xCCBBu, future25);
	out << endl;

	// Check Future subclassing
	out << F("TEST #9 Future subclassing...") << endl;
	out << F("#9.1 instantiate future") << endl;
	MyFuture my_future;
	ASSERT_STATUS(out, INVALID, my_future);
	out << F("#9.2 register_future()") << endl;
	ASSERT(out, manager.register_future(my_future));
	ECHO_ID(out, my_future);
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

	// Check value storage in Future
	out << F("TEST #10 Future value storage...") << endl;
	out << F("#10.1 instantiate future") << endl;
	Future<uint16_t, uint16_t> future26{12345};
	ASSERT_STATUS(out, INVALID, future26);
	out << F("#10.2 register_future()") << endl;
	ASSERT(out, manager.register_future(future26));
	ECHO_ID(out, future26);
	ASSERT_STATUS(out, NOT_READY, future26);
	out << F("#10.3 get storage value") << endl;
	uint16_t input = 0;
	ASSERT(out, manager.get_storage_value(future26.id(), (uint8_t*) &input, sizeof(input)));
	assert(out, F("get_storage_value((future26.id())"), 12345u, input);
	ASSERT_STATUS(out, NOT_READY, future26);
	out << F("#10.4 set_future_value()") << endl;
	ASSERT(out, manager.set_future_value(future26.id(), 123));
	ASSERT_STATUS(out, READY, future26);
	out << F("#10.5 get()") << endl;
	actual = 0;
	ASSERT(out, future26.get(actual));
	assert(out, F("future26.get() value"), 123u, actual);
	ASSERT_STATUS(out, INVALID, future26);
	out << endl;

	// Check Future without value (just done or error or not)
	out << F("TEST #11 Future without value...") << endl;
	out << F("#11.1 instantiate future") << endl;
	Future<> future27;
	ASSERT_STATUS(out, INVALID, future27);
	out << F("#11.2 register_future()") << endl;
	ASSERT(out, manager.register_future(future27));
	ECHO_ID(out, future27);
	ASSERT_STATUS(out, NOT_READY, future27);
	out << F("#11.3 set finish()") << endl;
	ASSERT(out, manager.set_future_finish(future27.id()));
	ASSERT_STATUS(out, READY, future27);
	ASSERT(out, future27.get());
	out << endl;

	time::delay_ms(1000);
	out << F("TEST #12 Future updated by ISR...") << endl;
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
		ECHO_ID(out, future);
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
