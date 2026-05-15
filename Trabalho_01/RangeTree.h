#ifndef __RangeTree_h
#define __RangeTree_h

#include "util/Array.h"
#include "PointTraits.h"
#include <functional>
#include <numeric>

namespace tcii::cg
{ // begin namespace tcii::cg

namespace rtree
{ // begin namespace rtree

using index_t = unsigned;
using IndexArray = Array<index_t>;

template <size_t D, typename P>
inline auto _x(const P& p)
{
  if constexpr (std::is_arithmetic_v<P>)
  {
    static_assert(D == 1);
    return p;
  }
  else
    return p[D - 1];
}

template <typename A>
using PointFunc = std::function<bool(const A&, size_t)>;

template <size_t D, typename P, typename A> class BBST;

// Expecialização para D = 1, onde a árvore é simplesmente uma BST (Binary Search Tree)
template <typename P, typename A>
class BBST<1, P, A>
{
public:
  using Bounds = typename PointTraits<P>::Bounds;
  using PointFunc = rtree::PointFunc<A>;

  // função expecializada para construir a árvore de dimensão D = 1, onde cada nó da árvore contém um índice para o ponto correspondente
  void build(const A& points)
  {
    // insert your code here
  }
 
  // função de busca para percorrer a árvore e encontrar os pontos que vão ser enviados à função f
  size_t query(const A& points, const Bounds& bounds, PointFunc f) const
  {
    // insert your code here
    return 0;
  }

}; // BBST

// Template para D > 1, onde a árvore é uma estrutura de árvore binária balanceada (BBST) com árvores associadas para as dimensões restantes
template <size_t D, typename P, typename A>
class BBST
{
public:
  using Bounds = typename PointTraits<P>::Bounds;
  using PointFunc = rtree::PointFunc<A>;

  ~BBST()
  {
    delete _root;
  }

  // função para contruir a arvore de dimenção D < 1, onde cada nó da árvore contém uma árvore associada para as dimensões restantes
  void build(const A& points)
  {
    assert(!_root); // o assert vai garantir q a arvore seja construida apenas uma vez.
    int n = points.size();
    _indices = new index_t[n];
    std::iota(_indices, _indices + n, 0); // preenche o vetor de indices ordenados com os indices dos pontos
    std::sort(_indices, _indices + n,
      [&points](int i1, int i2)
      {
        return _x<D>(points[i1]) < _x<D>(points[i2]);
      });
    _root = new Node{_x<D>(points[_indices[n / 2]]), _x<D>(points[_indices[0]]), _x<D>(points[_indices[n - 1]]), -1, 0}; // o valor de divisão é o valor da dimensão D do ponto mediano
    // Calcula o cout e o fist
    Calcula_Cout_Fist(_root,points,_indices);
  }

  // função recursiva para construir a árvore, onde cada nó da árvore contém uma árvore associada para as dimensões restantes
  Node* build(const A& points, const IndexArray& indices, int i,int n)
  {
    
  }

  void Calcula_Cout_Fist(Node* node, const A& points,const IndexArray& indices)
  {
    for(int i=0; i < n;i++)
    {
      if(_x<D>(points[indices[i]]) == _root->Split_Value)
      {
        if (_root->fist == -1) _root->fist = i; // se pegar _root->fist - 1 vai poder somar o cout com o indice do ponto atual e vai dar o numero de pontos que tem a mesma coordenada D do valor de divisão
        _root->cout++;
      }
    }
  }

  // Funçao de busca para percorere a arvore e encontar os pontos que vao ser enviados a funçao f
  size_t query(const A& points, const Bounds& bounds, PointFunc f) const
  {
    // insert your code here
    return 0;
  }

private:
  using real = typename P::value_type;
  using AssociatedTree = BBST<D - 1, P, A>;

  // definir a estrutura de nó da árvore, que deve conter um índice para o ponto correspondente, um ponteiro para a árvore associada e ponteiros para os filhos esquerdo e direito
  // definir as variaveis necessarias
  struct Node
  {
    /*
    de finir se os nos vao conter os indices da lista ordena pra aqula dimensao 
    ou se vao conter copias das listar com os indices ordenados que esta no _root
    */
    // insert your code here
    real Split_Value;
    real Min_Valure;
    real Max_Valure;
    int fist;
    int cout;
    
    Node* _childL{};
    Node* _childR{};
    AssociatedTree* _assocTree{};
    IndexArray* Conjunto_Cano;

    Node(real Split_Value, real Min_Valure, real Max_Valure, int fist, int cout):
      Split_Value{Split_Value},
      Min_Valure{Min_Valure},
      Max_Valure{Max_Valure},
      fist{fist},
      cout{cout}
    {
      // do nothing
    }

  }; // Node

  struct Conjunto_Nodal
  {
    int fist;
    int cout;
  };

  Node* _root{};
  IndexArray _indices;

}; // BBST

} // end namespace rtree

template <typename P, typename A>
class RangeTree
{
public:
  constexpr static auto D = point_dim_v<P>;

  using Bounds = typename PointTraits<P>::Bounds;
  using PointFunc = rtree::PointFunc<A>;

  RangeTree(const A& points):
    _points{points}
  {
    // do nothing
  }

  auto& points() const
  {
    return _points;
  }

  void build()
  {
    _mainTree.build(_points);
  }

  auto query(const Bounds& bounds, PointFunc f) const
  {
    return _mainTree.query(_points, bounds, f);
  }

private:
  const A& _points;
  rtree::BBST<D, P, A> _mainTree;

}; // RangeTree

} // end namespace tcii::cg

#endif // __RangeTree_h
