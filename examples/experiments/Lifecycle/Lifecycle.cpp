/*
 * This program is just for personal experiments here on C++ mixin for 
 * handling lifecycle of specific objects.
 */

#include <string.h>
#include <fastarduino/types_traits.h>

// includes for the example program itself
#include <fastarduino/tests/assertions.h>
#include <fastarduino/uart.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/flash.h>
#include <fastarduino/move.h>

// Register vector for UART (used for debug)
REGISTER_UATX_ISR(0)

// API Proof of Concept
//======================

//TODO check bad use of proxy constructor does not compile!
//TODO see if further code size optimization is possible
//FIXME 2 AbstractLifeCycle may have different managers?
// - in this case, how should the move (ctor and assignment) should be handled?
// => that would bring additional code maybe for nothing?

// Forward declaration
class AbstractLifeCycleManager;
template<typename T> class LifeCycle;

// LifeCycle handling
class AbstractLifeCycle
{
public:
	AbstractLifeCycle() = default;
	AbstractLifeCycle(AbstractLifeCycle&& that);
	~AbstractLifeCycle();
	AbstractLifeCycle& operator=(AbstractLifeCycle&& that);

	AbstractLifeCycle(const AbstractLifeCycle&) = delete;
	AbstractLifeCycle& operator=(const AbstractLifeCycle&) = delete;

	uint8_t id() const
	{
		return id_;
	}
	AbstractLifeCycleManager* manager() const
	{
		return manager_;
	}

private:
	uint8_t id_ = 0;
	AbstractLifeCycleManager* manager_ = nullptr;

	friend class AbstractLifeCycleManager;
};

class AbstractLifeCycleManager
{
public:
	template<typename T> uint8_t register_(LifeCycle<T>& instance)
	{
		return register_impl_(instance);
	}

	bool unregister_(uint8_t id)
	{
		AbstractLifeCycle** slot = find_slot_(id);
		if (slot == nullptr || *slot == nullptr)
			return false;
		AbstractLifeCycle& source = **slot;
		source.id_ = 0;
		source.manager_ = nullptr;
		*slot = nullptr;
		++free_slots_;
		return true;
	}

	uint8_t available_() const
	{
		return free_slots_;
	}
	
	bool move_(uint8_t id, AbstractLifeCycle& dest)
	{
		// Check this instance is managed
		AbstractLifeCycle** slot = find_slot_(id);
		if (slot == nullptr || *slot == nullptr) return false;
		// Perform move
		AbstractLifeCycle& source = **slot;
		dest.id_ = source.id_;
		dest.manager_ = this;
		source.id_ = 0;
		source.manager_ = nullptr;
		*slot = &dest;
		return true;
	}

	template<typename T> LifeCycle<T>* find_(uint8_t id) const
	{
		return static_cast<LifeCycle<T>*>(find_impl_(id));
	}

protected:
	AbstractLifeCycleManager(AbstractLifeCycle** slots, uint8_t size)
		:	size_{size}, slots_{slots}, free_slots_{size}
	{
		memset(slots, 0, size * sizeof(AbstractLifeCycle*));
	}
	~AbstractLifeCycleManager() = default;
	AbstractLifeCycleManager(const AbstractLifeCycleManager&) = delete;
	AbstractLifeCycleManager& operator=(const AbstractLifeCycleManager&) = delete;

private:
	uint8_t register_impl_(AbstractLifeCycle& instance)
	{
		// Youcannot register any instance if there are no free slots remaining
		if (free_slots_ == 0) return 0;
		// You cannot register an already registered future
		if (instance.id_ != 0) return 0;
		// Optimization: we start search AFTER the last removed id
		for (uint8_t i = last_removed_id_; i < size_; ++i)
			if (register_at_index_(instance, i))
				return i + 1;
		for (uint8_t i = 0; i <= last_removed_id_; ++i)
			if (register_at_index_(instance, i))
				return i + 1;
		return 0;
	}

	AbstractLifeCycle* find_impl_(uint8_t id) const
	{
		AbstractLifeCycle** slot = find_slot_(id);
		if (slot == nullptr)
			return nullptr;
		else
			return *slot;
	}

	AbstractLifeCycle** find_slot_(uint8_t id) const
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
		--free_slots_;
		return true;
	}

	const uint8_t size_;
	AbstractLifeCycle** slots_;
	uint8_t free_slots_;
	uint8_t last_removed_id_ = 0;
};

template<uint8_t SIZE> class LifeCycleManager : public AbstractLifeCycleManager
{
public:
	LifeCycleManager() : AbstractLifeCycleManager{slots_buffer_, SIZE} {}

private:
	AbstractLifeCycle* slots_buffer_[SIZE];
};

AbstractLifeCycle::AbstractLifeCycle(AbstractLifeCycle&& that)
{
	if (that.id_)
		that.manager()->move_(that.id_, *this);
}
AbstractLifeCycle::~AbstractLifeCycle()
{
	if (id_)
		manager()->unregister_(id_);
}
AbstractLifeCycle& AbstractLifeCycle::operator=(AbstractLifeCycle&& that)
{
	if (id_)
		manager()->unregister_(id_);
	if (that.id_)
		that.manager()->move_(that.id_, *this);
	return *this;
}

template<typename T> class LifeCycle : public AbstractLifeCycle, public T
{
public:
	LifeCycle(const T& value = T{}) : AbstractLifeCycle{}, T{value} {}
	LifeCycle(LifeCycle<T>&& that) = default;
	~LifeCycle() = default;
	LifeCycle<T>& operator=(LifeCycle<T>&& that) = default;
};

template<typename T> class Proxy
{
public:
	Proxy(T& dest) : id_{0}, dest_{&dest} {}
	template<typename U>
	Proxy(const LifeCycle<U>& dest) : id_{dest.id()}, manager_{dest.manager()}
	{
		// Statically check (at compile-time) that U is a subclass of T
		UNUSED types_traits::derives_from<U, T> check;
	}
	Proxy(const Proxy&) = default;
	Proxy& operator=(const Proxy&) = default;

	T& operator*()
	{
		return *target();
	}
	T* operator&()
	{
		return target();
	}
	T* operator->()
	{
		return target();
	}

	uint8_t id() const
	{
		return id_;
	}
	T* destination() const
	{
		return (id_ == 0 ? dest_ : nullptr);
	}
	AbstractLifeCycleManager* manager() const
	{
		return (id_ != 0 ? manager_ : nullptr);
	}

private:
	T* target() const
	{
		if (id_ == 0)
			return dest_;
		else
			return manager_->find_<T>(id_);
	}

	const uint8_t id_;
	union
	{
		T* dest_;
		AbstractLifeCycleManager* manager_;
	};
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

	int val() const
	{
		return val_;
	}

	static ostream& out()
	{
		return *out_;
	}

protected:
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

template<typename T> static void check(ostream& out, AbstractLifeCycleManager& manager, const T& init)
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
		LifeCycle<T>* found = manager.find_<T>(id);
		assert(out, F("manager.find_(id)"), found != nullptr);
		assert(out, F("manager.find_(id)"), &instance, found);
		out << F("val=") << dec << found->val() << endl;

		//TODO Check copy never compiles
		// LifeCycle<T> copy{instance};

		out << F("3. Move constructor") << endl;
		LifeCycle<T> move = std::move(instance);
		assert(out, F("original id() after registration"), 0, instance.id());
		assert(out, F("moved id() after registration"), id, move.id());
		assert(out, F("available_slots()"), MAX_LC_SLOTS - 1, manager.available_());

		out << F("4. Find after move") << endl;
		found = manager.find_<T>(id);
		assert(out, F("manager.find_(id)"), found != nullptr);
		assert(out, F("manager.find_(id)"), &move, found);
		out << F("val=") << dec << found->val() << endl;

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

class SubValue : public Value
{
public:
	SubValue(int val = 0, int val2 = 0) : Value{val}, val2_{val2} {}
	~SubValue() = default;
	SubValue(const SubValue& that) = default;
	SubValue(SubValue&& that) = default;
	SubValue& operator=(const SubValue& that) = default;
	SubValue& operator=(SubValue&& that) = default;

	int val2() const
	{
		return val2_;
	}

private:
	int val2_;
};

void check_proxies(ostream& out, AbstractLifeCycleManager& manager)
{
	Value v1{10};
	SubValue v2{20, 30};

	Proxy<Value> p1{v1};
	Proxy<Value> p2{v2};
	out << F("p1->val() ") << hex << &p1 << ' ' << dec << p1->val() << endl;
	out << F("p2->val() ") << hex << &p2 << ' ' << dec << p2->val() << endl;

	LifeCycle<Value> lc1{v1};
	assert(out, F("manager.register_(lc1)"), 1, manager.register_(lc1));
	assert(out, F("lc1.id()"), 1, lc1.id());
	LifeCycle<SubValue> lc2{v2};
	assert(out, F("manager.register_(lc2)"), 2, manager.register_(lc2));
	assert(out, F("lc2.id()"), 2, lc2.id());

	Proxy<Value> p3{lc1};
	out << F("p3.id=") << dec << p3.id()
		<< F(" p3.manager=") << hex << p3.manager() 
		<< F(" p3.dest=") << hex << p3.destination() << endl;
	Proxy<Value> p4{lc2};
	out << F("p4.id=") << dec << p4.id() 
		<< F(" p4.manager=") << hex << p4.manager()
		<< F(" p4.dest=") << hex << p4.destination() << endl;
	out << F("p3->val() ") << hex << &p3 << ' ' << dec << p3->val() << endl;
	out << F("p4->val() ") << hex << &p4 << ' ' << dec << p4->val() << endl;
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
	const Value VAL0 = Value{123};

	// Create manager
	out << F("Instantiate LifeCycleManager") << endl;
	LifeCycleManager<MAX_LC_SLOTS> manager;
	// Check available slots
	assert(out, F("available_slots()"), MAX_LC_SLOTS, manager.available_());

	// Check different types T (int, struct, with/without dtor/ctor/op=...)
	// check<Value>(out, manager, VAL0);

	check_proxies(out, manager);
}
