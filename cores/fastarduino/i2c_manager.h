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

#ifndef I2C_MANAGER_HH
#define I2C_MANAGER_HH

#include "i2c.h"
#include "i2c_handler.h"

namespace i2c
{
	template<I2CMode MODE_ = I2CMode::Standard> class I2CManager
	{
	public:
		static constexpr const I2CMode MODE = MODE_;

		I2CManager(I2C_STATUS_HOOK hook = 0) : handler_{hook}
		{
		}

		void begin() INLINE
		{
			handler_.begin();
		}
		void end() INLINE
		{
			handler_.end();
		}
		uint8_t status() const
		{
			return handler_.status();
		}

	private:
		using HANDLER = I2CHandler<MODE>;

		HANDLER& handler() INLINE
		{
			return handler_;
		}

		HANDLER handler_;
		template<I2CMode M> friend class I2CDevice;
	};
};

#endif /* I2C_MANAGER_HH */
