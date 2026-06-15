#ifndef __HEMeshDecoration_h
#define __HEMeshDecoration_h

#include "DecorationSet.h"
#include "Mesh.h" 
#include <iostream>
#include <algorithm>

namespace tcii::cg
{ 

template <typename V, typename T, typename E>
class EMPTY_BASES HEMeshDecoration: public SharedObject,
  public DecorationSet<V, T, E> 
{
public:
  using VA = V;
  using TA = T;
  using EA = E; 
  using Base = DecorationSet<V, T, E>;
  using PA = ObjectPtr<HEMeshDecoration>;

  
  static auto novo(const Mesh& malha)
  {
    return PA{new HEMeshDecoration{&malha}};
  }


  template <typename novoV, typename novoT, typename novoE>
  static auto novo(HEMeshDecoration<novoV, novoT, novoE>* n)
  {
    assert(n != nullptr);
    return PA{new HEMeshDecoration{std::move(*n)}};
  }

  const Mesh* mesh() const
  {
    return _mesh;
  }

  Mesh* mesh()
  {
    return const_cast<Mesh*>(_mesh.get());
  }

  
  void decorarArestasDeBorda()
  {
    auto ne = _mesh->edges().size();
    for (size_t i = 0; i < ne; ++i)
    {
      bool Borda = false;
      const auto& edge = _mesh->edges()[i];
      
      if (edge.halfEdges[0] != null_index && _mesh->halfEdges()[edge.halfEdges[0]].face == null_index) Borda = true;
      if (edge.halfEdges[1] != null_index && _mesh->halfEdges()[edge.halfEdges[1]].face == null_index) Borda = true;

      // Grava no índice <2> (Arestas), Atributo <0> do SoA
      attributes<2>(*this).set(i, Borda ? 1 : 0);
    }
  }

  void printDebugArestas() const
  {
    std::cout << "\n[DEBUG SOA - ARI FILHO] Mostrando Primeiras 5 Arestas Decoradas:\n";
    auto limite = std::min(_mesh->edges().size(), size_t(5));
    for (size_t i = 0; i < limite; ++i)
    {
      int flag = get<2, 0>(*this, i);
      std::cout << " -> Aresta " << i << ": Tipo = " << (flag == 1 ? "BORDA" : "INTERNA") << "\n";
    }
  }

private:
  ObjectPtr<const Mesh> _mesh;

  HEMeshDecoration(const Mesh* malha):
    Base{malha->vertices().size(), malha->faces().size(), malha->edges().size()},
    _mesh{malha}
  {
    // do nothing
  }

  template <typename novoV, typename novoT, typename novoE>
  HEMeshDecoration(HEMeshDecoration<novoV, novoT, novoE>&& n):
    Base{std::move(n)},
    _mesh{std::move(n._mesh)}
  {
    // do nothing
  }

  HEMeshDecoration(HEMeshDecoration&&) noexcept = default;

  template <typename, typename, typename> friend class HEMeshDecoration;

}; 

} // end namespace tcii::cg

#endif // __HEMeshDecoration_h