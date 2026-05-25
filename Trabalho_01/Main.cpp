/*
Alunos: Ari Vargas Leal Filho, Cassiano Carvalho de Souza, Lucas Lacerda Arruda.

Na execução deste trabalho, todos os objetivos foram concluídos integralmente:
Construção da BBST de dimensão 1D: Lucas (Build e Query);
Construção da BBST de dimensão D (2D, 3D, ...): Cassiano (Build) e Ari (Query).
A implementação rodou e devolveu corretamente todas as respostas dos casos de testes executados.

Link do vídeo (Google Drive): https://drive.google.com/file/d/1kl2i4KrdJ4JFGMfZVSyZtkHS3SN-hBMb/view?usp=sharing
OBS: O vídeo tem mais de 16 minutos, pois explicamos em detalhes a lógica usada nas funções.
*/

#include "graphics/Bounds3.h"
#include "graphics/Vec3.h"
#include "RangeTree.h"
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

// Função auxiliar para imprimir as coordenadas de um ponto dado seu ID.
void print(size_t id, const Point& p)
{
  printf("ID original %02zu -> Coordenadas: <%g, %g, %g>\n", id, p.x, p.y, p.z);
}

// Função de callback para imprimir um ponto encontrado durante a consulta na RangeTree.
bool printPoint(const PointArray& points, size_t pid)
{
  print(pid, points[pid]);
  return true;
}

// Função de teste para realizar uma consulta ingênua (naive) nos pontos, verificando quais estão contidos dentro dos limites especificados.
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
  size_t np{200};
  cg::Bounds3f bounds;

  bounds.inflate({0, 0, 0});
  bounds.inflate({100, 100, 100});
  
  auto points = PointSource{}.random(np, bounds);

  bounds.setEmpty();
  bounds.inflate({5, 5, 5});
  bounds.inflate({35, 35, 35});
  puts("**Naive query");
  queryTest(points, bounds);
  RangeTree rt{points};

  rt.build();
  puts("**RT query");
  np = rt.query(bounds, printPoint);
  printf("%zu points found\n", np);
  puts("Press any key to exit...");
  (void)getchar();
  return 0;
}