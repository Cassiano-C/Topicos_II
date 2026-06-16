#ifndef __NameableObject_h
#define __NameableObject_h

#include <string>

namespace tcii::cg
{ // begin namespace tcii::cg

class NameableObject
{
public:
  auto name() const
  {
    return _name.c_str();
  }

  void setName(const char* format, ...);

protected:
  NameableObject() = default;

  NameableObject(const char* name):
    _name{name}
  {
    // do nothing
  }

private:
  std::string _name;

}; // NameableObject

} // end namespace tcii::cg

#endif // __NameableObject_h
