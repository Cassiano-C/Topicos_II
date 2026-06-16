#ifndef __MeshA3_h
#define __MeshA3_h

#include "graphics/Vec3.h"
#include <vector>

namespace tcii::cg
{
    using index_t = unsigned;

    class MeshA3
    {
    public:
        size_t vertexCount() const { return 100; } // Ajuste conforme seu real
        size_t faceCount() const { return 100; }   // Ajuste conforme seu real
    };
} 
#endif