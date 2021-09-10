//   Copyright 2016-2020 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/*
 * Special check for LifeCycle and Proxy API (kind of unit tests).
 * Wiring:
 * - Arduino UNO
 *   - Standard USB to console
 * - on ATmega644 based boards:
 *   - D25 (PD1): TX output connected through USB Serial converter to console for display
 */

#include <fastarduino/tests/assertions.h>
#include <fastarduino/iomanip.h>
#include <fastarduino/flash.h>
#include <fastarduino/move.h>
#include <fastarduino/lifecycle.h>

#ifdef ARDUINO_UNO
#define HARD_UART
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static constexpr const uint8_t MAX_LC_SLOTS = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#elif defined (BREADBOARD_ATTINYX4)
#define RESTRICT_CODE
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static constexpr const uint8_t MAX_LC_SLOTS = 16;
#elif defined (BREADBOARD_ATMEGAXX4P)
#define HARD_UART
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 128;
static constexpr const uint8_t MAX_LC_SLOTS = 32;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)
#else
#error "Current target is not yet supported!"
#endif

using namespace streams;
using namespace lifecycle;
using namespace tests;

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

template<typename T> static void check(ostream& out, AbstractLifeCycleManager& manager, const T& init)
{
	out << F("Check LifeCycle management") << endl;
	{
		out << F("0. Instance creation") << endl;
		LifeCycle<T> instance{init};
		assert_equals(out, F("available_slots()"), MAX_LC_SLOTS, manager.available_());
		assert_equals(out, F("id() after construction"), 0, instance.id());

		out << F("1. Registration") << endl;
		uint8_t id = manager.register_(instance);
		assert_true(out, F("id returned by register_()"), id);
		assert_equals(out, F("id() after registration"), id, instance.id());
		assert_equals(out, F("available_slots()"), MAX_LC_SLOTS - 1, manager.available_());

		out << F("2. Find") << endl;
		LifeCycle<T>* found = manager.find_<T>(id);
		assert_true(out, F("manager.find_(id)"), found != nullptr);
		assert_equals(out, F("manager.find_(id)"), &instance, found);
		out << F("val=") << dec << found->val() << endl;

		// Check copy never compiles
		// LifeCycle<T> copy{instance};

		out << F("3. Move constructor") << endl;
		LifeCycle<T> move = std::move(instance);
		assert_equals(out, F("original id() after registration"), 0, instance.id());
		assert_equals(out, F("moved id() after registration"), id, move.id());
		assert_equals(out, F("available_slots()"), MAX_LC_SLOTS - 1, manager.available_());

		out << F("4. Find after move") << endl;
		found = manager.find_<T>(id);
		assert_true(out, F("manager.find_(id)"), found != nullptr);
		assert_equals(out, F("manager.find_(id)"), &move, found);
		out << F("val=") << dec << found->val() << endl;

		// Check copy never compiles
		// LifeCycle<T> copy2;
		// copy2 = move;

		out << F("5. Move assignment") << endl;
		LifeCycle<T> move2;
		move2 = std::move(move);
		assert_equals(out, F("original id() after registration"), 0, move.id());
		assert_equals(out, F("moved id() after registration"), id, move2.id());
		assert_equals(out, F("available_slots()"), MAX_LC_SLOTS - 1, manager.available_());
	}

	// Check destruction
	out << F("6. Destruction") << endl;
	assert_equals(out, F("available_slots()"), MAX_LC_SLOTS, manager.available_());
}

void check_light_proxies(ostream& out, AbstractLifeCycleManager& manager)
{
	out << F("Check LightProxy class") << endl;

	Value v1{10};
	SubValue v2{20, 30};

	assert_equals(out, F("sizeof LightProxy<Value>"), 2u, sizeof(LightProxy<Value>));
	LightProxy<Value> p1{v1};
	LightProxy<Value> p2{v2};
	out << F("p1()->val() ") << hex << p1() << ' ' << dec << p1()->val() << endl;
	out << F("p2()->val() ") << hex << p2() << ' ' << dec << p2()->val() << endl;

	LifeCycle<Value> lc1{v1};
	assert_equals(out, F("manager.register_(lc1)"), 1, manager.register_(lc1));
	assert_equals(out, F("lc1.id()"), 1, lc1.id());
	LifeCycle<SubValue> lc2{v2};
	assert_equals(out, F("manager.register_(lc2)"), 2, manager.register_(lc2));
	assert_equals(out, F("lc2.id()"), 2, lc2.id());

	LightProxy<Value> p3{lc1};
	out << F("p3.id=") << dec << p3.id()
		<< F(" p3.dest=") << hex << p3.destination() << endl;
	LightProxy<Value> p4{lc2};
	out << F("p4.id=") << dec << p4.id() 
		<< F(" p4.dest=") << hex << p4.destination() << endl;
	out << F("p3()->val() ") << hex << p3(&manager) << ' ' << dec << p3(&manager)->val() << endl;
	out << F("p4()->val() ") << hex << p4(&manager) << ' ' << dec << p4(&manager)->val() << endl;

	// Check LP<T> is copy-construtable from LP<T'> if T' is a T subclass
	LightProxy<SubValue> p5{lc2};

	LightProxy<Value> p6{p5};
	out << F("p6.id=") << dec << p6.id()
		<< F(" p6.dest=") << hex << p6.destination() << endl;
	out << F("p6()->val() ") << hex << p6(&manager) << ' ' << dec << p6(&manager)->val() << endl;

	// Check LP<T> is copy-assignable from LP<T'> if T' is a T subclass
	p6 = p5;
	out << F("p6.id=") << dec << p6.id()
		<< F(" p6.dest=") << hex << p6.destination() << endl;
	out << F("p6()->val() ") << hex << p6(&manager) << ' ' << dec << p6(&manager)->val() << endl;

	// This shall not compile
	// LightProxy<SubValue> p5{lc1};
}

void check_proxies(ostream& out, AbstractLifeCycleManager& manager)
{
	out << F("Check Proxy class") << endl;

	Value v1{10};
	SubValue v2{20, 30};

	assert_equals(out, F("sizeof Proxy<Value>"), 3u, sizeof(Proxy<Value>));
	Proxy<Value> p1{v1};
	Proxy<Value> p2{v2};
	out << F("p1->val() ") << hex << &(*p1) << ' ' << dec << p1->val() << endl;
	out << F("p2->val() ") << hex << &(*p2) << ' ' << dec << p2->val() << endl;

	LifeCycle<Value> lc1{v1};
	assert_equals(out, F("manager.register_(lc1)"), 1, manager.register_(lc1));
	assert_equals(out, F("lc1.id()"), 1, lc1.id());
	LifeCycle<SubValue> lc2{v2};
	assert_equals(out, F("manager.register_(lc2)"), 2, manager.register_(lc2));
	assert_equals(out, F("lc2.id()"), 2, lc2.id());

	Proxy<Value> p3{lc1};
	out << F("p3.id=") << dec << p3.id()
		<< F(" p3.manager=") << hex << p3.manager() 
		<< F(" p3.dest=") << hex << p3.destination() << endl;
	Proxy<Value> p4{lc2};
	out << F("p4.id=") << dec << p4.id() 
		<< F(" p4.manager=") << hex << p4.manager()
		<< F(" p4.dest=") << hex << p4.destination() << endl;
	out << F("p3->val() ") << hex << &(*p3) << ' ' << dec << p3->val() << endl;
	out << F("p4->val() ") << hex << &(*p4) << ' ' << dec << p4->val() << endl;

	// This shall not compile
	// Proxy<SubValue> p5{lc1};
}

void check_proxy_constructors(ostream& out, UNUSED AbstractLifeCycleManager& manager)
{
	out << F("Check Proxy constructors") << endl;
	Value v1{50};

	{
		Proxy<Value> p1;
		out << F("Proxy default constructor") << endl;
		assert_equals(out, F("p1.is_dynamic()"), false, p1.is_dynamic());
		assert_equals(out, F("p1.manager()"), (AbstractLifeCycleManager*) nullptr, p1.manager());
		assert_equals(out, F("p1.destination()"), (Value*) nullptr, p1.destination());

		p1 = Proxy<Value>{v1};
		out << F("Proxy assignment operator") << endl;
		assert_equals(out, F("p1.is_dynamic()"), false, p1.is_dynamic());
		assert_equals(out, F("p1.manager()"), (AbstractLifeCycleManager*) nullptr, p1.manager());
		assert_equals(out, F("p1.destination()"), &v1, p1.destination());
		assert_equals(out, F("p1->val()"), 50, p1->val());

		Proxy<Value> p2{p1};
		out << F("Proxy copy constructor") << endl;
		assert_equals(out, F("p2.is_dynamic()"), false, p2.is_dynamic());
		assert_equals(out, F("p2.manager()"), (AbstractLifeCycleManager*) nullptr, p2.manager());
		assert_equals(out, F("p2.destination()"), &v1, p2.destination());
		assert_equals(out, F("p2->val()"), 50, p2->val());
	}

	out << F("Check LightProxy constructors") << endl;
	{
		LightProxy<Value> p1;
		out << F("LightProxy default constructor") << endl;
		assert_equals(out, F("p1.is_dynamic()"), false, p1.is_dynamic());
		assert_equals(out, F("p1.destination()"), (Value*) nullptr, p1.destination());

		p1 = LightProxy<Value>{v1};
		out << F("LightProxy assignment operator") << endl;
		assert_equals(out, F("p1.is_dynamic()"), false, p1.is_dynamic());
		assert_equals(out, F("p1.destination()"), &v1, p1.destination());
		assert_equals(out, F("p1()->val()"), 50, p1()->val());

		LightProxy<Value> p2{p1};
		out << F("LightProxy copy constructor") << endl;
		assert_equals(out, F("p2.is_dynamic()"), false, p2.is_dynamic());
		assert_equals(out, F("p2.destination()"), &v1, p2.destination());
		assert_equals(out, F("p2()->val()"), 50, p2()->val());

		out << F("LightProxy conversion constructor from Proxy") << endl;
		Proxy<Value> p4{v1};
		LightProxy<Value> p5{p4};
		assert_equals(out, F("p5.is_dynamic()"), false, p5.is_dynamic());
		assert_equals(out, F("p5.destination()"), &v1, p5.destination());
		assert_equals(out, F("p5()->val()"), 50, p5()->val());
	}
}

ostream* Value::out_ = nullptr;

static char output_buffer[OUTPUT_BUFFER_SIZE];

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Enable interrupts at startup time
	sei();

#ifdef HARD_UART
	serial::hard::UATX<UART> uart{output_buffer};
#else
	serial::soft::UATX<TX> uart{output_buffer};
#endif
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
	assert_equals(out, F("available_slots()"), MAX_LC_SLOTS, manager.available_());

	// Check LC
	check<Value>(out, manager, VAL0);
	// Check full proxies
#ifndef RESTRICT_CODE
	check_proxies(out, manager);
#endif
	// Check light proxies
	check_light_proxies(out, manager);
	// Check other constructors
	check_proxy_constructors(out, manager);
}
