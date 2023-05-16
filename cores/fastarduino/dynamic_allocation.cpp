//   Copyright 2016-2023 Jean-Francois Poilpret
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

#include "defines.h"
#include "stddef.h"

// Allows to disengage dynamic allocation prevention (requires library rebuild)
// This also requires developers to implement their own new/delete as needed!
#ifndef USE_DYNAMIC_ALLOCATION

// This code is just to prevent any kind of dynamic allocation in FastArduino programs
void* operator new(size_t) = delete;
void* operator new[](size_t) = delete;
void* operator new(size_t, void*) = delete;
void* operator new[](size_t, void*) = delete;

void operator delete(void*) = delete;
void operator delete[](void*) = delete;
void operator delete(void*, size_t) = delete;
void operator delete[](void*, size_t) = delete;
void operator delete(void*, void*) = delete;
void operator delete[](void*, void*) = delete;

#endif
