/*
 * This program is just for personal experiments here on C++ mixin for 
 * handling lifecycle of specific objects.
 */

#include <fastarduino/boards/board.h>
#include <fastarduino/move.h>
#include <fastarduino/lifecycle.h>
// This include is for sei() method
#include <avr/interrupt.h>

using namespace lifecycle;

class Value
{
public:
	Value(int val = 0) : val_{val} {}
	~Value() = default;
	Value(const Value& that) = default;
	Value(Value&& that) = default;
	Value& operator=(const Value& that) = default;
	Value& operator=(Value&& that) = default;

	int val() const
	{
		return val_;
	}

protected:
	int val_;
};

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

class Value2
{
public:
	Value2(int8_t val = 0) : val_{val} {}
	~Value2() = default;
	Value2(const Value2& that) = default;
	Value2(Value2&& that) = default;
	Value2& operator=(const Value2& that) = default;
	Value2& operator=(Value2&& that) = default;

	int val() const
	{
		return val_;
	}

protected:
	int8_t val_;
};

class Value3
{
public:
	Value3(int32_t val = 0L) : val_{val} {}
	~Value3() = default;
	Value3(const Value3& that) = default;
	Value3(Value3&& that) = default;
	Value3& operator=(const Value3& that) = default;
	Value3& operator=(Value3&& that) = default;

	int val() const
	{
		return (int) val_;
	}

protected:
	int32_t val_;
};

static constexpr const uint8_t MAX_LC_SLOTS = 32;

template<typename T> static int check_lc(AbstractLifeCycleManager& manager, const T& init)
{
	int value = 0;

	LifeCycle<T> instance{init};
	value = instance.val();
	uint8_t id = manager.register_(instance);
	LifeCycle<T>* found = manager.find_<T>(id);
	value += found->val();

	LifeCycle<T> move = std::move(instance);
	value += move.val() * 2;
	found = manager.find_<T>(id);
	value += found->val() * 4;

	LifeCycle<T> move2;
	move2 = std::move(move);
	value += move2.val() * 8;
	found = manager.find_<T>(id);
	value += found->val() * 16;

	return value;
}

template<typename T> static int check_proxies(AbstractLifeCycleManager& manager, const T& init)
{
	LifeCycle<T> lc1{init};
	manager.register_(lc1);

	Proxy<T> p1{lc1};
	return p1->val();
}

template<typename T> static int check_proxies(const T& init)
{
	Proxy<T> p1{init};
	return p1->val();
}

template<typename T> static int check_light_proxies(AbstractLifeCycleManager& manager, const T& init)
{
	LifeCycle<T> lc1{init};
	manager.register_(lc1);

	LightProxy<T> p1{lc1};
	return p1(&manager)->val();
}

template<typename T> static int check_light_proxies(const T& init)
{
	LightProxy<T> p1{init};
	return p1()->val();
}

static int check_proxies_inheritance(AbstractLifeCycleManager& manager)
{
	int value = 0;

	const Value v1{10};
	const SubValue v2{20, 30};

	LifeCycle<Value> lc1{v1};
	manager.register_(lc1);

	LifeCycle<SubValue> lc2{v2};
	manager.register_(lc2);

	Proxy<Value> p1{lc1};
	Proxy<Value> p2{lc2};
	Proxy<SubValue> p3{lc2};

	value = p1->val();
	value += p2->val() * 2;	
	value += p3->val2() * 4;

	return value;
}

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Enable interrupts at startup time
	sei();

	const Value VAL0 = Value{1230};
	const Value2 VAL1 = Value2{123};
	const Value3 VAL2 = Value3{123000L};

	// Create manager
	LifeCycleManager<MAX_LC_SLOTS> manager;

	// Check different types T (int, struct, with/without dtor/ctor/op=...)
	int value = 0;
	do
	{
		__asm__ __volatile__("nop");
		value += check_lc(manager, VAL0);
		__asm__ __volatile__("nop");
		value += check_lc(manager, VAL1);
		__asm__ __volatile__("nop");
		value += check_lc(manager, VAL2);
		__asm__ __volatile__("nop");

		__asm__ __volatile__("nop");
		value += check_proxies(manager, VAL0);
		__asm__ __volatile__("nop");
		value += check_proxies(manager, VAL1);
		__asm__ __volatile__("nop");
		value += check_proxies(manager, VAL2);
		__asm__ __volatile__("nop");

		__asm__ __volatile__("nop");
		value += check_proxies(VAL0);
		__asm__ __volatile__("nop");
		value += check_proxies(VAL1);
		__asm__ __volatile__("nop");
		value += check_proxies(VAL2);
		__asm__ __volatile__("nop");

		__asm__ __volatile__("nop");
		value += check_light_proxies(manager, VAL0);
		__asm__ __volatile__("nop");
		value += check_light_proxies(manager, VAL1);
		__asm__ __volatile__("nop");
		value += check_light_proxies(manager, VAL2);
		__asm__ __volatile__("nop");

		__asm__ __volatile__("nop");
		value += check_light_proxies(VAL0);
		__asm__ __volatile__("nop");
		value += check_light_proxies(VAL1);
		__asm__ __volatile__("nop");
		value += check_light_proxies(VAL2);
		__asm__ __volatile__("nop");

		__asm__ __volatile__("nop");
		value += check_proxies_inheritance(manager);
		__asm__ __volatile__("nop");
	}
	while (value);
}
