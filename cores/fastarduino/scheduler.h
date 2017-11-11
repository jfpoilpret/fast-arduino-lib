//   Copyright 2016-2017 Jean-Francois Poilpret
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

#ifndef SCHEDULER_HH
#define SCHEDULER_HH

#include "events.h"
#include "linked_list.h"

namespace events
{
	class Job;

	template<typename CLOCK_> class Scheduler : public EventHandler, public containers::LinkedList<Job>
	{
	public:
		using CLOCK = CLOCK_;

		Scheduler(const CLOCK& clock, uint8_t type) INLINE : EventHandler{type}, _clock(clock)
		{
		}

		virtual void on_event(UNUSED const Event& event) override INLINE
		{
			traverse(*this);
		}

		bool operator()(Job& job);

		void schedule(Job& job) INLINE
		{
			insert(job);
		}
		void unschedule(Job& job) INLINE
		{
			remove(job);
		}

	private:
		const CLOCK& _clock;
	};

	class Job : public containers::Link<Job>
	{
	public:
		bool is_periodic() const INLINE
		{
			return _period != 0;
		}
		uint32_t next_time() const INLINE
		{
			return _next_time;
		}
		uint32_t period() const INLINE
		{
			return _period;
		}
		void reschedule(uint32_t when) INLINE
		{
			_next_time = when;
		}

		virtual void on_schedule(uint32_t millis) = 0;

	protected:
		Job(uint32_t next = 0, uint32_t period = 0) INLINE : _next_time(next), _period(period)
		{
		}

	private:
		uint32_t _next_time;
		uint32_t _period;

		template<typename CLOCK> friend class Scheduler;
	};

	template<typename CLOCK> bool Scheduler<CLOCK>::operator()(Job& job)
	{
		uint32_t now = _clock.millis();
		if (job.next_time() <= now)
		{
			job.on_schedule(now);
			if (!job.is_periodic()) return true;
			job.reschedule(now + job.period());
		}
		return false;
	}
}

#endif /* SCHEDULER_HH */
