#include "LinkedList.hh"

void LinkedListImpl::insert(LinkImpl* item)
{
	item->_next = _head;
	_head = item;
}

bool LinkedListImpl::remove(LinkImpl* item)
{
	if (_head == 0) return false;
	if (_head == item)
	{
		_head = _head->_next;
		item->_next = 0;
		return true;
	}
	LinkImpl* previous = _head;
	LinkImpl* current = _head->_next;
	while (current != 0)
	{
		if (current == item)
		{
			previous->_next = current->_next;
			item->_next = 0;
			return true;
		}
		previous = current;
		current = current->_next;
	}
	return false;
}
