#ifndef __Ray3_h
#define __Ray3_h

#include "Vec3.h"

namespace tcii::cg
{ // begin namespace tcii::cg

template <typename real>
class Ray3
{
public:
  Vec3<real> origin;

  Ray3(const Vec3<real>& origin, const Vec3<real>& direction):
    origin{origin}
  {
    setDirection(direction);
  }

  void setDirection(const Vec3<real>& direction)
  {
    _direction = direction.versor();
  }

  auto& direction() const
  {
    return _direction;
  }

  auto operator ()(real t) const
  {
    return origin + t * direction;
  }

private:
  Vec3<real> _direction;

}; // Ray3

template <typename T>
std::ostream&
operator <<(std::ostream& os, const Ray3<T>& r)
{
  os << 'o' << r.origin << ' ' << 'd' << r.direction();
  return os;
}

using Ray3f = Ray3<float>;

} // end namespace tcii::cg

#endif // __Ray3_h