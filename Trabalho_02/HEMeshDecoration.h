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

    template <size_t ElementIndex>
    auto getAttr(tcii::cg::index_t i) const {
        // Acessa o conjunto de atributos do elemento e pega o campo 0 (nossa cor/inteiro)
        return attributes<ElementIndex>(*this).template get<0>(i);
    }

    template <size_t ElementIndex, typename TField>
    void setAttr(tcii::cg::index_t i, TField&& value) {
        // Grava o valor no campo 0 do elemento correspondente
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