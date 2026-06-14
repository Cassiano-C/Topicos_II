#ifndef __Exception_h
#define __Exception_h

#include <cstdarg>
#include <stdexcept>
#include <string>

namespace tcii::cg
{ // begin namespace tcii::cg

std::string errorMessage(const char*, va_list);

template <typename E = std::runtime_error>
inline void
error(const char* format...)
{
  va_list args;

  va_start(args, format);
  throw E{errorMessage(format, args)};
}

std::logic_error badInvocation(const char*, const char*);

} // end namespace tcii::cg

#endif // __Exception_h
