#ifndef __Allocator_h
#define __Allocator_h

#include <functional>
#include <map>
#include <utility>
#include <cstdio>

namespace tcii::cg
{ // begin namespace tcii::cg

class Allocator
{
public:
  struct TypeInfo
  {
    size_t size;
    const char* name;

  }; // TypeInfo

  using DebugFunc = std::function<void(const void*, const TypeInfo&)>;

  static auto usedMemory()
  {
    return _usedMemory;
  }

  static void debug(DebugFunc func)
  {
    for (const auto& e : _memoryMap)
      func(e.first, e.second);
  }

  template <typename T, typename... Args>
  static auto New(Args&&... args)
  {
    auto ptr = new T(std::forward<Args>(args)...);
    auto typeInfo = TypeInfo{sizeof(T), typeid(T).name()};

    _usedMemory += typeInfo.size;
    _memoryMap[ptr] = typeInfo;
#ifdef _DEBUG
#ifdef _DEBUG_ALLOCATOR
    printf("**Allocating %s (%llu B)\n", typeInfo.name, typeInfo.size);
#endif // _DEBUG_ALLOCATOR
#endif // _DEBUG
    return ptr;
  }

  template <typename T>
  static bool Delete(T* ptr)
  {
    if (auto e = _memoryMap.find(ptr); e != _memoryMap.end())
    {
      _usedMemory -= e->second.size;
#ifdef _DEBUG
#ifdef _DEBUG_ALLOCATOR
      printf("**Freeing %s (%llu B)\n", e->second.name, e->second.size);
#endif // _DEBUG_ALLOCATOR
#endif // _DEBUG
      _memoryMap.erase(e);
      delete ptr;
      return true;
    }
    return false;
  }

private:
  inline static size_t _usedMemory;
  inline static std::map<const void*, TypeInfo> _memoryMap;

}; // Allocator

inline void
printUsedMemory()
{
  printf("Used Memory: %zu bytes\n", Allocator::usedMemory());
}

inline void
printMemoryMap()
{
  if (Allocator::usedMemory())
  {
    puts("       Ptr       | Size (B) | Type Name");
    Allocator::debug([](auto ptr, const auto& typeInfo)
      {
        printf("%p | %08zu | %s\n", ptr, typeInfo.size, typeInfo.name);
      });
  }
}

} // end namespace tcii::cg

#endif // __Allocator_h
