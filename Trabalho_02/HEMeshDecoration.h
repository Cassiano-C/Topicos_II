#ifndef HEMeshDecoration_h
#define HEMeshDecoration_h

#include "DecorationSet.h"
#include "util/SharedObject.h" // Presumindo o uso de SharedObject / ObjectPtr da disciplina

namespace tcii::cg
{

template <typename V, typename E, typename F, typename B>
class EMPTY_BASES HEMeshDecoration : public SharedObject,
                                     public DecorationSet<V, E, F, B>
{
public:
    using Base = DecorationSet<V, E, F, B>;
    using pointer = ObjectPtr<HEMeshDecoration>;

    // Método New para instanciar usando contagens diretas (como usado no seu Main.cpp)
    static auto New(index_t nv, index_t ne, index_t nf, index_t nb)
    {
        return pointer{new HEMeshDecoration{nv, ne, nf, nb}};
    }

    // Facilidade para buscar atributos (get)
    template <size_t ElementIndex>
    auto getAttr(index_t i) const
    {
        // attributes<ElementIndex>(this) acessa a SoA correspondente ao elemento
        // .template get<0>(i) pega o tipo base guardado nela (considerando SoA simples)
        return attributes<ElementIndex>(this).template get<0>(i);
    }

    // Facilidade para modificar atributos (set)
    template <size_t ElementIndex, typename TField>
    void setAttr(index_t i, TField&& value)
    {
        attributes<ElementIndex>(*this).template set<0>(i, std::forward<TField>(value));
    }

private:
    // Construtor privado que repassa os tamanhos para o DecorationSet alocar memória
    HEMeshDecoration(index_t nv, index_t ne, index_t nf, index_t nb) :
        Base{nv, ne, nf, nb}
    {
        // Alocação interna gerenciada automaticamente pelo DecorationSet
    }
};

} // namespace tcii::cg

#endif // __HEMeshDecoration_h