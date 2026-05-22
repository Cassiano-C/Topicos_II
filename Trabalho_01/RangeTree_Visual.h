#ifndef __RangeTree_Visual_h
#define __RangeTree_Visual_h

#include "util/Array.h"
#include "PointTraits.h"
#include <functional>
#include <numeric>
#include <sstream>
#include <fstream>
#include <string>

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

// Especialização para D = 1, onde a árvore é simplesmente uma BST (Binary Search Tree)
template <typename P, typename A>
class BBST<1, P, A>
{
public:
  using Bounds = typename PointTraits<P>::Bounds;
  using PointFunc = rtree::PointFunc<A>;

  // Permitir que BBST de dimensões maiores acessem os membros privados desta classe
  template <size_t X, typename Y, typename Z>
  friend class BBST;

  void build(const A &points, int tamanho, IndexArray indices)
  {
    _indices = IndexArray(tamanho);
    for (int i = 0; i < tamanho; i++)
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

  void print_tree(const A& points, int indent = 0) const
  {
    std::cout << std::string(indent, ' ') << "═══════════════════════════════════════════\n";
    std::cout << std::string(indent, ' ') << "  BST (Dimensão 1) - " << tamanho << " nós\n";
    std::cout << std::string(indent, ' ') << "═══════════════════════════════════════════\n";
    std::cout << "\n";
  }

  // Função pública para D=1 caso seja chamada diretamente
  void exportar_para_graphviz(const std::string &nome_arquivo) const
  {
    std::ofstream arquivo(nome_arquivo);
    if (!arquivo.is_open())
      return;
    arquivo << "digraph G {\n  rankdir=LR;\n  node [shape=box];\n";
    int id = 0;
    exportar_recursivo(nullptr, arquivo, id, "");
    arquivo << "}\n";
  }

  // Esta função será chamada pela dimensão D=2 para desenhar os pontos ordenados de D=1
  void exportar_recursivo(auto node, std::ofstream &arquivo, int &id_contador, const std::string &meu_id) const
  {
    // Como D=1 é um vetor ordenado, vamos desenhar os pontos encadeados linearmente
    std::string id_anterior = "";
    for (int i = 0; i < tamanho; ++i)
    {
      std::string id_atual = "node_d1_" + std::to_string(id_contador++);

      // Se houver uma forma de acessar o valor real do ponto, você pode colocá-lo aqui
      arquivo << "  " << id_atual << " [label=\"Idx: " << _indices[i] << "\", style=filled, fillcolor=\"#fff9c4\"];\n";

      if (i == 0 && !meu_id.empty())
      {
        // Conecta o nó da árvore pai ao primeiro elemento do vetor D=1
        arquivo << "    " << meu_id << " -> " << id_atual << " [style=dotted, color=purple, label=\"vetor D=1\"];\n";
      }
      if (!id_anterior.empty())
      {
        arquivo << "  " << id_anterior << " -> " << id_atual << ";\n";
      }
      id_anterior = id_atual;
    }//*/
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
    
    _root = build_recursivo(points, 0, n);
  }

  // função recursiva para construir a árvore, onde cada nó da árvore contém uma árvore associada para as dimensões restantes
  auto* build_recursivo(const A& points, int inicio,int fim)
  {
    if(inicio >= fim) return static_cast<Node*>(nullptr);

    int meio = (inicio + fim) / 2;
    Node* newNode = new Node(_x<D>(points[_indices[meio]]), _x<D>(points[_indices[inicio]]), _x<D>(points[_indices[fim-1]]), meio);
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

    for(int i = meio-1;i >= inicio && _x<D>(points[_indices[i]]) == node->Split_Value;i--)
    {
      node->antes++;
      node->fist = i;
    }
    for(int i = meio + 1;i < fim && _x<D>(points[_indices[i]]) == node->Split_Value;i++)
    {
      node->depois++;
    }
    node->cout += node->antes + node->depois;
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

  void exportar_para_graphviz(const std::string &nome_arquivo) const
  {
    std::ofstream arquivo(nome_arquivo);
    if (!arquivo.is_open())
      return;

    arquivo << "digraph G {\n";
    arquivo << "  rankdir=TB;\n";
    arquivo << "  node [shape=box, fontname=\"Arial\"];\n";

    int id_contador = 0;
    std::string raiz_id = "node_" + std::to_string(id_contador++);

    exportar_recursivo(_root, arquivo, id_contador, raiz_id);

    arquivo << "}\n";
    arquivo.close();
  }

  void exportar_recursivo(auto node, std::ofstream &arquivo, int &id_contador, const std::string &meu_id) const
  {
    if (!node)
      return;

    // CORREÇÃO AQUI: Ajustado para usar Min_Valure e Max_Valure conforme definido na sua struct Node
    arquivo << "  " << meu_id << " [label=\"Split: " << node->Split_Value << "\\nFaixa: [" << node->Min_Valure << ", " << node->Max_Valure << "]\", style=filled, fillcolor=\"#e1f5fe\"];\n";

    // Se houver árvore associada
    if (node->_assocTree)
    {
      std::string sub_id = "cluster_" + meu_id;
      arquivo << "  subgraph " << sub_id << " {\n";
      arquivo << "    label = \"Sub-árvore Associada D=" << D << "\";\n";
      arquivo << "    color = gray;\n";
      arquivo << "    style = dashed;\n";

      std::string assoc_raiz_id = "node_" + std::to_string(id_contador++);

      // BLINDAGEM COMPILAÇÃO: Se a próxima árvore for D=1, ela não tem _root!
      if constexpr (D > 2)
      {
        // Para D=3 descendo para D=2 (D=2 possui _root)
        node->_assocTree->exportar_recursivo(node->_assocTree->_root, arquivo, id_contador, assoc_raiz_id);
        arquivo << "    " << meu_id << " -> " << assoc_raiz_id << " [style=dotted, color=blue];\n";
      }
      else
      {
        // Para D=2 descendo para D=1 (D=1 NÃO possui _root, passamos nullptr)
        node->_assocTree->exportar_recursivo(nullptr, arquivo, id_contador, meu_id);
      }

      arquivo << "  }\n";
    }

    if (node->_childL)
    {
      std::string esq_id = "node_" + std::to_string(id_contador++);
      arquivo << "  " << meu_id << " -> " << esq_id << " [label=\"L\"];\n";
      exportar_recursivo(node->_childL, arquivo, id_contador, esq_id);
    }
    if (node->_childR)
    {
      std::string dir_id = "node_" + std::to_string(id_contador++);
      arquivo << "  " << meu_id << " -> " << dir_id << " [label=\"R\"];\n";
      exportar_recursivo(node->_childR, arquivo, id_contador, dir_id);
    }
  }
  using real = typename P::value_type;
  using AssociatedTree = BBST<D - 1, P, A>;
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

    Node *_childL{};
    Node *_childR{};
    AssociatedTree *_assocTree{};
    // IndexArray* Conjunto_Cano;

    Node(real Split_Value, real Min_Valure, real Max_Valure, int fist) : 
    Split_Value{Split_Value},
    Min_Valure{Min_Valure},
    Max_Valure{Max_Valure},
    fist{fist},
    cout{1},
    antes{0},
    depois{0},
    _childL{nullptr},
    _childR{nullptr},
    _assocTree{nullptr}
    {
      // do nothing
    }

  }; // Node

  Node *_root{};

  size_t query(const A& points, const Bounds& bounds, PointFunc f) const
  {
    if (!_root) return 0;
    return query_recursivo(points, bounds, f, _root);
  }

private:

  // 2. Função interna que faz o trabalho recursivo real
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

    }
      for (int _valores = node->fist; _valores < node->fist + node->cout; _valores++) {
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

      // Caso 3: Desce recursivamente para os filhos esquerdo e direito
      if (node->_childL && node->Split_Value > bounds.min()[D-1]) {
        pontos_encontrados += query_recursivo(points, bounds, f, node->_childL);
      }
      if (node->_childR && node->Split_Value < bounds.max()[D-1]) {
        pontos_encontrados += query_recursivo(points, bounds, f, node->_childR);
      }
    }


    return pontos_encontrados;
  }

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

  // definir a estrutura de nó da árvore, que deve conter um índice para o ponto correspondente, um ponteiro para a árvore associada e ponteiros para os filhos esquerdo e direito
  // definir as variaveis necessarias
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

  void exportar_para_graphviz(const std::string &nome_arquivo) const
  {
    _mainTree.exportar_para_graphviz(nome_arquivo);
  }

private:
  const A& _points;
  rtree::BBST<D, P, A> _mainTree;

}; // RangeTree

} // end namespace tcii::cg

#endif // __RangeTree_Visual_h