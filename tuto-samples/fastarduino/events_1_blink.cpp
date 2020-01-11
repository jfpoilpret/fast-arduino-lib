#include <fastarduino/gpio.h>
#include <fastarduino/pci.h>
#include <fastarduino/events.h>
#include <fastarduino/time.h>

#define PCI_NUM 2
static constexpr const board::Port BUTTONS_PORT = board::Port::PORT_D;
static constexpr const board::DigitalPin LED = board::DigitalPin::LED;

using namespace events;

using EVENT = Event<uint8_t>;
static constexpr const uint8_t BUTTON_EVENT = Type::USER_EVENT;

// Class handling PCI interrupts and transforming them into events
class EventGenerator
{
public:
	EventGenerator(containers::Queue<EVENT>& event_queue):event_queue_{event_queue}, buttons_{0x00, 0xFF} {}

	void on_pin_change()
	{
		event_queue_.push_(EVENT{BUTTON_EVENT, buttons_.get_PIN()});
	}

private:
	containers::Queue<EVENT>& event_queue_;
	gpio::FastPort<BUTTONS_PORT> buttons_;
};

REGISTER_PCI_ISR_METHOD(PCI_NUM, EventGenerator, &EventGenerator::on_pin_change, board::InterruptPin::D0_PD0_PCI2)

void blink(uint8_t buttons)
{
	// If no button is pressed, do nothing
	if (!buttons) return;

	gpio::FAST_PIN<LED> led;
	// Buttons are plit in 2 groups of four:
	// - 1st group sets 5 iterations
	// - 2nd group sets 10 iterations
	// Note: we multiply by 2 because one blink iteration means toggling the LED twice
	uint8_t iterations = (buttons & 0x0F ? 5 : 10) * 2;
	// In each group, each buttons define the delay between LED toggles
	// - 1st/5th button: 200ms
	// - 2nd/6th button: 400ms
	// - 3rd/7th button: 800ms
	// - 4th/8th button: 1600ms
	uint16_t delay = (buttons & 0x11 ? 200 : buttons & 0x22 ? 400 : buttons & 0x44 ? 800 : 1600);
	while (iterations--)
	{
		led.toggle();
		time::delay_ms(delay);
	}
}

static const uint8_t EVENT_QUEUE_SIZE = 32;

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Prepare event queue
	EVENT buffer[EVENT_QUEUE_SIZE];
	containers::Queue<EVENT> event_queue{buffer};

	// Create and register event generator
	EventGenerator generator{event_queue};
	interrupt::register_handler(generator);

	// Setup PCI interrupts
	interrupt::PCISignal<PCI_NUM> signal;
	signal.enable_pins_(0xFF);
	signal.enable_();

	// Setup LED pin as output
	gpio::FAST_PIN<LED> led{gpio::PinMode::OUTPUT};

	// Enable interrupts at startup time
	sei();

	// Event Loop
	while (true)
	{
		EVENT event = containers::pull(event_queue);
		if (event.type() == BUTTON_EVENT)
			// Invert levels as 0 means button pushed (and we want 1 instead)
			blink(event.value() ^ 0xFF);
	}
	return 0;
}
