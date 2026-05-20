#ifndef __RangeTree_h
#define __RangeTree_h

#include "util/Array.h"
#include "PointTraits.h"
#include <functional>
#include <numeric>
#include <sstream>

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
  void build(const A& points,int tamanho, IndexArray indices)
  {
    // busca o valor do ponto: _x<1>(points[1])
    _indices = IndexArray(tamanho);
    for(int i=0; i < tamanho; i++)
    {
      _indices[i] = indices[i];
    }
    this->tamanho = tamanho;
    std::sort(_indices.begin(), _indices.end(),
      [&points](int i1, int i2)
      {
        return _x<1>(points[i1]) < _x<1>(points[i2]);
      });
  }
 
  // função de busca para percorrer a árvore e encontrar os pontos que vão ser enviados à função f
  size_t query(const A& points, const Bounds& bounds, PointFunc f) const
  { 
    for (int i = 0; i < tamanho; i++){
      if (bounds.min()[0] > _x<1>(points[_indices[i]])) {
        continue;
      } else if (bounds.max()[0] >= _x<1>(points[_indices[i]])) {
        f(points, _indices[i]);
      } else {
        break;
      }
    }
    return 0;
  }

  void print_tree(const A& points, int indent = 0) const
  {
    std::cout << std::string(indent, ' ') << "═══════════════════════════════════════════\n";
    std::cout << std::string(indent, ' ') << "  BST (Dimensão 1) - " << tamanho << " nós\n";
    std::cout << std::string(indent, ' ') << "═══════════════════════════════════════════\n";
    
    /*for (int i = 0; i < tamanho; ++i)
    {
      size_t idx = _indices[i];
      std::cout << std::string(indent, ' ') << "  " << i << ": ";
      std::cout << "idx=" << idx << " | valor=" << _x<1>(points[idx]) << "\n";
    }*/
    std::cout << "\n";
  }

private:
  IndexArray _indices;
  int tamanho;
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

  void build(const A& points,const int n,const IndexArray indices = IndexArray{})
  {
    assert(!_root); // o assert vai garantir q a arvore seja construida apenas uma vez.
    _indices = IndexArray(n);
    if(indices.size() > 0 && n > 0)
    {
      for(int i = 0; i < n; i++)
      {
        _indices[i] = indices[i];
      }
    }else
    {
      std::iota(_indices.begin(), _indices.end(), 0); // preenche o vetor de indices ordenados com os indices dos pontos
    }
    
    std::sort(_indices.begin(), _indices.end(),
      [&points](int i1, int i2)
      {
        return _x<D>(points[i1]) < _x<D>(points[i2]);
      });
    
    int i = n / 2;
    _root = build_recursivo(points, 0, n);
  }

  // função recursiva para construir a árvore, onde cada nó da árvore contém uma árvore associada para as dimensões restantes
  auto* build_recursivo(const A& points, int inicio,int fim)
  {

    if(inicio >= fim) return static_cast<Node*>(nullptr);

    int meio = (inicio + fim) / 2;
    Node* newNode = new Node(_x<D>(points[_indices[meio]]), _x<D>(points[_indices[inicio]]), _x<D>(points[_indices[fim-1]]), meio, 0);
    Calcula_Cout_Fist(newNode, points, inicio, fim);

    int tamanho_conjunto_canonic = fim - inicio;
    if(tamanho_conjunto_canonic > 0)
    {
      newNode->_assocTree = new AssociatedTree;
      newNode->_assocTree->build(points, tamanho_conjunto_canonic, Gera_indises(inicio, fim, tamanho_conjunto_canonic));
    }

    if(tamanho_conjunto_canonic > 1)
    {
      newNode->_childL = build_recursivo(points, inicio, meio - newNode->antes);
      newNode->_childR = build_recursivo(points, (meio + 1) + newNode->depois, fim);
    }

    return newNode;
  }

  IndexArray Gera_indises(int inicio,int fim, int n)
  {
    IndexArray indices = IndexArray(n);
    int j = 0;
    for(int i=inicio; i < fim; i++)
    {
      indices[j] = _indices[i];
      j++;
    }
    return indices;
  }

  void Calcula_Cout_Fist(auto &node, const A& points,int inicio,int fim)
  {
    int meio = (inicio + fim) / 2;
    for(int i=inicio; i < meio;i++)
    {
      if(_x<D>(points[_indices[i]]) == node->Split_Value)
      {
        if (i < node->fist) node->fist = i; // se pegar node->fist - 1 vai poder somar o cout com o indice do ponto atual e vai dar o numero de pontos que tem a mesma coordenada D do valor de divisão
        node->cout++;
        node->antes++;
      }
    }

    for(int i = meio+1; i < fim;i++)
    {
      if(_x<D>(points[_indices[i]]) == node->Split_Value)
      {
        node->cout++;
        node->depois++;
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
  void print_tree(const A& points, int indent = 0) const
  {
    if (!_root)
    {
      std::cout << std::string(indent, ' ') << "[Árvore Vazia - Dimensão " << D << "]\n";
      return;
    }
    
    std::cout << "\n";
    std::cout << std::string(indent, ' ') << "═══════════════════════════════════════════════════════════\n";
    std::cout << std::string(indent, ' ') << "  RANGE TREE - DIMENSÃO " << D << "\n";
    std::cout << std::string(indent, ' ') << "═══════════════════════════════════════════════════════════\n";
    std::cout << "\n";
    
    print_recursivo(_root, indent, points, "");
    std::cout << "\n";
  }

private:

  // Função privada recursiva para desenhar a estrutura
  void print_recursivo(auto node, int indent, const A& points, const std::string& prefix) const
  {
    if (!node) return;
    
    // Imprime o nó atual
    std::cout << prefix;
    std::cout << "├─ Nó [D=" << D << "] ";
    std::cout << "pivô=" << node->Split_Value;
    std::cout << " | faixa=[" << node->Min_Valure << ", " << node->Max_Valure << "]";
    std::cout << " | count=" << node->cout;
    if (node->antes > 0 || node->depois > 0) {
      std::cout << " (←" << node->antes << "/→" << node->depois << ")";
    }
    std::cout << "\n";
    
    // Se houver árvore associada, mostra de forma compacta
    if (node->_assocTree)
    {
      std::string new_prefix = prefix + "│   ";
      std::cout << prefix << "│   └─ Sub-árvore DIM=" << (D-1) << ":\n";
      
      // Salva o buffer atual
      std::stringstream buffer;
      auto old_buffer = std::cout.rdbuf(buffer.rdbuf());
      node->_assocTree->print_tree(points, indent + 4);
      std::cout.rdbuf(old_buffer);
      
      // Imprime cada linha com o prefixo adequado
      std::string line;
      while (std::getline(buffer, line)) {
        if (!line.empty()) {
          std::cout << prefix << "│     " << line << "\n";
        }
      }
    }
    
    // Processa os filhos
    if (node->_childL || node->_childR)
    {
      if (node->_childL)
      {
        std::string new_prefix = prefix + "│   ";
        std::cout << prefix << "├─ Esquerda (x < " << node->Split_Value << "):\n";
        print_recursivo(node->_childL, indent + 2, points, new_prefix);
      }
      
      if (node->_childR)
      {
        std::string new_prefix = prefix + "│   ";
        if (!node->_childL) std::cout << prefix << "├─";
        else std::cout << prefix << "└─";
        std::cout << "Direita (x ≥ " << node->Split_Value << "):\n";
        print_recursivo(node->_childR, indent + 2, points, new_prefix);
      }
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
    float Split_Value;
    float Min_Valure;
    float Max_Valure;
    int fist;
    int cout;

    int antes;
    int depois;

    Node* _childL{};
    Node* _childR{};
    AssociatedTree* _assocTree{};
    // IndexArray* Conjunto_Cano;

    Node(float Split_Value, float Min_Valure, float Max_Valure, int fist, int cout):
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
    _mainTree.build(_points, _points.size());
  }

  void print() const
  {
    _mainTree.print_tree(_points);
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