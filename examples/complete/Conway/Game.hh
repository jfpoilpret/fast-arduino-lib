#ifndef GAME_HH
#define GAME_HH

#include <avr/sfr_defs.h>

//TODO Improve templating by defining the right type for each row (uint8_t, uint16_t, uint32_t...)
// Row Type could be deduced from COLUMNS value (or conversely)
//TODO Maybe make a class to hold one generation and access its members?
template<uint8_t ROWS, uint8_t COLUMNS>
class GameOfLife
{
public:
	GameOfLife(uint8_t game[ROWS]):_current_generation{game}, _empty{}, _still{} {}
	
	void progress_game()
	{
		uint8_t next_generation[ROWS];
		for (uint8_t row = 0; row < ROWS; ++row)
			for (uint8_t col = 0; col < COLUMNS; ++col)
			{
				uint8_t count_neighbours = neighbours(row, col);
				if (count_neighbours == 3 || (count_neighbours == 4 && (_current_generation[row] & _BV(col))))
					// cell is alive
					next_generation[row] |= _BV(col);
				else
					// cell is dead
					next_generation[row] &= ~_BV(col);
			}
		// Copy next generation to current one
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
	static uint8_t neighbours_in_row(uint8_t game_row, uint8_t col)
	{
		//TODO possibly optimize by:
		// - copy row to GPIOR0
		// - rotate GPIOR (col+1) times
		// check individual bits 0, 1 and 2
		uint8_t count = (game_row & _BV(col)) ? 1 : 0;
		if (game_row & _BV(col ? col - 1 : COLUMNS - 1)) ++count;
		if (game_row & _BV(col == COLUMNS - 1 ? 0 : col + 1)) ++count;
		return count;
	}
	
	uint8_t neighbours(uint8_t row, uint8_t col)
	{
		uint8_t count = neighbours_in_row(row ? _current_generation[row - 1] : _current_generation[ROWS - 1], col);
		count += neighbours_in_row(row == ROWS - 1 ? _current_generation[0] : _current_generation[row + 1], col);
		count += neighbours_in_row(_current_generation[row], col);
		return count;
	}

	uint8_t* _current_generation;
	bool _empty;
	bool _still;
};

#endif /* GAME_HH */
