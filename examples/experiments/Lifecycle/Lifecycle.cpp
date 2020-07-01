/*
 * This program is just for personal experiments here on C++ mixin for 
 * handling lifecycle of specific objects.
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

// Register vector for UART (used for debug)
REGISTER_UATX_ISR(0)

// API Proof of Concept
//======================

//TODO LifeCycleManager improvements: add template to allow register and find with strong types?
//TODO see if further code size optimization is possible

// Forward declaration
class LifeCycleManager;

// LifeCycle handling
class AbstractLifeCycle
{
public:
	AbstractLifeCycle() = default;
	uint8_t id() const
	{
		return id_;
	}

protected:
	uint8_t id_ = 0;
	LifeCycleManager* manager_ = nullptr;

	friend class LifeCycleManager;
};

class LifeCycleManager
{
public:
	template<uint8_t SIZE>
	LifeCycleManager(AbstractLifeCycle* (&slots)[SIZE]) : size_{SIZE}, slots_{slots} {}
	~LifeCycleManager() = default;
	LifeCycleManager(const LifeCycleManager&) = delete;
	LifeCycleManager& operator=(const LifeCycleManager&) = delete;

	uint8_t register_(AbstractLifeCycle& instance)
	{
		// You cannot register an already registered future
		if (instance.id_ != 0)
			return 0;
		// Optimization: we start search AFTER the last removed id
		for (uint8_t i = last_removed_id_; i < size_; ++i)
			if (register_at_index_(instance, i))
				return i + 1;
		for (uint8_t i = 0; i <= last_removed_id_; ++i)
			if (register_at_index_(instance, i))
				return i + 1;
		return 0;
	}

	bool unregister_(uint8_t id)
	{
		AbstractLifeCycle** slot = find_slot_(id);
		if (slot == nullptr || *slot == nullptr)
			return false;
		(*slot)->id_ = 0;
		(*slot)->manager_ = nullptr;
		*slot = nullptr;
		return true;
	}

	uint8_t available_() const
	{
		uint8_t free = 0;
		for (uint8_t i = 0; i < size_; ++i)
			if (slots_[i] == nullptr)
				++free;
		return free;
	}
	
	bool move_(uint8_t id, AbstractLifeCycle& dest)
	{
		// Check this instance is managed
		AbstractLifeCycle** slot = find_slot_(id);
		if (slot == nullptr || *slot == nullptr)
			return false;
		// Perform move
		AbstractLifeCycle& source = **slot;
		dest.id_ = source.id_;
		dest.manager_ = this;
		source.id_ = 0;
		source.manager_ = nullptr;
		*slot = &dest;
		return true;
	}

	AbstractLifeCycle* find_(uint8_t id)
	{
		AbstractLifeCycle** slot = find_slot_(id);
		if (slot == nullptr)
			return nullptr;
		else
			return *slot;
	}

private:
	AbstractLifeCycle** find_slot_(uint8_t id)
	{
		if (id == 0 || id > size_)
			return nullptr;
		else
			return &slots_[id - 1];
	}

	bool register_at_index_(AbstractLifeCycle& instance, uint8_t index)
	{
		if (slots_[index] != nullptr)
			return false;
		instance.id_ = static_cast<uint8_t>(index + 1);
		instance.manager_ = this;
		slots_[index] = &instance;
		return true;
	}

	const uint8_t size_;
	AbstractLifeCycle** slots_;
	uint8_t last_removed_id_ = 0;
};

template<typename T> class LifeCycle : public AbstractLifeCycle, public T
{
public:
	LifeCycle(const T& value = T{}) : T{value} {}
	LifeCycle(const LifeCycle<T>&) = delete;
	LifeCycle(LifeCycle<T>&& that) : AbstractLifeCycle{std::move(that)}, T{std::move((T&&) that)}
	{
		//TODO try to move it to superclass (non template)
		if (that.id_)
			that.manager_->move_(that.id_, *this);
	}
	~LifeCycle()
	{
		T::out() << F("~LifeCycle(), id_ = ") << streams::dec << id_ << streams::endl;
		//TODO try to move it to superclass (non template)
		if (id_)
			manager_->unregister_(id_);
	}
	LifeCycle<T>& operator=(const LifeCycle<T>&) = delete;
	LifeCycle<T>& operator=(LifeCycle<T>&& that)
	{
		T::operator=(std::move(that));
		//TODO try to move it to superclass (non template)
		if (id_)
			manager_->unregister_(id_);
		if (that.id_)
			that.manager_->move_(that.id_, *this);
		return *this;
	}
};

// Usage Example
//===============
using namespace streams;

// Define types that output traces on each ctor/dtor/operator=
class Value
{
public:
	static void set_out(ostream& out)
	{
		out_ = &out;
	}
	Value(int val = 0) : val_{val}
	{
		trace('c');
	}
	~Value()
	{
		trace('d');
	}
	Value(const Value& that) : val_{that.val_}
	{
		trace('C');
	}
	Value(Value&& that) : val_{that.val_}
	{
		trace('M');
	}
	Value& operator=(const Value& that)
	{
		this->val_ = that.val_;
		trace('=');
		return *this;
	}
	Value& operator=(Value&& that)
	{
		this->val_ = that.val_;
		trace('m');
		return *this;
	}

	static ostream& out()
	{
		return *out_;
	}

private:
	void trace(char method)
	{
		if (out_)
			*out_ << method << dec << val_ << ' ' << hex << this << endl;
	}

	static ostream* out_;
	int val_;
};

ostream* Value::out_ = nullptr;

static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static char output_buffer[OUTPUT_BUFFER_SIZE];

static constexpr const uint8_t MAX_LC_SLOTS = 32;
static AbstractLifeCycle* lc_slots[MAX_LC_SLOTS];

template<typename T> static void check(ostream& out, LifeCycleManager& manager, const T& init)
{
	{
		out << F("0. Instance creation") << endl;
		LifeCycle<T> instance{init};
		assert(out, F("available_slots()"), MAX_LC_SLOTS, manager.available_());
		assert(out, F("id() after construction"), 0, instance.id());

		out << F("1. Registration") << endl;
		uint8_t id = manager.register_(instance);
		assert(out, F("id returned by register_()"), id);
		assert(out, F("id() after registration"), id, instance.id());
		assert(out, F("available_slots()"), MAX_LC_SLOTS - 1, manager.available_());

		out << F("2. Find") << endl;
		AbstractLifeCycle* found = manager.find_(id);
		assert(out, F("manager.find_(id)"), found != nullptr);
		assert(out, F("manager.find_(id)"), &instance, found);

		//TODO Check copy never compiles
		// LifeCycle<T> copy{instance};

		out << F("3. Move constructor") << endl;
		LifeCycle<T> move = std::move(instance);
		assert(out, F("original id() after registration"), 0, instance.id());
		assert(out, F("moved id() after registration"), id, move.id());
		assert(out, F("available_slots()"), MAX_LC_SLOTS - 1, manager.available_());

		out << F("4. Find after move") << endl;
		found = manager.find_(id);
		assert(out, F("manager.find_(id)"), found != nullptr);
		assert(out, F("manager.find_(id)"), &move, found);

		//TODO Check copy never compiles
		// LifeCycle<T> copy;
		// copy = move;

		out << F("5. Move assignment") << endl;
		LifeCycle<T> move2;
		move2 = std::move(move);
		assert(out, F("original id() after registration"), 0, move.id());
		assert(out, F("moved id() after registration"), id, move2.id());
		assert(out, F("available_slots()"), MAX_LC_SLOTS - 1, manager.available_());
	}

	// Check destruction
	out << F("6. Destruction") << endl;
	assert(out, F("available_slots()"), MAX_LC_SLOTS, manager.available_());
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

	out << F("Starting...") << endl;

	Value::set_out(out);
	out << F("Create constant Value first") << endl;
	const Value VAL0 = Value{1};

	// Create manager
	out << F("Instantiate LifeCycleManager") << endl;
	LifeCycleManager manager{lc_slots};
	// Check available slots
	assert(out, F("available_slots()"), MAX_LC_SLOTS, manager.available_());

	// Check different types T (int, struct, with/without dtor/ctor/op=...)
	check<Value>(out, manager, VAL0);
}
