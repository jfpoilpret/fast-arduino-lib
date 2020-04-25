//TODO add FastArduino standard header

#ifndef MOVE_HH
#define MOVE_HH

#include "types_traits.h"

//TODO doc!
namespace std
{
	template<typename T>
	constexpr typename types_traits::remove_reference<T>::type&& move(T&& t)
	{
		return static_cast<typename types_traits::remove_reference<T>::type&&>(t);
	}
}

#endif /* MOVE_HH */
