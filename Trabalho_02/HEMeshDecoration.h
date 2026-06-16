#ifndef __HEMeshDecoration_h
#define __HEMeshDecoration_h

#include "MeshA3.h"
#include "DecorationSet.h"
#include "util/SharedObject.h"

namespace tcii::cg
{
    template <typename V, typename E, typename F, typename B>
    class HEMeshDecoration : public SharedObject, 
                             public DecorationSet<ElementSoA<V>, ElementSoA<E>, ElementSoA<F>, ElementSoA<B>>
    {
    public:
        using Base = DecorationSet<ElementSoA<V>, ElementSoA<E>, ElementSoA<F>, ElementSoA<B>>;
        
        static ObjectPtr<HEMeshDecoration> New(index_t nv, index_t ne, index_t nf, index_t nb)
        {
            return ObjectPtr<HEMeshDecoration>{new HEMeshDecoration(nv, ne, nf, nb)};
        }

        HEMeshDecoration(index_t nv, index_t ne, index_t nf, index_t nb) : Base(nv, ne, nf, nb) {}

    public:
        template<size_t N, size_t I = 0, typename U>
        void setAttr(index_t i, const U& value) {
            tcii::cg::attributes<N>(*this).template set<I>(i, value);
        }

        template<size_t N, size_t I = 0>
        auto getAttr(index_t i) const {
            return tcii::cg::attributes<N>(*this).template get<I>(i);
        }
    };
} 
#endif