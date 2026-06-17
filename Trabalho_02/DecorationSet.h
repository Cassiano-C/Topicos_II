#ifndef __DecorationSet_h
#define __DecorationSet_h

#include "util/SoA.h"
#include <cassert>
#include <type_traits>

#ifdef _MSC_VER
#define EMPTY_BASES __declspec(empty_bases)
#else
#define EMPTY_BASES
#endif

namespace tcii::cg
{

struct Allocator
{
  template <typename T>
  static T* allocate(size_t size) { return new T[size]; }

  template <typename T>
  static void free(T* ptr) { delete[] ptr; }
};

using index_t = unsigned;

template <typename... Fields>
using ElementSoA = SoA<Allocator, index_t, Fields...>;

template <typename T>
class ElementAttributes
{
public:
  ElementAttributes(index_t size): _soa{size} { assert(size > 0); }
  
  template <size_t I>
  auto& get(index_t i) const { return _soa.template get<I>(i); }

  template <size_t I, typename Field>
  void set(index_t i, Field&& field) { _soa.template get<I>(i) = std::forward<Field>(field); }

private:
  T _soa;
};

template <size_t E, typename T>
class EMPTY_BASES ElementDecoration
{
public:
  ElementDecoration(index_t size): _attributes{size} {}
  auto& attributes() { return _attributes; }
  const auto& attributes() const { return _attributes; }
private:
  ElementAttributes<T> _attributes;
};

template <size_t N, typename... Ts>
class DecorationSet_t {};

template <size_t N, typename T, typename... Ts>
class EMPTY_BASES DecorationSet_t<N, T, Ts...>:
  public DecorationSet_t<N + 1, Ts...>,
  public ElementDecoration<N, T>
{
public:
  using Base = DecorationSet_t<N + 1, Ts...>;

  DecorationSet_t(index_t size, auto... sizes):
    Base{sizes...},
    ElementDecoration<N, T>{size}
  {}

  template <size_t E>
  friend auto& attributes(DecorationSet_t<N, T, Ts...>& ds) noexcept
  {
    if constexpr (E == N) return ds.ElementDecoration<N, T>::attributes();
    else return attributes<E>(static_cast<Base&>(ds));
  }
};

template <size_t E, size_t N, typename... Ts>
inline auto& attributes(DecorationSet_t<N, Ts...>& ds) noexcept
{
  return attributes<E>(ds);
}

template <size_t E, size_t N, typename... Ts>
inline const auto& attributes(const DecorationSet_t<N, Ts...>& ds) noexcept
{
  return attributes<E>(const_cast<DecorationSet_t<N, Ts...>&>(ds));
}

template <typename... Ts>
using DecorationSet = DecorationSet_t<0, Ts...>;

} // end namespace tcii::cg

#endif // __DecorationSet_h