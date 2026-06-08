#include "AVL.h"
#include <cstdio>

using namespace tcii;

void
print(const int& i)
{
  printf("%d\n", i);
}

template <typename C>
void
tryInsert(avl::Tree<int, C>& t, int i)
{
  auto [it, success] = t.insert(i);
  printf("%d %sinserted\n", *it, success ? "" : "NOT ");
}

template <typename T>
using C = std::greater<T>;

int
main()
{
  avl::Tree<int, C<int>> t;

  t.insert({5, 3, 7, 9, 8, 0, 4, 2, 1});
  tryInsert(t, 6);
  tryInsert(t, 3);
  printf("Tree size: %d height: %d\n", t.size(), t.height());
  puts("iterate():");
  t.iterate(print);
  puts("Range-based for:");
  for (auto i : t)
    print(i);
  return 0; 
}
