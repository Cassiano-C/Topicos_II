#ifndef __AVL_h
#define __AVL_h

#include <functional>
#include <initializer_list>
#include <cstdint>
#include <utility>

namespace tcii::avl
{ // begin namespace tcii::avl

struct TreeNodeBase
{
  TreeNodeBase* _parent;
  TreeNodeBase* _childL{};
  TreeNodeBase* _childR{};
  int _height{};

  TreeNodeBase(TreeNodeBase* parent):
    _parent{parent}
  {
    // do nothing
  }

protected:
  ~TreeNodeBase() = default;

}; // TreeNodeBase

struct AVLHelper
{
  static auto height(const TreeNodeBase* node)
  {
    return node ? node->_height : 0;
  }

  static auto balanceFactor(const TreeNodeBase* node)
  {
    return node ? height(node->_childR) - height(node->_childL) : 0;
  }

  static void updateHeight(TreeNodeBase* node)
  {
    if (node)
    {
      auto hl = height(node->_childL);
      auto hr = height(node->_childR);

      node->_height = (hl > hr ? hl : hr) + 1;
    }
  }

  static void rotateL(TreeNodeBase*&);
  static void rotateR(TreeNodeBase*&);
  static void balance(TreeNodeBase*&);

}; // AVLHelper

struct TreeIteratorBase
{
  bool operator ==(const TreeIteratorBase& other) const
  {
    return _node == other._node;
  }

  bool operator !=(const TreeIteratorBase& other) const
  {
    return !operator ==(other);
  }

protected:
  TreeNodeBase* _node;

  TreeIteratorBase(TreeNodeBase* node):
    _node{node}
  {
    // do nothing
  }

  void increment();
  void decrement();

}; // TreeIteratorBase

template <typename T>
class TreeNode: public TreeNodeBase
{
public:
  T _value;

  TreeNode(const T& value, TreeNodeBase* parent):
    TreeNodeBase{parent},
    _value{value}
  {
    // do nothing
  }

  ~TreeNode()
  {
    delete (TreeNode*)_childL;
    delete (TreeNode*)_childR;
  }

}; // TreeNode

template <typename T>
class TreeIterator: public TreeIteratorBase
{
public:
  TreeIterator(TreeNode<T>* node):
    TreeIteratorBase{node}
  {
    // do nothing
  }

  auto& operator *() const
  { 
    return ((TreeNode<T>*)_node)->_value;
  }

  auto operator ->() const
  {
    return &operator *();
  }

  auto& operator ++()
  {
    increment();
    return *this;
  }

  auto operator ++(int)
  {
    auto temp = *this;
 
    increment();
    return temp;
  }
    
  // INSERT YOUR CODE HERE

}; // TreeIterator

template <typename T, typename C = std::less<T>>
class Tree
{
public:
  using iterator = TreeIterator<T>;
  using IterFunc = void(const T&);

  Tree(C comp = C{}):
    _comp{comp}
  {
    // do nothing
  }

  ~Tree()
  {
    clear();
  }

  void clear()
  {
    if (_root)
    {
      delete _root;
      _root = nullptr;
      _nodeCount = 0;
    }
  }

  std::pair<iterator, bool> insert(const T&);

  void insert(std::initializer_list<T> list)
  {
    for (const auto& value : list)
      insert(value);
  }

  auto size() const
  {
    return _nodeCount;
  }

  auto empty() const
  {
    return !_root;
  }

  void iterate(IterFunc func) const
  {
    iterate(_root, func);
  }

  auto height() const
  {
    return AVLHelper::height(_root);
  }

  iterator begin() const;

  auto end() const
  {
    return iterator{nullptr};
  }

  iterator find(const T&) const;

  auto contains(const T& value) const
  {
    return find(value) != end();
  }
  
  //Declaração do método booleano erase
  bool erase(const T& value);

private:
  using Node = TreeNode<T>;

  Node* _root{};
  unsigned _nodeCount{};
  C _comp;

  //Declaração do método recursivo erase
  static Node* erase(Node* node, const T& value, bool& sucess, C& comp);
  
  static Node* insert(Node*, TreeNodeBase*, const T&, Node*&, C&);
  static void iterate(Node*, IterFunc);

}; // Tree

template <typename T, typename C>
auto
Tree<T, C>::insert(const T& value) -> std::pair<iterator, bool>
{
  Node* valueNode;
  auto root = insert(_root, nullptr, value, valueNode, _comp);
  auto success = bool(root);

  if (success)
  {
    _root = root;
    _nodeCount++;
  }
  return {valueNode, success};
}

template <typename T, typename C>
auto
Tree<T, C>::begin() const -> iterator
{
  TreeNodeBase* node{_root};

  if (node)
    while (node->_childL)
      node = node->_childL;
  return iterator{(Node*)node};
}

template <typename T, typename C>
auto
Tree<T, C>::find(const T& value) const -> iterator
{
  //atual inicia na root
  const Node* atual = _root;

  //Enquanto atual não for nullptr
  while (atual)
  {
    //Se o valor do nó for menor que o valor a ser buscado, vai pro filho à esquerda
    if(_comp(value, atual->_value))
    {
      atual = (Node*)atual->_childL;
    }

    //Se o valor do nó for maior que o valor a ser buscado, vai pro filho à direita
    else if(_comp(atual->_value, value))
    {
      atual = (Node*)atual->_childR;
    }

    //Caso seja igual, achamos o valor, retornamos o iterador do nó
    else{
      return iterator{(Node*) atual};
    }
  }

  //Caso termine o laço, então o valor não foi achado, retorna end()
  return end();
}

//Método público booleano que retorna true para remoção e false para não remoção
template<typename T, typename C>
bool
Tree<T, C>::erase(const T& value)
{
  //Utiliza do método find para achar o valor
  //Se o valor não for encontrado, retorna false
  if(find(value) == end())
  {
    return false;
  }

  //Caso seja...

  bool sucess = false;

  //Vai para a função recursiva e retorna sucess igual a true
  _root = erase(_root, value, sucess, _comp);

  if (sucess)
  {
    _nodeCount--;//Decrementa 1 do número de nós
  }
  return sucess; //retorna true
}

template <typename T, typename C>
TreeNode<T>*
Tree<T, C>::erase(Node* node, const T& value, bool& sucess, C& comp)
{
  if(!node) //Se não houver árvore, retorna nulo
  {
    return nullptr;
  }

  ////Se o valor do nó a ser removido for menor do nó atual, faça um erase (recursão) do filho esquerdo
  if (comp(value, node->_value))
  {
    node->_childL = erase((Node*)node->_childL, value, sucess, comp);
  }

  ////Se o valor do nó a ser removido for maior do nó atual, faça um erase (recursão) do filho direito
  else if (comp(node->_value, value))
  {
    node->_childR = erase((Node*)node->_childR, value, sucess, comp);
  }

  //Caso seja o mesmo valor, achamos o nó a ser removido
  else
  {
    sucess = true; //A remoção irá acontecer (true)


    //Caso o nó a ser removido tenha apenas o filho á direita ou seja nó folha
    if(!node->_childL)
    {
      //Baseia-se no filho direito (se houver)
      Node* aux = (Node*) node->_childR;

      //Se aux for igual a nullptr, então não existe filho direito (é no folha)
      //Caso existe, o filho a direita assume o lugar do pai removido
      if(aux)
      {
        aux->_parent = node->_parent;
      }

      node->_childR = nullptr; //Desconecta o ponteiro do filho a direita do nó 
      delete node; //Remove o nó
      return aux; //Retorna sem o nó que foi removido
    }

    //Nesse caso, é lógico afirmar que se a 1ª condição não for verdadeira, então existe filho à esquerda, sem possibilidade de ser nó folha
    //Com isso, o if(aux) não é necessário
    else if(!node->_childR)
    { //Baseia-se no filho esquerdo
      Node* aux = (Node*) node->_childL;
      aux->_parent = node->_parent; //Substitui o pai (nó a ser removido) pelo filho esquerdo
      node->_childL = nullptr; //Desconecta o ponteiro do filho a esquerda do nó
      delete node; //Remove o nó
      return aux; //Retorna sem o nó que foi removido
    }

    //Caso o nó tenha filho direito e filho esquerdo
    else
    {
      //Utilizaremops do algoritmo de Hibbard para deixar a AVL com suas condições válidas
      
      //Pegaremos o filho direito do nó
      Node* sub = (Node*)node->_childR;

      //A partit do filho direito, verificamos se esse filho tem filho à esqeurda
      //Desse filho esqeurdo, verificamos se tem filho esqeurdo, até chegar no nó mais à esqeurda a partir do filho direito do nó a ser removido.
      while(sub->_childL)
      {
        sub = (Node*)sub->_childL;
      }

      //O nó mais À esquerda do filho direito ou o próprio filho direito caso esse não tenha filho esquerdo substitui o valor do nó a ser removido com o seu
      node->_value = sub->_value;

      //O método erase vai descartar o nó que substituiu o valor do nó "removido" com o seu.
      node->_childR = erase((Node*)node->_childR, sub->_value, sucess, comp);
    }
  }

  //Após a remoção, devemos balancear a árvore e retornar ela balanceada.
  TreeNodeBase* auxNode{node};
  AVLHelper::updateHeight(auxNode); 
  AVLHelper::balance(auxNode);
  return (Node*)auxNode;
}

template <typename T, typename C>
TreeNode<T>*
Tree<T, C>::insert(Node* node,
  TreeNodeBase* parent,
  const T& value,
  Node*& valueNode,
  C& comp)
{
  if (!node)
    return valueNode = new Node{value, parent};
  if (comp(value, node->_value))
  {
    if (auto l = insert((Node*)node->_childL, node, value, valueNode, comp))
      node->_childL = l;
    else
      return nullptr;
  }
  else if (comp(node->_value, value))
  {
    if (auto r = insert((Node*)node->_childR, node, value, valueNode, comp))
      node->_childR = r;
    else
      return nullptr;
  }
  else
    return (valueNode = node), nullptr;

  TreeNodeBase* temp{node};

  AVLHelper::balance(temp);
  return (Node*)temp;
}

template <typename T, typename C>
void
Tree<T, C>::iterate(Node* node, IterFunc func)
{
  if (node)
  {
    iterate((Node*)node->_childL, func);
    func(node->_value);
    iterate((Node*)node->_childR, func);
  }
}

} // end namespace tcii::avl

#endif // __AVL_h
