/*
Alunos: Ari Vargas Leal Filho, Cassiano Carvalho de Souza, Lucas Lacerda Arruda.

Na execução deste trabalho, todos os objetivos foram concluídos integralmente:
Construção da BBST de dimensão 1D: Lucas (Build e Query);
Construção da BBST de dimensão D (2D, 3D, ...): Cassiano (Build) e Ari (Query).
A implementação rodou e devolveu corretamente todas as respostas dos casos de testes executados.

Link do vídeo (Google Drive): https://drive.google.com/file/d/1kl2i4KrdJ4JFGMfZVSyZtkHS3SN-hBMb/view?usp=sharing
OBS: O vídeo tem mais de 16 minutos, pois explicamos em detalhes a lógica usada nas funções.
*/

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
  /*
    Essa função vai apenas ordenar o vetor de indices que receber da dimenção superior e ordenar 
    ele na dimensão 1, ou seja, ordena os indices dos pontos de acordo com o valor da coordenada 1 dos pontos.
  */
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
  /*
    Essa função vai percorrer o vetor de indices ordenados e verificar se o valor da coordenada 1 do ponto correspondente ao indice atual está dentro dos limites informados. 
    Se estiver, a função f é chamada com o ponto correspondente ao indice atual e o contador de pontos encontrados é incrementado. 
    O loop continua até que o valor da coordenada 1 do ponto correspondente ao indice atual seja maior que o limite máximo informado, momento em que a busca é interrompida.
  */
  size_t query(const A& points, const Bounds& bounds, PointFunc f) const
  { 
    size_t pontos_encontrados = 0;
    for (int i = 0; i < tamanho; i++){
      if (bounds.min()[0] > _x<1>(points[_indices[i]])) {
        continue;
      } else if (bounds.max()[0] >= _x<1>(points[_indices[i]])) {
        f(points, _indices[i]);
        pontos_encontrados++;
      } else {
        break;
      }
    }
    return pontos_encontrados;
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
  /*
    Essa função vai construir a árvore de dimensão D > 1, onde cada nó da árvore contém uma árvore associada para as dimensões restantes.
    A função recebe o vetor de pontos, o número de pontos e um vetor de índices ordenados (opcional) e constrói a árvore usando a função build_recursivo. 
    O vetor de índices ordenados é usado para construir a árvore de dimensão D > 1, onde cada nó da árvore contém uma árvore associada para as dimensões restantes. 
    Se o vetor de índices ordenados não for fornecido, a função cria um vetor de índices ordenados usando a função std::iota para preencher o vetor com os índices dos pontos.
  */
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

    _root = build_recursivo(points, 0, n);
  }

  /*
    Essa função vai percorrer a árvore e encontrar os pontos que vão ser enviados à função f
  */
  size_t query(const A& points, const Bounds& bounds, PointFunc f) const
  {
    if (!_root) return 0;
    return query_recursivo(points, bounds, f, _root);
  }

private:

  // função recursiva para construir a árvore, onde cada nó da árvore contém uma árvore associada para as dimensões restantes
  /*
    Essa função vai construir a árvore de dimensão D > 1, onde cada nó da árvore contém uma árvore associada para as dimensões restantes. 
    A função recebe o vetor de pontos, os índices de início e fim do vetor de índices ordenados e retorna um ponteiro para o nó raiz da árvore construída. 
    A função calcula o valor de split como a coordenada D do ponto correspondente ao índice do meio do vetor de índices ordenados. 
    Em seguida, a função calcula o número de pontos que têm a mesma coordenada D do valor de split e armazena essa informação no nó. 
    Se houver mais de um ponto com a mesma coordenada D do valor de split, a função constrói uma árvore associada para esses pontos usando a função build da classe BBST para a dimensão D-1. 
    Por fim, a função chama recursivamente para construir as subárvores esquerda e direita usando os índices ajustados para levar em conta os pontos com a mesma coordenada D do valor de split.
  */
  auto* build_recursivo(const A& points, int inicio,int fim)
  {

    if(inicio >= fim) return static_cast<Node*>(nullptr);

    int meio = (inicio + fim) / 2;
    Node* newNode = new Node(_x<D>(points[_indices[meio]]), _x<D>(points[_indices[inicio]]), _x<D>(points[_indices[fim-1]]), meio);
    Calcula_Count_First(newNode, points, inicio, fim,meio);

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

  /*
    Essa função vai gerar um vetor de índices com os índices dos pontos que estão dentro da faixa informada.
    A função recebe os índices de início e fim do vetor de índices ordenados e o número de pontos que têm a mesma coordenada D do valor de split. 
    A função cria um novo vetor de índices e preenche esse vetor com os índices dos pontos correspondentes aos índices de início e fim do vetor de índices ordenados. 
    Esse vetor vai ser o conjunto canonico para a construção da árvore associada, ou seja, ele vai conter os índices dos pontos que têm a mesma coordenada D do valor de split e que estão dentro da faixa informada.
  */
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

  /*
    Essa função vai calcular o número de pontos que têm a mesma coordenada D do valor de split e armazena essa informação no nó.
  */
  void Calcula_Count_First(auto &node, const A& points,int inicio,int fim,int meio)
    {
    for(int i = meio-1;i >= inicio && _x<D>(points[_indices[i]]) == node->Split_Value;i--)
    {
      node->antes++;
      node->first = i;
    }
    for(int i = meio+1;i < fim && _x<D>(points[_indices[i]]) == node->Split_Value;i++)
    {
      node->depois++;
    }
    node->count += node->antes + node->depois;
  }

  /*
    Essa função vai percorrer a árvore e encontrar os pontos que vão ser enviados à função f
    A função recebe o vetor de pontos, os limites da consulta, a função f e um ponteiro para o nó atual da árvore. 
    A função verifica se o nó atual está totalmente contido na faixa informada. 
    Se estiver, a função chama a função query da árvore associada para encontrar os pontos correspondentes aos índices armazenados no nó e envia esses pontos para a função f. 
    Se o valor de split do nó atual corta a faixa informada, a função percorre os índices armazenados no nó e verifica se os pontos correspondentes a esses índices estão dentro da faixa informada. 
    Se estiverem, a função f é chamada com esses pontos e o contador de pontos encontrados é incrementado. 
    Por fim, a função chama recursivamente para percorrer as subárvores esquerda e direita, ajustando os limites da consulta para levar em conta o valor de split do nó atual.
  */
  size_t query_recursivo(const A& points, const Bounds& bounds, PointFunc f, auto* node) const
  {
    if (!node) return 0;

    size_t pontos_encontrados = 0;

    // Caso 1: Nó está totalmente contido na faixa desta dimensão
    if (node->Min_Valure >= bounds.min()[D-1] && node->Max_Valure <= bounds.max()[D-1])
    {
      // Dispara a busca na próxima dimensão (D-1)
      pontos_encontrados += node->_assocTree->query(points, bounds, f);
    }
    else
    {
      // Caso 2: O valor de split corta a faixa informada
      if (node->Split_Value >= bounds.min()[D-1] && node->Split_Value <= bounds.max()[D-1]) {
        for (int _valores = node->first; _valores < node->first + node->count; _valores++) {
          bool ponto_valido = true;
          
          for (int _dim = D - 1; _dim >= 1; _dim--) {
            real value = points[_indices[_valores]][_dim - 1]; 
            if (value < bounds.min()[_dim-1] || value > bounds.max()[_dim-1]) {
              ponto_valido = false;
              break;
            }
          }
          
          if (ponto_valido) {
            f(points, _indices[_valores]);
            pontos_encontrados++; // Encontrou um ponto válido aqui!
          }
        }
      }

      // Caso 3: Desce recursivamente para os filhos esquerdo e direito
      if (node->_childL != nullptr && node->Split_Value > bounds.min()[D-1]) {
        pontos_encontrados += query_recursivo(points, bounds, f, node->_childL);
      }
      if (node->_childR != nullptr && node->Split_Value < bounds.max()[D-1]) {
        pontos_encontrados += query_recursivo(points, bounds, f, node->_childR);
      }
    }

    return pontos_encontrados;
  }


  using real = typename P::value_type;
  using AssociatedTree = BBST<D - 1, P, A>;

  /*
    Estrutura que representa um nó da árvore de intervalos.
    Split_Value: valor de divisão para a dimensão D do nó.
    Min_Valure: valor mínimo da coordenada D dos pontos armazenados no nó.
    Max_Valure: valor máximo da coordenada D dos pontos armazenados no nó.
    first: índice do primeiro ponto no vetor de índices ordenados que tem a mesma coordenada D do valor de divisão.
    count: número de pontos que têm a mesma coordenada D do valor de divisão.
    antes: número de pontos que têm a mesma coordenada D do valor de divisão e estão antes do índice "meio do vetor" no vetor de índices ordenados.
    depois: número de pontos que têm a mesma coordenada D do valor de divisão e estão depois do índice "meio do vetor" no vetor de índices ordenados.
    _childL: ponteiro para o filho esquerdo do nó.
    _childR: ponteiro para o filho direito do nó.
    _assocTree: ponteiro para a árvore associada que contém os pontos que têm a mesma coordenada D do valor de divisão. Essa árvore associada é 
    construída usando a função build da classe BBST para a dimensão D-1 e é usada para encontrar os pontos correspondentes aos índices armazenados 
    no nó quando o nó está totalmente contido na faixa informada durante a consulta.
   */
  struct Node
  {
    real Split_Value;
    real Min_Valure;
    real Max_Valure;
    int first;
    int count;

    int antes;
    int depois;

    Node* _childL{};
    Node* _childR{};
    AssociatedTree* _assocTree{};

    // Construtor para inicializar os membros do nó
    /*
    Essa função é o construtor da estrutura Node, que representa um nó da árvore de intervalos.
    O construtor recebe os seguintes parâmetros:
    Split_Value: valor de divisão para a dimensão D do nó.
    Min_Valure: valor mínimo da coordenada D dos pontos armazenados no nó.
    Max_Valure: valor máximo da coordenada D dos pontos armazenados no nó.
    first: índice do primeiro ponto no vetor de índices ordenados que tem a mesma coordenada D do valor de divisão.
    O construtor inicializa os membros do nó com os valores fornecidos e define os ponteiros para os filhos esquerdo e direito como nullptr, bem como o ponteiro para
    a árvore associada como nullptr. O membro count é inicializado como 1, e os membros antes e depois são inicializados como 0.
    */
    Node(real Split_Value, real Min_Valure, real Max_Valure, int first):
      Split_Value{Split_Value},
      Min_Valure{Min_Valure},
      Max_Valure{Max_Valure},
      first{first},
      count{1},
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
    // Ja mando os pontos e quantidade de pontos 
    _mainTree.build(_points, _points.size());
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