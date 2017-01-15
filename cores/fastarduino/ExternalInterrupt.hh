#ifndef EXTERNALINTERRUPT_HH
#define EXTERNALINTERRUPT_HH

class ExternalInterruptHandler
{
public:
	virtual bool on_pin_change() = 0;
};

class EmptyInterruptHandler: public ExternalInterruptHandler
{
public:
	virtual bool on_pin_change() override
	{
		return false;
	}
};

//TODO More functional subclasses to:
// - allow detection of PCI mode (RAISE, LOWER...)
// - allow chaining PCI handling to several handlers
//TODO this is used when we have different handlers for the same port


#endif /* EXTERNALINTERRUPT_HH */

