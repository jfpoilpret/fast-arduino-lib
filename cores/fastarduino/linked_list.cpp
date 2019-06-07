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

#include "linked_list.h"

namespace containers
{
	/// @cond notdocumented
	void LinkedListImpl::insert(LinkImpl* item)
	{
		item->next_ = head_;
		head_ = item;
	}

	bool LinkedListImpl::remove(LinkImpl* item)
	{
		if (head_ == nullptr) return false;
		if (head_ == item)
		{
			head_ = head_->next_;
			item->next_ = nullptr;
			return true;
		}
		LinkImpl* previous = head_;
		LinkImpl* current = head_->next_;
		while (current != nullptr)
		{
			if (current == item)
			{
				previous->next_ = current->next_;
				item->next_ = nullptr;
				return true;
			}
			previous = current;
			current = current->next_;
		}
		return false;
	}
	/// @endcond
}
