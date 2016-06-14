#ifndef LINKEDLIST_HH
#define	LINKEDLIST_HH

class LinkImpl
{
public:
	LinkImpl() __attribute__((always_inline)) : _next{0} {}
	
protected:
	LinkImpl* _next;	
	friend class LinkedListImpl;
};

class LinkedListImpl
{
public:
	LinkedListImpl() __attribute__((always_inline)) : _head{0} {}
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
	void insert(T& item) __attribute__((always_inline))
	{
		LinkedListImpl::insert(&item);
	}
	bool remove(T& item) __attribute__((always_inline))
	{
		return LinkedListImpl::remove(&item);
	}
	template<typename F> void traverse(F f)
	{
		T* current = head();
		while (current != 0)
		{
			f(*current);
			current = current->next();
		}
	}
	
private:
	T *head() __attribute__((always_inline))
	{
		return (T*) _head;
	}
};

template<typename T>
class Link: private LinkImpl
{
private:
	T *next() __attribute__((always_inline))
	{
		return (T*) _next;
	}
	friend class LinkedList<T>;
};

#endif	/* LINKEDLIST_HH */
