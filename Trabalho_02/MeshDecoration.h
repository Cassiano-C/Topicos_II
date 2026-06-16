#ifndef __MeshDecoration_h
#define __MeshDecoration_h


#include "DecorationSet.h"
#include "TriangleMesh.h"

namespace tcii::cg
{ // begin namespace tcii::cg


/////////////////////////////////////////////////////////////////////
//
// MeshDecoration: class for triangle mesh decoration
// ==============
template <typename V, typename T>
class EMPTY_BASES MeshDecoration: public SharedObject,
  public DecorationSet<V, T>
{
public:
  using VA = V;
  using TA = T;
  using Base = DecorationSet<V, T>;
  using pointer = ObjectPtr<MeshDecoration>;

  static auto New(const TriangleMesh& mesh)
  {
    return pointer{new MeshDecoration{&mesh}};
  }

  template <typename OtherV, typename OtherT>
  static auto New(MeshDecoration<OtherV, OtherT>* other)
  {
    assert(other != nullptr);
    return pointer{new MeshDecoration{std::move(*other)}};
  }

  TriangleMesh* mesh() const
  {
    return _mesh;
  }

  TriangleMesh* mesh()
  {
    return _mesh;
  }

private:
  ObjectPtr<TriangleMesh> _mesh;

  MeshDecoration(const TriangleMesh* mesh):
    Base{mesh->data().vertexCount(), mesh->data().triangleCount()},
    _mesh{mesh}
  {
    // do nothing
  }

  MeshDecoration(MeshDecoration&&) noexcept = default;

  template <typename OtherV, typename OtherT>
  MeshDecoration(MeshDecoration<OtherV, OtherT>&& other):
    Base{std::move(other)},
    _mesh{other.mesh()}
  {
    // do nothing
  }

}; // MeshDecoration

} // end namespace tcii::cg

#endif // __MeshDecoration_h
