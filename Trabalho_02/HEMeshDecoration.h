#ifndef __HEMeshDecoration_h
#define __HEMeshDecoration_h

#include "util/SharedObject.h"
#include <vector>

namespace tcii::cg {

template<typename V, typename E, typename F, typename B>
class HEMeshDecoration : public SharedObject {
public:
    static auto New(size_t nv, size_t ne, size_t nf, size_t nb) {
        return ObjectPtr<HEMeshDecoration>(new HEMeshDecoration(nv, ne, nf, nb));
    }

    template<size_t N, typename T>
    void setAttr(size_t idx, T val) { getArray<N>()[idx] = val; }

    template<size_t N>
    auto getAttr(size_t idx) { return getArray<N>()[idx]; }

private:
    HEMeshDecoration(size_t nv, size_t ne, size_t nf, size_t nb) 
        : vData(nv), eData(ne), fData(nf), bData(nb) {}

    std::vector<V> vData;
    std::vector<E> eData;
    std::vector<F> fData;
    std::vector<B> bData;

    // A MUDANÇA ESTÁ AQUI:
    template<size_t N> 
    auto& getArray() {
        if constexpr (N == 0) return vData;
        else if constexpr (N == 1) return eData;
        else if constexpr (N == 2) return fData;
        else return bData;
    }
};

} 
#endif