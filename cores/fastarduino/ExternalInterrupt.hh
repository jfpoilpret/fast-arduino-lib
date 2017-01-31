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

