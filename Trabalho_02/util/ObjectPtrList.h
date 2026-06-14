#ifndef __ObjectPtrList_h
#define __ObjectPtrList_h

#include "util/LinkedList.h"
#include "util/SharedObject.h"
#include <concepts>

namespace tcii::cg
{ // begin namespace tcii::cg

template <typename T>
concept Shared = std::derived_from<T, SharedObject>;

using ObjectPtrListBase = LinkedList<ObjectPtr<SharedObject>>;
using ObjectPtrListIteratorBase = ObjectPtrListBase::Iterator;

template <Shared T> class ObjectPtrList;

template <Shared T>
class ObjectPtrListIterator: public ObjectPtrListIteratorBase
{
public:
  T* operator *() const
  {
    return (T*)ObjectPtrListIteratorBase::operator *().get();
  }

  auto operator ->() const
  {
    return operator *();
  }

private:
  ObjectPtrListIterator(const ObjectPtrListIteratorBase& it):
    ObjectPtrListIteratorBase{it}
  {
    // do nothing
  }

  friend ObjectPtrList<T>;

}; // ObjectPtrListIterator

template <Shared T>
class ObjectPtrList: public ObjectPtrListBase
{
public:
  using Iterator = ObjectPtrListIterator<T>;

  void add(const T* node)
  {
    ObjectPtrListBase::add(node);
  }

  Iterator begin() const
  {
    return ObjectPtrListBase::begin();
  }

  Iterator end() const
  {
    return ObjectPtrListBase::end();
  }

}; // ObjectPtrList

} // end namespace tcii::cg

#endif // __ObjectPtrList_h
