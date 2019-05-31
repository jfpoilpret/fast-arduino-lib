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

/*
 * Frequency generator example, used to play the Imperial March.
 * The melody play is stoppable by pushing a button.
 * In this example, the melody is stored in SRAM.
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - D6: connect to a 5V piezo buzzer with the othe lead connected to ground
 *   - D2: connect to a push button connected to GND
 */

// Imperial march tones thanks:
// http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March

// Example of square wave generation, using CTC mode and COM toggle
#include <fastarduino/time.h>
#include <fastarduino/devices/tone_player.h>
#include <fastarduino/gpio.h>
#include <fastarduino/int.h>

// Board-dependent settings
// static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
// static constexpr const board::DigitalPin OUTPUT = board::PWMPin::D9_PB1_OC1A;
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
static constexpr const board::PWMPin OUTPUT = board::PWMPin::D6_PD6_OC0A;
static constexpr const board::ExternalInterruptPin STOP = board::ExternalInterruptPin::D2_PD2_EXT0;

using devices::audio::Tone;
using namespace devices::audio::SpecialTone;
using GENERATOR = devices::audio::ToneGenerator<NTIMER, OUTPUT>;
using PLAYER = devices::audio::TonePlayer<NTIMER, OUTPUT>;
using QTONEPLAY = PLAYER::QTonePlay;

static QTONEPLAY music[] =
{
	// First part
	QTONEPLAY{Tone::A1, 500},
	QTONEPLAY{Tone::A1, 500},
	QTONEPLAY{Tone::A1, 500},
	QTONEPLAY{Tone::F1, 350},
	QTONEPLAY{Tone::C2, 150},
	QTONEPLAY{Tone::A1, 500},
	QTONEPLAY{Tone::F1, 350},
	QTONEPLAY{Tone::C2, 150},
	QTONEPLAY{Tone::A1, 650},
	QTONEPLAY{Tone::SILENCE, 150},

	// Second part
	QTONEPLAY{Tone::E2, 500},
	QTONEPLAY{Tone::E2, 500},
	QTONEPLAY{Tone::E2, 500},
	QTONEPLAY{Tone::F2, 350},
	QTONEPLAY{Tone::C2, 150},
	QTONEPLAY{Tone::Gs1, 500},
	QTONEPLAY{Tone::F1, 350},
	QTONEPLAY{Tone::C2, 150},
	QTONEPLAY{Tone::A1, 650},
	QTONEPLAY{Tone::SILENCE, 150},

	// Third part (repeated once)
	QTONEPLAY{REPEAT_START},
	QTONEPLAY{Tone::A2, 500},
	QTONEPLAY{Tone::A1, 300},
	QTONEPLAY{Tone::A1, 150},
	QTONEPLAY{Tone::A2, 400},
	QTONEPLAY{Tone::Gs2, 200},
	QTONEPLAY{Tone::G2, 200},
	QTONEPLAY{Tone::Fs2, 125},
	QTONEPLAY{Tone::F2, 125},
	QTONEPLAY{Tone::Fs2, 250},
	QTONEPLAY{Tone::SILENCE, 250},

	QTONEPLAY{Tone::As1, 250},
	QTONEPLAY{Tone::Ds2, 400},
	QTONEPLAY{Tone::D2, 200},
	QTONEPLAY{Tone::Cs2, 200},
	QTONEPLAY{Tone::C2, 125},
	QTONEPLAY{Tone::B1, 125},
	QTONEPLAY{Tone::C2, 250},
	QTONEPLAY{Tone::SILENCE, 250},

	QTONEPLAY{Tone::F1, 125},
	QTONEPLAY{Tone::Gs1, 500},
	QTONEPLAY{Tone::F1, 375},
	QTONEPLAY{Tone::A1, 125},
	QTONEPLAY{Tone::C2, 500},
	QTONEPLAY{Tone::A1, 375},
	QTONEPLAY{Tone::C2, 125},
	QTONEPLAY{Tone::E2, 650},
	QTONEPLAY{Tone::SILENCE, 250},
	QTONEPLAY{REPEAT_END, 1},

	QTONEPLAY{END, 0}
};

class PlayerStop
{
public:
	PlayerStop(PLAYER& player):player_{player}, stop_{gpio::PinMode::INPUT_PULLUP}
	{
		interrupt::register_handler(*this);
	}

private:
	void pin_change()
	{
		if (!stop_.value()) player_.stop();
	}

	PLAYER& player_;
	gpio::FastPinType<board::EXT_PIN<STOP>()>::TYPE stop_;

	DECL_INT_ISR_HANDLERS_FRIEND
};

REGISTER_INT_ISR_METHOD(0, STOP, PlayerStop, &PlayerStop::pin_change)

int main() __attribute__((OS_main));
int main()
{
	sei();
	GENERATOR generator;
	PLAYER player{generator};
	PlayerStop stop_handler{player};
	interrupt::INTSignal<STOP> signal{interrupt::InterruptTrigger::FALLING_EDGE};
	time::delay_ms(5000);

	signal.enable();
	player.play(music);
}
