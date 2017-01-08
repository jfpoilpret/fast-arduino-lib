/*
 * Timer compilation example.
 * 
 * This example is used only for checking constexpr function that compute prescaler/OCR values for given delay.
 */

#include <fastarduino/Timer.hh>
#include <stddef.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
// Define vectors we need in the example
#elif defined (ARDUINO_MEGA)
// Define vectors we need in the example
#elif defined (BREADBOARD_ATTINYX4)
// Define vectors we need in the example
#else
#error "Current target is not yet supported!"
#endif

// Some functions to calculate ideal prescaler/OCR values for a given time period

constexpr const Board::TimerPrescaler PRESCALERS[] =
{
	Board::TimerPrescaler::NO_PRESCALING,
	Board::TimerPrescaler::DIV_8,
	Board::TimerPrescaler::DIV_64,
	Board::TimerPrescaler::DIV_256,
	Board::TimerPrescaler::DIV_1024
};

constexpr const Board::TimerPrescaler ALL_PRESCALERS[] =
{
	Board::TimerPrescaler::NO_PRESCALING,
	Board::TimerPrescaler::DIV_8,
	Board::TimerPrescaler::DIV_32,
	Board::TimerPrescaler::DIV_64,
	Board::TimerPrescaler::DIV_128,
	Board::TimerPrescaler::DIV_256,
	Board::TimerPrescaler::DIV_1024
};

constexpr uint32_t prescaler_quotient(Board::TimerPrescaler p, uint32_t us)
{
	return (F_CPU / 1000000UL * us) / _BV(uint8_t(p));
}

constexpr uint32_t prescaler_remainder(Board::TimerPrescaler p, uint32_t us)
{
	return (F_CPU / 1000000UL * us) % _BV(uint8_t(p));
}

template<uint32_t MAX>
constexpr bool prescaler_is_adequate(uint32_t quotient)
{
	return quotient > 1 and quotient < MAX;
}

template<uint32_t MAX>
constexpr Board::TimerPrescaler best_prescaler_in_2(Board::TimerPrescaler p1, Board::TimerPrescaler p2, uint32_t us)
{
	return (!prescaler_is_adequate<MAX>(prescaler_quotient(p1, us)) ? p2 :
			!prescaler_is_adequate<MAX>(prescaler_quotient(p2, us)) ? p1 :
			prescaler_remainder(p1, us) < prescaler_remainder(p2, us) ? p1 :
			prescaler_remainder(p1, us) > prescaler_remainder(p2, us) ? p2 :
			prescaler_quotient(p1, us) > prescaler_quotient(p2, us) ? p1 : p2);
}

template<uint32_t MAX>
constexpr Board::TimerPrescaler best_prescaler(const Board::TimerPrescaler* begin, const Board::TimerPrescaler* end, uint32_t us)
{
	return (begin + 1 == end ? *begin : best_prescaler_in_2<MAX>(*begin, best_prescaler<MAX>(begin + 1 , end, us), us));
}

template<uint32_t MAX, size_t N>
constexpr Board::TimerPrescaler best_prescaler(const Board::TimerPrescaler(&prescalers)[N], uint32_t us)
{
	return best_prescaler<MAX>(prescalers, prescalers + N, us);
}

template<typename TYPE>
constexpr TYPE counter(Board::TimerPrescaler prescaler, uint32_t us)
{
	return (TYPE) prescaler_quotient(prescaler, us) - 1;
}

constexpr const uint32_t MAX_8BITS = 256;
constexpr const uint32_t MAX_16BITS = 65536;

// NB: static_assert are based on F_CPU = 16MHz (they would fail for other frequencies)
constexpr const Board::TimerPrescaler PRESCALER_1US_8BITS = best_prescaler<MAX_8BITS>(PRESCALERS, 1);
constexpr const uint8_t COUNTER_1US_8BITS = counter<uint8_t>(PRESCALER_1US_8BITS, 1);
static_assert(PRESCALER_1US_8BITS == Board::TimerPrescaler::NO_PRESCALING, 
	"PRESCALER_1US_8BITS == Board::TimerPrescaler::NO_PRESCALING");
static_assert(COUNTER_1US_8BITS == 0x0F, "COUNTER_1US_8BITS == 0x0F");

constexpr const Board::TimerPrescaler PRESCALER_1US_16BITS = best_prescaler<MAX_16BITS>(PRESCALERS, 1);
constexpr const uint16_t COUNTER_1US_16BITS = counter<uint16_t>(PRESCALER_1US_16BITS, 1);
static_assert(PRESCALER_1US_16BITS == Board::TimerPrescaler::NO_PRESCALING, 
	"PRESCALER_1US_16BITS == Board::TimerPrescaler::NO_PRESCALING");
static_assert(COUNTER_1US_16BITS == 0x000F, "COUNTER_1US_16BITS == 0x000F");

constexpr const Board::TimerPrescaler PRESCALER_10US_8BITS = best_prescaler<MAX_8BITS>(PRESCALERS, 10);
static_assert(PRESCALER_10US_8BITS == Board::TimerPrescaler::NO_PRESCALING, 
	"PRESCALER_10US_8BITS == Board::TimerPrescaler::NO_PRESCALING");

constexpr const Board::TimerPrescaler PRESCALER_10US_16BITS = best_prescaler<MAX_16BITS>(PRESCALERS, 10);
static_assert(PRESCALER_10US_16BITS == Board::TimerPrescaler::NO_PRESCALING, 
	"PRESCALER_10US_16BITS == Board::TimerPrescaler::NO_PRESCALING");

constexpr const Board::TimerPrescaler PRESCALER_100US_8BITS = best_prescaler<MAX_8BITS>(PRESCALERS, 100);
static_assert(PRESCALER_100US_8BITS == Board::TimerPrescaler::DIV_8, 
	"PRESCALER_100US_8BITS == Board::TimerPrescaler::DIV_8");

constexpr const Board::TimerPrescaler PRESCALER_100US_16BITS = best_prescaler<MAX_16BITS>(PRESCALERS, 100);
static_assert(PRESCALER_100US_16BITS == Board::TimerPrescaler::NO_PRESCALING, 
	"PRESCALER_100US_16BITS == Board::TimerPrescaler::NO_PRESCALING");

constexpr const Board::TimerPrescaler PRESCALER_1MS_8BITS = best_prescaler<MAX_8BITS>(PRESCALERS, 1000);
static_assert(PRESCALER_1MS_8BITS == Board::TimerPrescaler::DIV_64, 
	"PRESCALER_1MS_8BITS == Board::TimerPrescaler::DIV_64");

constexpr const Board::TimerPrescaler PRESCALER_1MS_16BITS = best_prescaler<MAX_16BITS>(PRESCALERS, 1000);
static_assert(PRESCALER_1MS_16BITS == Board::TimerPrescaler::NO_PRESCALING, 
	"PRESCALER_1MS_16BITS == Board::TimerPrescaler::NO_PRESCALING");

constexpr const Board::TimerPrescaler PRESCALER_10MS_8BITS = best_prescaler<MAX_8BITS>(PRESCALERS, 10000);
static_assert(PRESCALER_10MS_8BITS == Board::TimerPrescaler::DIV_1024, 
	"PRESCALER_10MS_8BITS == Board::TimerPrescaler::DIV_1024");

constexpr const Board::TimerPrescaler PRESCALER_10MS_16BITS = best_prescaler<MAX_16BITS>(PRESCALERS, 10000);
static_assert(PRESCALER_10MS_16BITS == Board::TimerPrescaler::DIV_8, 
	"PRESCALER_10MS_16BITS == Board::TimerPrescaler::DIV_8");

constexpr const Board::TimerPrescaler PRESCALER_16MS_8BITS = best_prescaler<MAX_8BITS>(PRESCALERS, 160000);
static_assert(PRESCALER_16MS_8BITS == Board::TimerPrescaler::DIV_1024, 
	"PRESCALER_16MS_8BITS == Board::TimerPrescaler::DIV_1024");

int main()
{
}
