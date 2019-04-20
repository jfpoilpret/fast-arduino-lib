//   Copyright 2016-2019 Jean-Francois Poilpret
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

#include <fastarduino/time.h>
#include <fastarduino/i2c_device.h>
#include <fastarduino/uart.h>

// Define vectors we need in the example
REGISTER_UATX_ISR(0)

static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// UART buffer for traces
static char output_buffer[OUTPUT_BUFFER_SIZE];

// Subclass I2CDevice to make protected methods available
class PublicDevice: public i2c::I2CDevice<i2c::I2CMode::Standard>
{
public:
	PublicDevice(MANAGER& manager): i2c::I2CDevice<i2c::I2CMode::Standard>{manager} {}
	friend int main();
};

// MCP23017 specific
const uint8_t DEVICE_ADDRESS = 0x20 << 1;
// All registers addresses (in BANK 0 mode only)
constexpr const uint8_t IODIR_A = 0x00;
constexpr const uint8_t IODIR_B = 0x01;
constexpr const uint8_t IPOL_A = 0x02;
constexpr const uint8_t IPOL_B = 0x03;
constexpr const uint8_t GPINTEN_A = 0x04;
constexpr const uint8_t GPINTEN_B = 0x05;
constexpr const uint8_t DEFVAL_A = 0x06;
constexpr const uint8_t DEFVAL_B = 0x07;
constexpr const uint8_t INTCON_A = 0x08;
constexpr const uint8_t INTCON_B = 0x09;
constexpr const uint8_t IOCON = 0x0A;
constexpr const uint8_t GPPU_A = 0x0C;
constexpr const uint8_t GPPU_B = 0x0D;
constexpr const uint8_t INTF_A = 0x0E;
constexpr const uint8_t INTF_B = 0x0F;
constexpr const uint8_t INTCAP_A = 0x10;
constexpr const uint8_t INTCAP_B = 0x11;
constexpr const uint8_t GPIO_A = 0x12;
constexpr const uint8_t GPIO_B = 0x13;
constexpr const uint8_t OLAT_A = 0x14;
constexpr const uint8_t OLAT_B = 0x15;
// IOCON bits (not all are used in this implementation)
constexpr const uint8_t IOCON_BANK = _BV(7);
constexpr const uint8_t IOCON_MIRROR = _BV(6);
constexpr const uint8_t IOCON_SEQOP = _BV(5);
constexpr const uint8_t IOCON_DISSLW = _BV(4);
constexpr const uint8_t IOCON_HAEN = _BV(3);
constexpr const uint8_t IOCON_ODR = _BV(2);
constexpr const uint8_t IOCON_INTPOL = _BV(1);

using namespace streams;

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();
	
	serial::hard::UATX<UART> uart{output_buffer};
	uart.register_handler();
	uart.begin(115200);
	ostream out = uart.out();
	out.width(0);
	out.setf(ios::hex, ios::basefield);
	out << "Start" << endl;
	
	// Start TWI interface
	i2c::I2CManager<i2c::I2CMode::Standard> manager;
	manager.begin();
	out << "I2C interface started" << endl;
	out << "status #1 " << manager.status() << endl;
	time::delay_ms(1000);
	
	PublicDevice rtc{manager};
	
	// Initialize IOCON
	rtc.write(DEVICE_ADDRESS, IOCON, i2c::BusConditions::START_NO_STOP);
	rtc.write(DEVICE_ADDRESS, 0x00, i2c::BusConditions::NO_START_STOP);
	out << "status #2 " << manager.status() << endl;
	time::delay_ms(2000);

	// Read IOCON
	uint8_t data;
	rtc.write(DEVICE_ADDRESS, IOCON, i2c::BusConditions::START_NO_STOP);
	out << "status #3 " << manager.status() << endl;
	rtc.read(DEVICE_ADDRESS, data, i2c::BusConditions::REPEAT_START_STOP);
	out << "status #4 " << manager.status() << endl;
	out	<< "IOCON: " << data << endl;

	//TODO More tests: set GPIO direction, set GPIO value...
	rtc.write(DEVICE_ADDRESS, IODIR_A, i2c::BusConditions::START_NO_STOP);
	rtc.write(DEVICE_ADDRESS, 0x00, i2c::BusConditions::NO_START_STOP);
	out << "status #5 " << manager.status() << endl;

	rtc.write(DEVICE_ADDRESS, GPIO_A, i2c::BusConditions::START_NO_STOP);
	rtc.write(DEVICE_ADDRESS, 0x11, i2c::BusConditions::NO_START_STOP);
	out << "status #6 " << manager.status() << endl;

	// Stop TWI interface
	manager.end();
	out << "status #7 " << manager.status() << endl;
	out << "End" << endl;
}
