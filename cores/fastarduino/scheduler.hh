#ifndef SCHEDULER_HH
#define	SCHEDULER_HH

#include "Events.hh"
#include "LinkedList.hh"

using namespace Events;

class AbstractJob;

template<typename CLOCK>
class SchedulerHandler: public LinkedList<AbstractJob>
{
public:
	SchedulerHandler(const CLOCK& clock) INLINE: _clock(clock) {}
	void operator()(const Event& event UNUSED) INLINE
	{
		traverse(*this);
	}
	bool operator()(AbstractJob& job);
	
private:
	const CLOCK& _clock;
};

template<typename CLOCK>
class Scheduler: public FunctorHandler<SchedulerHandler<CLOCK>>
{
public:
	Scheduler(const CLOCK& clock, uint8_t type) INLINE
		:FunctorHandler<SchedulerHandler<CLOCK>>{type, SchedulerHandler<CLOCK>{clock}} {}
	void schedule(AbstractJob& job) INLINE
	{
		this->functor().insert(job);
	}
	void unschedule(AbstractJob& job) INLINE
	{
		this->functor().remove(job);
	}
};

class AbstractJob: public Link<AbstractJob>
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

	//TODO shouldn't this method be private only?
	void handle(uint32_t millis) INLINE
	{
		_f(_env, millis);
	}

private:
	//TODO refactor to make it used everywhere we need this pattern!
	typedef void (*F)(void* env, uint32_t millis);
	AbstractJob(uint32_t next = 0, uint32_t period = 0, void* env = 0, F f = 0) INLINE
		:_next_time(next), _period(period), _f{f}, _env{env} {}

	uint32_t _next_time;
	uint32_t _period;
	F _f;
	void* _env;
	
	template<typename CLOCK> friend class Scheduler;
	friend class VirtualJob;
	template<typename FUNCTOR> friend class FunctorJob;
};

// Derive this class to define jobs based on a virtual method.
class VirtualJob: public AbstractJob
{
protected:
	VirtualJob() INLINE : AbstractJob{} {}
	VirtualJob(uint32_t next, uint32_t period) INLINE
		: AbstractJob{next, period, this, apply} {}
	virtual void execute(uint32_t millis) = 0;

private:
	static void apply(void* env, uint32_t millis) INLINE
	{
		((VirtualJob*) env)->execute(millis);
	}
};
	
// Instantiate this template with a Functor when a functor is applicable.
// FUNCTOR must be a class defining:
// void operator()(uint32_t millis);
// This approach generally gives smaller code and data than VirtualJob approach
template<typename FUNCTOR>
class FunctorJob: public AbstractJob
{
public:
	FunctorJob() INLINE : AbstractJob{} {}
	FunctorJob(uint32_t next, uint32_t period, FUNCTOR f) INLINE
		: AbstractJob{next, period, this, apply}, _f{f} {}
private:
	static void apply(void* env, uint32_t millis) INLINE
	{
		((FunctorJob<FUNCTOR>*) env)->_f(millis);
	}
	FUNCTOR _f;
};

template<typename CLOCK>
bool SchedulerHandler<CLOCK>::operator()(AbstractJob& job)
{
	uint32_t now = _clock.millis();
	if (job.next_time() <= now)
	{
		job.handle(now);
		if (!job.is_periodic())
			return true;
		job.reschedule(now + job.period());
	}
	return false;
}

#endif	/* SCHEDULER_HH */
