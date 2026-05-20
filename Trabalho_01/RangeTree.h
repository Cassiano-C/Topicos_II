#ifndef __RangeTree_h
#define __RangeTree_h

#include "util/Array.h"
#include "PointTraits.h"
#include <functional>
#include <numeric>

namespace tcii::cg
{ // begin namespace tcii::cg

namespace rtree
{
 // begin namespace rtree
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

  // função para contruir a arvore de dimenção D > 1, onde cada nó da árvore contém uma árvore associada para as dimensões restantes
  void build(const A& points,index_t* indices,int n)
  {
    assert(!_root); // o assert vai garantir q a arvore seja construida apenas uma vez.
    _indices = new index_t[n];
    std::iota(_indices, _indices + n, 0); // preenche o vetor de indices ordenados com os indices dos pontos
    
    if(indices != nullptr && n > 0)
    {
      _indices = indices;
    }
    
    std::sort(_indices->begin(), _indices->end(),
      [&points](int i1, int i2)
      {
        return _x<D>(points[i1]) < _x<D>(points[i2]);
      });
    
    int i = n / 2;
    _root = build_recursivo(points, nullptr, 0, n);

    print_tree();
  }

  // função recursiva para construir a árvore, onde cada nó da árvore contém uma árvore associada para as dimensões restantes
  Node* build_recursivo(const A& points, int inicio,int fim)
  {

    if(inicio >= fim) return nullptr;

    int meio = (inicio + fim) / 2;
    Node* newNode = new Node(_x<D>(points[_indices[meio]]), _x<D>(points[_indices[inicio]]), _x<D>(points[_indices[fim-1]]), meio, 0);
    Calcula_Cout_Fist(*newNode, points, inicio, fim);

    int tamanho_conjunto_canonic = fim - inicio;
    if(tamanho_conjunto_canonic > 0)
    {
      newNode->_assocTree = new AssociatedTree;
      newNode->_assocTree->build(points, Gera_indises(inicio, fim, tamanho_conjunto_canonic), tamanho_conjunto_canonic);
    }

    if(tamanho_conjunto_canonic > 1)
    {
      newNode->_childL = build_recursivo(points, inicio, meio - newNode->antes);
      newNode->_childR = build_recursivo(points, (meio + 1) + newNode->depois, fim);
    }

    return newNode;
  }

  index_t* Gera_indises(int inicio,int fim, int n)
  {
    index_t* indices = new IndexArray(n);
    int k = 0;
    for(int j=inicio; j < fim; j++)
    {
      indices[k] = _indices[j];
      k++;
    }
    return indices;
  }

  void Calcula_Cout_Fist(Node &node, const A& points,int inicio,int fim)
  {
    int n = (inicio - fim) / 2;
    for(int i=0; i < n;i++)
    {
      if(_x<D>(points[_indices[i]]) == _root->Split_Value)
      {
        if (i < node.fist) node.fist = i; // se pegar _root->fist - 1 vai poder somar o cout com o indice do ponto atual e vai dar o numero de pontos que tem a mesma coordenada D do valor de divisão
        node.cout++;
        node.antes++;
      }
    }

    for(int i = n+1; i < fim;i++)
    {
      if(_x<D>(points[_indices[i]]) == _root->Split_Value)
      {
        node.cout++;
        node.depois++;
      }
    }
  }

  // Funçao de busca para percorere a arvore e encontar os pontos que vao ser enviados a funçao f
  size_t query(const A& points, const Bounds& bounds, PointFunc f) const
  {
    // insert your code here
    return 0;
  }

  // Função pública para disparar a impressão a partir da raiz
  void print_tree(int indent = 0) const
  {
    if (!_root)
    {
      std::cout << std::string(indent, ' ') << "[Árvore Vazia em D=" << D << "]\n";
      return;
    }
    std::cout << std::string(indent, ' ') << "=== ÁRVORE DIMENSÃO D = " << D << " ===\n";
    print_recursivo(_root, indent);
  }

private:

  // Função privada recursiva para desenhar a estrutura
  void print_recursivo(Node* node, int indent) const
  {
    if (!node) return;

    // 1. Imprime o nó atual (Pivô e limites)
    std::string espaco(indent, ' ');
    std::cout << espaco << "├── [Nó] Pivô: " << node->Split_Value 
              << " | Faixa: [" << node->Min_Valure << ", " << node->Max_Valure << "]\n";

    // 2. Se houver árvore associada (dimensão D-1), imprime ela com recuo extra
    if (node->_assocTree)
    {
      std::cout << espaco << "│   └── Membros Associados (D=" << D-1 << "):\n";
      node->_assocTree->print_tree(indent + 8); // Entra na subárvore com mais recuo
    }

    // 3. Imprime os filhos esquerdo e direito recursivamente
    if (node->_childL)
    {
      std::cout << espaco << "│   ├── Esquerda:\n";
      print_recursivo(node->_childL, indent + 4);
    }
    if (node->_childR)
    {
      std::cout << espaco << "│   └── Direita:\n";
      print_recursivo(node->_childR, indent + 4);
    }
  }

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

    int antes;
    int depois;

    Node* _childL{};
    Node* _childR{};
    AssociatedTree* _assocTree{};
    // IndexArray* Conjunto_Cano;

    Node(real Split_Value, real Min_Valure, real Max_Valure, int fist, int cout):
      Split_Value{Split_Value},
      Min_Valure{Min_Valure},
      Max_Valure{Max_Valure},
      fist{fist},
      cout{cout},
      antes{0},
      depois{0},
      _childL{nullptr},
      _childR{nullptr},
      _assocTree{nullptr}
    {
      // do nothing
    }

  }; // Node

  Node* _root{};
  IndexArray _indices;

}; // BBST

}// end namespace rtree

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
    _mainTree.build(_points, nullptr, _points.size());
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
