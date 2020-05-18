//   Copyright 2016-2020 Jean-Francois Poilpret
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

#include "future.h"

namespace future
{
	/// @cond notdocumented
	// Static definition for AbstractFutureManager singleton
	AbstractFutureManager* AbstractFutureManager::instance_ = nullptr;

	bool AbstractFutureManager::register_future_(AbstractFuture& future)
	{
		// You cannot register an already registered future
		if (future.id() != 0)
			return false;
		// Optimization: we start search AFTER the last removed id
		for (uint8_t i = last_removed_id_; i < size_; ++i)
			if (register_at_index_(future, i))
				return true;
		for (uint8_t i = 0; i <= last_removed_id_; ++i)
			if (register_at_index_(future, i))
				return true;
		return false;
	}

	uint8_t AbstractFutureManager::get_future_value_size_(uint8_t id) const
	{
		AbstractFuture* future = find_future(id);
		if (future == nullptr)
			return 0;
		return future->get_output_size_();
	}

	bool AbstractFutureManager::set_future_finish_(uint8_t id) const
	{
		AbstractFuture* future = find_future(id);
		if (future == nullptr)
			return false;
		return future->set_finish_();
	}
	
	bool AbstractFutureManager::set_future_value_(uint8_t id, uint8_t chunk) const
	{
		AbstractFuture* future = find_future(id);
		if (future == nullptr)
			return false;
		return future->set_chunk_(chunk);
	}

	bool AbstractFutureManager::set_future_value_(uint8_t id, const uint8_t* chunk, uint8_t size) const
	{
		AbstractFuture* future = find_future(id);
		if (future == nullptr)
			return false;
		return future->set_chunk_(chunk, size);
	}

	bool AbstractFutureManager::set_future_error_(uint8_t id, int error) const
	{
		AbstractFuture* future = find_future(id);
		if (future == nullptr)
			return false;
		return future->set_error_(error);
	}

	uint8_t AbstractFutureManager::get_storage_value_size_(uint8_t id) const
	{
		AbstractFuture* future = find_future(id);
		if (future == nullptr)
			return 0;
		return future->get_input_size_();
	}

	bool AbstractFutureManager::get_storage_value_(uint8_t id, uint8_t& chunk) const
	{
		AbstractFuture* future = find_future(id);
		if (future == nullptr)
			return false;
		return future->get_chunk_(chunk);
	}

	bool AbstractFutureManager::get_storage_value_(uint8_t id, uint8_t* chunk, uint8_t size) const
	{
		AbstractFuture* future = find_future(id);
		if (future == nullptr)
			return false;
		return future->get_chunk_(chunk, size);
	}

	bool AbstractFutureManager::register_at_index_(AbstractFuture& future, uint8_t index)
	{
		if (futures_[index] != nullptr)
			return false;
		update_future_(future.id_, &future, nullptr);
		future.id_ = static_cast<uint8_t>(index + 1);
		future.status_ = FutureStatus::NOT_READY;
		futures_[index] = &future;
		return true;
	}
	/// @endcond
}