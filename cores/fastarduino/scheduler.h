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

/// @cond api

/**
 * @file
 * Support for jobs scheduling.
 */
#ifndef SCHEDULER_HH
#define SCHEDULER_HH

#include "events.h"
#include "linked_list.h"

namespace events
{
	class Job;

	/**
	 * Schedule jobs at predefined periods of time.
	 * The timebase is provided by @p CLOCK @p clock instance.
	 * A scheduler is an `EventHandler` that must thus be attached to a `Dispatcher`
	 * as in this snippet:
	 * @code
	 * using namespace events;
	 * using EVENT = Event<void>;
	 * 
	 * REGISTER_WATCHDOG_CLOCK_ISR(EVENT)
	 * 
	 * class MyJob: public Job
	 * {
	 *     ...
	 * }
	 * 
	 * int main()
	 * {
	 *     // Create event queue
	 *     const uint8_t EVENT_QUEUE_SIZE = 32;
	 *     EVENT buffer[EVENT_QUEUE_SIZE];
	 *     containers::Queue<EVENT> event_queue{buffer};
	 * 
	 *     // Prepare event dispatcher, clock and scheduler
	 * 	   Dispatcher<EVENT> dispatcher;
	 *     watchdog::Watchdog<EVENT> watchdog{event_queue};
	 *     watchdog.register_watchdog_handler();
	 *     Scheduler<watchdog::Watchdog<EVENT>, EVENT> scheduler{watchdog, Type::WDT_TIMER};
	 *     dispatcher.insert(scheduler);
	 * 
	 *     // Create and register a job
	 *     MyJob job;
	 *     scheduler.schedule(job);
	 * 
	 *     // Start clock (watchdog)
	 *     watchdog.begin(watchdog::WatchdogSignal::TimeOut::TO_64ms);
	 * 
	 *     // Main event loop
	 *     while (true)
	 *     {
	 *         EVENT event = pull(event_queue);
	 *         dispatcher.dispatch(event);
	 *     }
	 * }
	 * @endcode
	 * In that snippet, we use `Watchdog` as the clock source, but other sources are 
	 * available.
	 * 
	 * @tparam CLOCK_ the type of @p clock that will be used as time base
	 * @tparam EVENT_ the `events::Event<T>` dispatched by the system
	 * 
	 * @sa Job
	 * @sa Event
	 * @sa Dispatcher
	 * @sa watchdog::Watchdog
	 * @sa time::RTT
	 */
	template<typename CLOCK_, typename EVENT_> class Scheduler : public EventHandler<EVENT_>, public containers::LinkedList<Job>
	{
	public:
		/** The type of @p clock source used by this Scheduler. */
		using CLOCK = CLOCK_;
		/** The `events::Event<T>` dispatched by the system and expected by this Scheduler. */
		using EVENT = EVENT_;

		//TODO DOC
		Scheduler(const CLOCK& clock, uint8_t type) INLINE : EventHandler<EVENT>{type}, clock_{clock}
		{
		}

		/// @cond notdocumented
		virtual void on_event(UNUSED const EVENT& event) override INLINE
		{
			traverse(*this);
		}

		bool operator()(Job& job);
		/// @endcond

		//TODO DOC
		void schedule(Job& job) INLINE
		{
			insert(job);
		}
		//TODO DOC
		void unschedule(Job& job) INLINE
		{
			remove(job);
		}

	private:
		const CLOCK& clock_;
	};

	//TODO DOC
	class Job : public containers::Link<Job>
	{
	public:
		//TODO DOC
		bool is_periodic() const INLINE
		{
			return period_ != 0;
		}

		//TODO DOC
		uint32_t next_time() const INLINE
		{
			return next_time_;
		}

		//TODO DOC
		uint32_t period() const INLINE
		{
			return period_;
		}

		//TODO DOC
		void reschedule(uint32_t when) INLINE
		{
			next_time_ = when;
		}

		//TODO DOC
		virtual void on_schedule(uint32_t millis) = 0;

	protected:
		//TODO DOC
		Job(uint32_t next = 0, uint32_t period = 0) INLINE : next_time_{next}, period_{period}
		{
		}

	private:
		uint32_t next_time_;
		uint32_t period_;

		template<typename CLOCK, typename T> friend class Scheduler;
	};

	template<typename CLOCK, typename T> bool Scheduler<CLOCK, T>::operator()(Job& job)
	{
		uint32_t now = clock_.millis();
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
/// @endcond
