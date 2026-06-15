#include "Mesh.h"
#include "HEMeshDecoration.h"
#include <iostream>
#include <cassert>

using namespace tcii::cg;

enum { VD, TD, ED };

namespace tcii::cg {
  ObjectPtr<TriangleMesh> readOBJ(const char* filename);
}

auto
decorate1(const Mesh& mesh)
{
  using Color = Vec3f;
  using VA = ElementSoA<Color>;
  using TA = ElementSoA<Color>;
  using EA = ElementSoA<int>;
  using MD = HEMeshDecoration<VA, TA, EA>;

  auto md = MD::novo(mesh);
  auto nv = mesh.vertices().size();

  printf("sizeof(md1): %zu\n", sizeof(*md));
  for (decltype(nv) i = 0; i < nv; ++i)
    attributes<VD>(*md).set(i, Color{0, 0, 1});
  set<VD, 0>(*md, 0, Color{1, 1, 0});

  auto nt = mesh.faces().size();


  for (decltype(nt) i = 0; i < nt; ++i)
    attributes<TD>(*md).set(i, Color{0, 1, 0});
  set<TD, 0>(*md, 0, Color{0, 1, 1});

  md->decorarArestasDeBorda();


  return md;
}

template <typename OtherMD>
auto
decorate2(OtherMD* other)
{
  using Color = Vec3f;
  using VA = ElementSoA<Color, Vec3f>;
  using MD = HEMeshDecoration<VA, typename OtherMD::TA, typename OtherMD::EA>;

  auto md = MD::novo(other);
  auto nv = md->mesh()->vertices().size();


  printf("sizeof(md2): %zu\n", sizeof(*md));
  for (decltype(nv) i = 0; i < nv; ++i)
    attributes<VD>(*md).set(i, Color{0, 0, 1}, Vec3f{0, 1, 1});
  set<VD, 1>(*md, 0, Vec3f{2, 2, 2});
  set<TD, 0>(*md, 0, Color{1, 1, 1});
  return md;
}

template <typename OtherMD>
auto
decorate3(OtherMD* other)
{
  using MD = HEMeshDecoration<typename OtherMD::VA, void, void>;

  auto md = MD::novo(other);


  printf("sizeof(md3): %zu\n", sizeof(*md));
  set<VD, 1>(*md, 0, Vec3f{3, 3, 3});
  return md;
}

void
test(const Mesh& mesh)
{
  using VA = ElementSoA<Vec3f, Vec3f>;
  using TA = ElementSoA<Vec3f>;
  using EA = ElementSoA<int>;
  using DS = DecorationSet<VA, TA, EA>;
  DS ds1{mesh.vertices().size(), mesh.faces().size(), mesh.edges().size()};
  printf("**%zu\n", sizeof ds1);


  set<0, 1>(ds1, 0, Vec3f{1, 1, 1});

  auto v10 = get<0, 1>(ds1, 0);

  set<1, 0>(ds1, 1, v10);
  std::cout << get<1, 0>(ds1, 1) << '\n';

  DecorationSet<void, TA, void> ds2{std::move(ds1)};

  std::cout << get<1, 0>(ds2, 1) << '\n';
}

int
main()
{
  auto filename = "../../meshes/f-16.obj";
  auto triangleMesh = readOBJ(filename);

  if (!triangleMesh)
    printf("Could not read '%s'\n", filename);
  else
  {
    Mesh minhaMesh(*triangleMesh);

    test(minhaMesh);
    std::cout << "\n--- Iniciando Execucao do Pipeline A3 ---\n\n";

    auto md1 = decorate1(minhaMesh);

    std::cout << get<VD, 0>(*md1, 0) << '\n';
    std::cout << get<TD, 0>(*md1, 0) << '\n';
    md1->printDebugArestas();


    auto md2 = decorate2(md1.get());

    std::cout << get<VD, 1>(*md2, 0) << '\n';
    std::cout << get<TD, 0>(*md2, 0) << '\n';
    std::cout << "Aresta 0 (Flag recuperada em md2): " << get<ED, 0>(*md2, 0) << '\n';

    auto md3 = decorate3(md2.get());

    std::cout << get<VD, 1>(*md3, 0) << '\n';
  }
  puts("Press any key to exit...");
  (void)getchar();
  return 0;
}
