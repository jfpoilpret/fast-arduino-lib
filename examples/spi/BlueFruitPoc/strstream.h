#include <string.h>
#include <stdlib.h>
#include <fastarduino/flash.h>

//TODO make it a 1st-class ios_base/ostreambuf work! => benefit from all manipulator!
class AbstractStringBuilder
{
public:
	template<uint8_t SIZE>
	AbstractStringBuilder(char (&buffer)[SIZE])
		: buffer_{buffer}, current_{buffer_}, size_{SIZE} {}
	AbstractStringBuilder(const AbstractStringBuilder&) = delete;
	AbstractStringBuilder& operator=(const AbstractStringBuilder&) = delete;

	AbstractStringBuilder& prefix()
	{
		prefix_ = true;
		return *this;
	}
	AbstractStringBuilder& no_prefix()
	{
		prefix_ = false;
		return *this;
	}
	AbstractStringBuilder& hex()
	{
		hex_ = true;
		return *this;
	}
	AbstractStringBuilder& dec()
	{
		hex_ = false;
		return *this;
	}
	
	AbstractStringBuilder& operator<<(const char* str)
	{
		while (*str)
			*current_++ = *str++;
		return *this;
	}
	
	AbstractStringBuilder& operator<<(const flash::FlashStorage* str)
	{
		uint16_t address = (uint16_t) str;
		while (char value = pgm_read_byte(address++))
			*current_++ = value;
		return *this;
	}

	AbstractStringBuilder& operator<<(uint16_t value)
	{
		if (hex_)
		{
			if (prefix_)
				*this << F("0x");
			
		}
		else
		{
			utoa(value, current_, 10);
			current_ += strlen(current_);
		}
		return *this;
	}
	
	
	const char* string()
	{
		// Terminate string
		*current_ = '\0';
		return buffer_;
	}

private:
	bool hex_ = false;
	bool prefix_ = false;
	char* buffer_;
	char* current_;
	uint8_t size_;
};
