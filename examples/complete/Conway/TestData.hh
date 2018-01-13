//   Copyright 2016-2018 Jean-Francois Poilpret
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

#ifndef TESTDATA_HH
#define TESTDATA_HH

#include <stdint.h>

//static uint8_t data[] =
//{
//	0B00111100,
//	0B01100110,
//	0B10000001,
//	0B10100101,
//	0B10000001,
//	0B10011001,
//	0B01100110,
//	0B00111100
//};

// Game of Life paterns example
// Oscillator #1 OK
//static uint8_t data[] =
//{
//	0B00000000,
//	0B00000000,
//	0B00010000,
//	0B00010000,
//	0B00010000,
//	0B00000000,
//	0B00000000,
//	0B00000000
//};
// Oscillator #2 OK
//static uint8_t data[] =
//{
//	0B00000000,
//	0B00000000,
//	0B00000000,
//	0B00011100,
//	0B00111000,
//	0B00000000,
//	0B00000000,
//	0B00000000
//};
// Oscillator #3 OK
//static uint8_t data[] =
//{
//	0B00000000,
//	0B00000000,
//	0B00110000,
//	0B00110000,
//	0B00001100,
//	0B00001100,
//	0B00000000,
//	0B00000000
//};
// Still #1 OK
//static uint8_t data[] =
//{
//	0B00000000,
//	0B00000000,
//	0B00000000,
//	0B00011000,
//	0B00100100,
//	0B00011000,
//	0B00000000,
//	0B00000000
//};
// Glider #1 OK
static uint8_t data[] =
{
	0B01000000,
	0B00100000,
	0B11100000,
	0B00000000,
	0B00000000,
	0B00000000,
	0B00000000,
	0B00000000
};

#endif /* TESTDATA_HH */
