#include "MeshDecoration.h"

using namespace tcii::cg;

enum { VD, TD };

auto
decorate1(const TriangleMesh& mesh)
{
  using Color = Vec3f;
  using VA = ElementSoA<Color>;
  using TA = ElementSoA<Color>;
  using MD = MeshDecoration<VA, TA>;

  auto md = MD::New(mesh);
  auto nv = mesh.data().vertexCount();

  printf("sizeof(md1): %zu\n", sizeof(*md));
  for (decltype(nv) i = 0; i < nv; ++i)
    attributes<VD>(*md).set(i, Color{0, 0, 1});
  set<VD, 0>(*md, 0, Color{1, 1, 0});

  auto nt = mesh.data().triangleCount();

  for (decltype(nt) i = 0; i < nt; ++i)
    attributes<TD>(*md).set(i, Color{0, 1, 0});
  set<TD, 0>(*md, 0, Color{0, 1, 1});
  return md;
}

template <typename OtherMD>
auto
decorate2(OtherMD* other)
{
  using Color = Vec3f;
  using VA = ElementSoA<Color, Vec3f>;
  using MD = MeshDecoration<VA, typename OtherMD::TA>;

  auto md = MD::New(other);
  auto nv = md->mesh()->data().vertexCount();

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
  using MD = MeshDecoration<typename OtherMD::VA, void>;

  auto md = MD::New(other);

  printf("sizeof(md3): %zu\n", sizeof(*md));
  set<VD, 1>(*md, 0, Vec3f{3, 3, 3});
  return md;
}

void
test()
{
  /*
  struct A
  {
    A() {}
    void f() {}
  };
  struct B
  {
    B() {}
    void f() {}
  };
  struct C
  {
    int c;
  };
  struct EMPTY_BASES D: A, B, C
  {
    int d;
  };

  printf("sizeof(A):%llu\n", sizeof(A));
  printf("sizeof(C):%llu\n", sizeof(C));
  printf("sizeof(D):%llu\n", sizeof(D));
  */
  using VA = ElementSoA<Vec3f, Vec3f>;
  using TA = ElementSoA<Vec3f>;
  using MD = MeshDecoration<VA, TA>;
  using DS = DecorationSet<VA, TA, void>;
  DS ds1{3u, 4u, 5u};
  printf("**%zu\n", sizeof ds1);

  auto& a0 = attributes<0>(ds1);
  auto& a1 = attributes<1>(ds1);
  //auto& a2 = attributes<2>(ds);

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
  //test();
  /*
  puts("Press any key to exit...");
  (void)getchar();
  return 0;
  */

  auto filename = "../../meshes/f-16.obj";
  auto mesh = readOBJ(filename);

  if (!mesh)
    printf("Could not read '%s'\n", filename);
  else
  {
    //mesh->print(filename);

    auto md1 = decorate1(*mesh);

    std::cout << get<VD, 0>(*md1, 0) << '\n';
    std::cout << get<TD, 0>(*md1, 0) << '\n';

    auto md2 = decorate2(md1.get());

    std::cout << get<VD, 1>(*md2, 0) << '\n';
    std::cout << get<TD, 0>(*md2, 0) << '\n';
 
    auto md3 = decorate3(md2.get());

    std::cout << get<VD, 1>(*md3, 0) << '\n';

  }
  puts("Press any key to exit...");
  (void)getchar();
  return 0;
}
