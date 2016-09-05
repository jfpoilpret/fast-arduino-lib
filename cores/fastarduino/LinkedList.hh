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
