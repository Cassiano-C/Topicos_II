#ifndef __LinkedList_h
#define __LinkedList_h

#include "util/Allocator.h"
#include <cassert>

#define _USE_ALLOCATOR

namespace tcii::cg
{ // begin namespace tcii::cg

//
// Forward definitions
//
template <typename T> class LinkedList;
template <typename T> class LinkedListIterator;

template <typename T>
class LinkedListNode
{
private:
  LinkedListNode* _next{};
  T _value;

  LinkedListNode(const T& value):
    _value{value}
  {
    // do nothing
  }

  friend LinkedList<T>;
  friend LinkedListIterator<T>;
  friend Allocator;

}; // LinkedListNode

template <typename T>
class LinkedListIterator
{
public:
  using Iterator = LinkedListIterator<T>;
  using List = LinkedList<T>;

  LinkedListIterator(const List* list, const LinkedListNode<T>* node):
    _list{list},
    _node{node}
  {
    // do nothing
  }

  auto operator ==(const Iterator& other) const
  {
#ifdef _DEBUG
    if (_list != other._list)
      return false;
#endif // _DEBUG
    return _node == other._node;
  }

  auto operator !=(const Iterator& other) const
  {
    return !operator ==(other);
  }

  auto& operator ++()
  {
    assert(_node != nullptr);
    _node = _node->_next;
    return *this;
  }

  auto operator ++(int) // post-increment
  {
    auto temp = *this;

    operator ++();
    return temp;
  }

  auto& operator *() const
  {
    assert(_node != nullptr);
    return _node->_value;
  }

  auto& operator ->() const
  {
    return operator *();
  }

private:
  const List* _list;
  const LinkedListNode<T>* _node;

}; // LinkedListIterator

template <typename T>
using IterFunc = std::function<void(const T&)>;

template <typename T>
class LinkedList
{
public:
  using Iterator = LinkedListIterator<T>;

  ~LinkedList()
  {
    clear();
  }

  LinkedList() = default;

  auto size() const
  {
    return _size;
  }

  void add(const T&);
  void clear();

  auto empty() const
  {
    return _size == 0;
  }

  void iterate(IterFunc<T> func) const;

  auto begin() const
  {
    return Iterator{this, _head};
  }

  auto end() const
  {
    return Iterator{this, nullptr};
  }

private:
  LinkedListNode<T>* _head{};
  LinkedListNode<T>* _tail{};
  unsigned _size{};

}; // LinkedList

template <typename T>
void
LinkedList<T>::add(const T& value)
{
#ifdef _USE_ALLOCATOR
  auto node = Allocator::New<LinkedListNode<T>>(value);
#else
  auto node = new LinkedListNode<T>{value};
#endif // _USE_ALLOCATOR

  if (!_head)
    _head = node;
  else
    _tail->_next = node;
  _tail = node;
  _size++;
}

template <typename T>
void
LinkedList<T>::clear()
{
  while (_head)
  {
    auto temp = _head;

    _head = temp->_next;
#ifdef _USE_ALLOCATOR
    Allocator::Delete(temp);
#else
    delete temp;
#endif // _USE_ALLOCATOR
  }
  _size = 0;
}

template <typename T>
void
LinkedList<T>::iterate(IterFunc<T> func) const
{
  for (auto node = _head; node; node = node->_next)
    func(node->_value);
}

} // end namespace tcii::cg

#endif // __LinkedList_h
