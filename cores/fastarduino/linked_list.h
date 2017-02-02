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

#ifndef LINKEDLIST_HH
#define	LINKEDLIST_HH

#include "utilities.hh"

class LinkImpl
{
public:
	LinkImpl() INLINE : _next{0} {}
	
protected:
	LinkImpl* _next;	
	friend class LinkedListImpl;
};

class LinkedListImpl
{
public:
	LinkedListImpl() INLINE : _head{0} {}
	void insert(LinkImpl* item);
	bool remove(LinkImpl* item);
	template<typename F> void traverse(F f);
	
protected:
	LinkImpl* _head;
};

template<typename T>
class LinkedList: private LinkedListImpl
{
public:	
	void insert(T& item) INLINE
	{
		LinkedListImpl::insert(&item);
	}
	bool remove(T& item) INLINE
	{
		return LinkedListImpl::remove(&item);
	}
	template<typename F> void traverse(F f)
	{
		T* current = head();
		while (current != 0)
		{
			T* next = current->next();
			if (f(*current))
				remove(*current);
			current = next;
		}
	}
	
private:
	T *head() INLINE
	{
		return (T*) _head;
	}
};

template<typename T>
class Link: private LinkImpl
{
private:
	T *next() INLINE
	{
		return (T*) _next;
	}
	friend class LinkedList<T>;
};

#endif	/* LINKEDLIST_HH */
