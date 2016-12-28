#ifndef GAME_HH
#define GAME_HH

#include <avr/sfr_defs.h>

#if defined(ARDUINO_UNO)
#define HAS_TRACE 1
#include <fastarduino/streams.hh>
extern FormattedOutput<OutputBuffer> trace;
#endif

template<uint8_t ROWS_ = 8, typename ROW_TYPE_ = uint8_t>
class GameOfLife
{
public:
	using ROW_TYPE = ROW_TYPE_;
	static constexpr const uint8_t ROWS = ROWS_;
	static constexpr const uint8_t COLUMNS = sizeof(ROW_TYPE) * 8;
	
	GameOfLife(ROW_TYPE game[ROWS]):_current_generation{game}, _empty{}, _still{} {}
	
	void progress_game()
	{
#ifdef HAS_TRACE
		trace << "process_game()\n" << flush;
#endif
		ROW_TYPE next_generation[ROWS];
		// Initialize first iteration (optimization)
		ROW_TYPE previous = _current_generation[ROWS - 1];
		ROW_TYPE current = _current_generation[0];
		for (uint8_t row = 0; row < ROWS; ++row)
		{
#ifdef HAS_TRACE
			trace << "row #" << dec << row << endl << flush;
#endif
			ROW_TYPE next = (row == ROWS - 1 ? _current_generation[0] : _current_generation[row + 1]);
			ROW_TYPE ok, code;
			neighbours(current, previous, next, code, ok);
			
			// New current is based on ok and code:
			// if ok == 0, then new current cell is dead
			// if ok == 1 and code == 1 (4 neighbours including self), then new current is same as old current
			// if ok == 1 and code == 0 (3 neighbours including self), then new current is alive
			ROW_TYPE new_current = ok & ((code & current) | ~code);
			next_generation[row] = new_current; 
			// Prepare next iteration
			previous = current;
			current = next;
		}
		// Copy next generation to current one and update status flags
		_still = true;
		_empty = true;
		for (uint8_t row = 0; row < ROWS; ++row)
		{
			if (_current_generation[row] != next_generation[row])
				_still = false;
			if ((_current_generation[row] = next_generation[row]))
				_empty = false;
		}
	}
	
	inline bool is_empty()
	{
		return _empty;
	}

	inline bool is_still()
	{
		return _still;
	}

private:
	static ROW_TYPE rotate_left(ROW_TYPE input)
	{
		return (input << 1) | (input >> (COLUMNS - 1));
	}
	static ROW_TYPE rotate_right(ROW_TYPE input)
	{
		return (input >> 1) | (input << (COLUMNS - 1));
	}
	static void adder(ROW_TYPE input_a, ROW_TYPE input_b, ROW_TYPE input_carry, ROW_TYPE& output_sum, ROW_TYPE& output_carry)
	{
		// Perform bit-parallel calculation of "full adder" (A + B + carry)
		output_sum = input_a ^ input_b;
		output_carry = (output_sum & input_carry) | (input_a & input_b);
		output_sum ^= input_carry;
	}
	static void adder(ROW_TYPE input_a, ROW_TYPE input_b, ROW_TYPE& output_sum, ROW_TYPE& output_carry)
	{
		// Perform bit-parallel calculation of "half adder" (A + B))
		output_sum = input_a ^ input_b;
		output_carry = input_a & input_b;
	}
	
	static void neighbours(ROW_TYPE row1, ROW_TYPE row2, ROW_TYPE row3, ROW_TYPE& code, ROW_TYPE& ok)
	{
		ROW_TYPE count_high, count_low;
		// Perform bit-parallel calculation and update count
		// On return, count (high/low) contain the number of live cells over 3 rows, column per column [0-3]
		adder(row1, row2, row3, count_low, count_high);

		// Compute bit-parallel number of neighbours for each column
		// On return, for each column we return a code indicating:
		// - if the cell at this column has 3 neighbours (including itself)
		// - if the cell at this column has 4 neighbours (including itself)
		// - if the cell has less than 3 or more than 4 neighbours (including itself)

		// To perform bit-parallel computation we'll need to rotate copies of count bytes left and right
		// Add each column to its left and right columns into 4 bits [0-9]
		ROW_TYPE total_0, total_1, total_2, total_3, carry_0, carry_1, carry_2;
		
		adder(count_low, rotate_left(count_low), rotate_right(count_low), total_0, carry_0);
		adder(count_high, rotate_left(count_high), rotate_right(count_high), total_1, carry_1);
		// Add carries now
		adder(total_1, carry_0, total_1, carry_2);
		adder(carry_1, carry_2, total_2, total_3);

#ifdef HAS_TRACE
		trace.width(8);
		trace << "count_h = " << bin << count_high << endl << flush;
		trace << "count_l = " << bin << count_low << "\n\n" << flush;
		trace << "total_3 = " << bin << total_3 << endl << flush;
		trace << "total_2 = " << bin << total_2 << endl << flush;
		trace << "total_1 = " << bin << total_1 << endl << flush;
		trace << "total_0 = " << bin << total_0 << "\n\n" << flush;
		trace.width(0);
#endif
		// Compute result bits
		// - OK = 0 => too few or too many neighbours
		// - OK = 1 / CODE = 0 -> 3 neighbours including self
		// - OK = 1 / CODE = 1 -> 4 neighbours including self
		ok = (~total_3) & (total_1 ^ total_2) & ~(total_0 ^ total_1);
		code = ok & total_2;
#ifdef HAS_TRACE
		trace.width(8);
		trace << "ok   = " << bin << ok << endl << flush;
		trace << "code = " << bin << code << "\n\n" << flush;
		trace.width(0);
#endif
	}
	
	ROW_TYPE* _current_generation;
	bool _empty;
	bool _still;
};

#endif /* GAME_HH */
