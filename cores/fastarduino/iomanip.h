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

/// @cond api

/**
 * @file
 * C++-like std::iostream facilities.
 */
#ifndef IOMANIP_H
#define IOMANIP_H

#include <ctype.h>
#include <stddef.h>
#include "ios.h"

namespace streams
{
	/// @cond notdocumented
	class setw_
	{
	public:
		template<typename FSTREAM> void operator()(FSTREAM& stream) const
		{
			stream.width(width_);
		}

	private:
		constexpr setw_(uint8_t width) : width_{width} {}
		const uint8_t width_;
		friend constexpr const setw_ setw(uint8_t width);
	};
	template<typename FSTREAM> FSTREAM& operator<<(FSTREAM& stream, const setw_ f)
	{
		f(stream);
		return stream;
	}
	template<typename FSTREAM> FSTREAM& operator>>(FSTREAM& stream, const setw_ f)
	{
		f(stream);
		return stream;
	}

	class setprecision_
	{
	public:
		template<typename FSTREAM> void operator()(FSTREAM& stream) const
		{
			stream.precision(precision_);
		}

	private:
		constexpr setprecision_(uint8_t precision) : precision_{precision} {}
		const uint8_t precision_;
		friend constexpr const setprecision_ setprecision(uint8_t precision);
	};
	template<typename FSTREAM> FSTREAM& operator<<(FSTREAM& stream, const setprecision_ f)
	{
		f(stream);
		return stream;
	}
	template<typename FSTREAM> FSTREAM& operator>>(FSTREAM& stream, const setprecision_ f)
	{
		f(stream);
		return stream;
	}

	class setfill_
	{
	public:
		template<typename FSTREAM> void operator()(FSTREAM& stream) const
		{
			stream.fill(fill_);
		}

	private:
		constexpr setfill_(char fill) : fill_{fill} {}
		const char fill_;
		friend constexpr const setfill_ setfill(char fill);
	};
	template<typename FSTREAM> FSTREAM& operator<<(FSTREAM& stream, const setfill_ f)
	{
		f(stream);
		return stream;
	}
	template<typename FSTREAM> FSTREAM& operator>>(FSTREAM& stream, const setfill_ f)
	{
		f(stream);
		return stream;
	}

	class setbase_
	{
	public:
		template<typename FSTREAM> void operator()(FSTREAM& stream) const
		{
			stream.setf(base_, ios::basefield);
		}

	private:
		constexpr setbase_(int b) : base_{b == 2 ? ios::bin : b == 8 ? ios::oct : b == 16 ? ios::hex : ios::dec} {}
		const ios::fmtflags base_;
		friend constexpr const setbase_ setbase(int base);
	};
	template<typename FSTREAM> FSTREAM& operator<<(FSTREAM& stream, const setbase_ f)
	{
		f(stream);
		return stream;
	}
	template<typename FSTREAM> FSTREAM& operator>>(FSTREAM& stream, const setbase_ f)
	{
		f(stream);
		return stream;
	}

	class setiosflags_
	{
	public:
		template<typename FSTREAM> void operator()(FSTREAM& stream) const
		{
			stream.setf(mask_);
		}

	private:
		constexpr setiosflags_(ios::fmtflags mask) : mask_{mask} {}
		const ios::fmtflags mask_;
		friend constexpr const setiosflags_ setiosflags(ios::fmtflags mask);
	};
	template<typename FSTREAM> FSTREAM& operator<<(FSTREAM& stream, const setiosflags_ f)
	{
		f(stream);
		return stream;
	}
	template<typename FSTREAM> FSTREAM& operator>>(FSTREAM& stream, const setiosflags_ f)
	{
		f(stream);
		return stream;
	}

	class resetiosflags_
	{
	public:
		template<typename FSTREAM> void operator()(FSTREAM& stream) const
		{
			stream.unsetf(mask_);
		}

	private:
		constexpr resetiosflags_(ios::fmtflags mask) : mask_{mask} {}
		const ios::fmtflags mask_;
		friend constexpr const resetiosflags_ resetiosflags(ios::fmtflags mask);
	};
	template<typename FSTREAM> FSTREAM& operator<<(FSTREAM& stream, const resetiosflags_ f)
	{
		f(stream);
		return stream;
	}
	template<typename FSTREAM> FSTREAM& operator>>(FSTREAM& stream, const resetiosflags_ f)
	{
		f(stream);
		return stream;
	}
	/// @endcond

	/**
	 * Set the field width to be used on output (and some input) operations.
	 * This method should only be used as a stream manipulator.
	 * 
	 * The folowing example displays `123` right-aligned on 10 positions, i.e.
	 * with 7 *fill* characters prepended:
	 * @code
	 * out << setw(10) << dec << right << 123 << endl;
	 * @endcode
	 * Note that `setw()` is effective only for one input or output operation,
	 * and thus must be called before each operation.
	 */
	constexpr const setw_ setw(uint8_t width)
	{
		return setw_{width};
	}

	/**
	 * Set the decimal precision to be used to format floating-point values on 
	 * output operations.
	 * This method should only be used as a stream manipulator.
	 * 
	 * The folowing example displays `123.456789` with various precision, i.e.
	 * various number of decimals afeter the decimal point:
	 * @code
	 * out << setprecision(2) << 123.456789 << endl;
	 * out << setprecision(4) << 123.456789 << endl;
	 * out << setprecision(6) << 123.456789 << endl;
	 * out << setprecision(8) << 123.456789 << endl;
	 * @endcode
	 */
	constexpr const setprecision_ setprecision(uint8_t precision)
	{
		return setprecision_{precision};
	}

	/**
	 * Set the ios::basefield to one of its possible values (ios::dec, ios::bin, 
	 * ios::oct or ios::hex) according to @p base, which must be one of 10, 2, 8 or 16.
	 * This method should only be used as a stream manipulator.
	 * 
	 * The folowing example displays `123` under all available bases:
	 * @code
	 * out << setbase(10) << 123 << endl;
	 * out << setbase(2) << 123 << endl;
	 * out << setbase(8) << 123 << endl;
	 * out << setbase(16) << 123 << endl;
	 * @endcode
	 * 
	 * Note that it is generally preferrable to use the other manipulators.
	 * @sa dec()
	 * @sa bin()
	 * @sa oct()
	 * @sa hex()
	 */
	constexpr const setbase_ setbase(int base)
	{
		return setbase_{base};
	}

	/**
	 * Set a new *fill* character.
	 * This method should only be used as a stream manipulator.
	 * 
	 * The following example displays `123` in hexadecimal form on 4 positions,
	 * padded with `0` if necessary:
	 * @code
	 * out << setfill(`0`) << setw(4) << hex << right << 123 << endl;
	 * @endcode
	 */
	constexpr const setfill_ setfill(char fill)
	{
		return setfill_{fill};
	}

	/**
	 * Set the format flags specified by @p mask.
	 * This method should only be used as a stream manipulator.
	 * 
	 * Behaves as if `ios::setf` was called with @p mask as argument, on the 
	 * stream on which it is inserted/extracted as a manipulator (it can be 
	 * inserted/extracted on input streams or output streams).
	 * See ios::fmtflags for more information on the particular flags that can
	 * be modified with this manipulator function.
	 * @sa ios::fmtflags
	 * @sa ios::setf()
	 */
	constexpr const setiosflags_ setiosflags(ios::fmtflags mask)
	{
		return setiosflags_{mask};
	}

	/**
	 * Unset the format flags specified by @p mask.
	 * This method should only be used as a stream manipulator.
	 * 
	 * Behaves as if `ios::unsetf` was called with @p mask as argument, on the 
	 * stream on which it is inserted/extracted as a manipulator (it can be 
	 * inserted/extracted on input streams or output streams).
	 * See ios::fmtflags for more information on the particular flags that can
	 * be modified with this manipulator function.
	 * @sa ios::fmtflags
	 * @sa ios::unsetf()
	 */
	constexpr const resetiosflags_ resetiosflags(ios::fmtflags mask)
	{
		return resetiosflags_{mask};
	}
}

#endif /* IOMANIP_H */
/// @endcond
