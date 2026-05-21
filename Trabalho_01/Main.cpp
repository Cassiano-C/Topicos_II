#include "graphics/Bounds3.h"
#include "graphics/Vec3.h"
#include "RangeTree_Visual.h"
#include "Utils.h"
#include <cstdio>
#include <cstdlib>

using namespace tcii;

using Point = cg::Vec3f;
using PointArray = cg::Array<Point>;
using Bounds = cg::Bounds3f;

template <>
struct cg::PointTraits<Point>
{
  static constexpr size_t dim = 3;
  using Bounds = ::Bounds;
};

using PointSource = cg::PointSource<Point, PointArray>;
using RangeTree = cg::RangeTree<Point, PointArray>;

void print(size_t id, const Point& p)
{
  printf("ID original %02zu -> Coordenadas: <%g, %g, %g>\n", id, p.x, p.y, p.z);
}

bool printPoint(const PointArray& points, size_t pid)
{
  print(pid, points[pid]);
  return true;
}

void queryTest(const PointArray& points, const Bounds& bounds)
{
  size_t np{};
  for (size_t i = 0; i < points.size(); ++i)
  {
    if (bounds.contains(points[i]))
    {
      printf("  [Lógica Naive] Match: ");
      print(i, points[i]);
      np++;
    }
  }
  printf("Total encontrado (Naive): %zu pontos.\n\n", np);
}

int main(int argc, char** argv)
{
  // 1. Criando um conjunto de pontos estático e previsível
  PointArray points(12);
  
  // Pontos fora da caixa de busca (valores baixos ou muito altos)
  points[0]  = Point{0.5f, 0.5f, 0.5f};
  points[1]  = Point{0.8f, 2.0f, 2.0f}; // Fora em X
  points[2]  = Point{4.5f, 4.5f, 4.5f}; // Fora em tudo (muito alto)

  // Pontos dentro da caixa de busca [1.0, 1.0, 1.0] ate [3.0, 3.0, 3.0]
  points[3]  = Point{1.5f, 1.5f, 1.5f};
  points[4]  = Point{2.0f, 2.0f, 2.0f};
  points[5]  = Point{2.5f, 2.5f, 2.5f};
  
  // Casos Críticos: Pontos exatamente nas bordas do Bounds para testar intervalos abertos/fechados
  points[6]  = Point{1.0f, 1.0f, 1.0f}; // Borda mínima
  points[7]  = Point{3.0f, 3.0f, 3.0f}; // Borda máxima

  // Casos Ultra Críticos: Coordenadas duplicadas em alguma dimensão (Para testar node.antes / node.depois)
  points[8]  = Point{2.0f, 1.2f, 2.8f}; // Compartilha X=2.0 com o points[4]
  points[9]  = Point{2.0f, 2.9f, 1.1f}; // Compartilha X=2.0 com o points[4] e [8]
  points[10] = Point{1.8f, 2.0f, 2.0f}; // Compartilha Y=2.0 e Z=2.0 com points[4]
  points[11] = Point{2.2f, 2.0f, 1.5f}; // Compartilha Y=2.0 com points[4] e [10]


  // 2. Definindo a caixa de consulta (Bounds) de forma controlada
  Bounds bounds;
  bounds.setEmpty();
  bounds.inflate({1.0f, 1.0f, 1.0f}); // Canto Mínimo da busca
  bounds.inflate({3.0f, 3.0f, 3.0f}); // Canto Máximo da busca*/

  /*/ Altomaticamente
  size_t np{100};
  cg::Bounds3f bounds;

  bounds.inflate({0, 0, 0});
  bounds.inflate({5, 5, 5});
  
  auto points = PointSource{}.random(np, bounds);

  bounds.setEmpty();
  bounds.inflate({1, 1, 1});
  bounds.inflate({3, 3, 3});//*/

  printf("==================================================\n");
  printf("Caixa de Consulta (Bounds): Min<1,1,1> ate Max<3,3,3>\n");
  printf("==================================================\n\n");

  puts("** Iniciando: Naive query (Busca Sequencial de Controle)");
  queryTest(points, bounds);

  puts("** Iniciando: Construção da Range Tree (rt.build())");
  RangeTree rt{points};
  rt.build();

  // rt.print();
  rt.exportar_para_graphviz("range_tree.dot");
  
  puts("\n** Iniciando: RT query (Sua busca implementada)");
  // Quando implementar a query, descomente as linhas abaixo:
  size_t np = rt.query(bounds, printPoint);
  printf("Total encontrado (Range Tree): %zu pontos.\n", np);//*/

  puts("\nPressione qualquer tecla para sair...");
  (void)getchar();
  return 0;
}