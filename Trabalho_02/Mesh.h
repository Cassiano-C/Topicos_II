#ifndef __Mesh_h
#define __Mesh_h

#include "TriangleMesh.h"
#include "util/SharedObject.h"
#include <vector>

namespace tcii::cg
{

using index_t = unsigned;
constexpr index_t null_index = -1u;

struct HalfEdge {
    index_t origin = null_index;
    index_t twin = null_index;
    index_t edge = null_index;
    index_t face = null_index;
    index_t next = null_index;
    index_t prev = null_index;
};

struct Vertex {
    Vec3f position;
    index_t halfEdge = null_index;
};

struct Edge {
    index_t halfEdges[2] = { null_index, null_index };
};

struct Face {
    index_t halfEdge = null_index;
};

struct Boundary {
    index_t halfEdge = null_index;
};

class Mesh {
public:
    Mesh(const TriangleMesh& triangleMesh);

    const std::vector<Vertex>& vertices() const { return _vertices; }
    const std::vector<Face>& faces() const { return _faces; }
    const std::vector<HalfEdge>& halfEdges() const { return _halfEdges; }

    void processIncidentEdges(index_t vertexIdx, auto&& function);
    void processVertexKRing(index_t vertexIdx, int k, auto&& function);
    void processFaceKRing(index_t faceIdx, int k, auto&& function);

    void setDecoration(ObjectPtr<SharedObject> deco) { _decoration = deco; }
    template<typename T>
    T* getDecoration() { return static_cast<T*>(_decoration.get()); }

    index_t vertexCount() const { return (index_t)_vertices.size(); }
    index_t edgeCount() const { return (index_t)_edges.size(); }
    index_t faceCount() const { return (index_t)_faces.size(); }
    index_t boundaryCount() const { return (index_t)_boundaries.size(); }

    void printTopology() const;
    void renderGL() const;

private:
    std::vector<Vertex> _vertices;
    std::vector<Edge> _edges;
    std::vector<Face> _faces;
    std::vector<Boundary> _boundaries;
    std::vector<HalfEdge> _halfEdges;
    ObjectPtr<SharedObject> _decoration;
};

// --- Implementações Inline ---

inline void Mesh::processIncidentEdges(index_t vertexIdx, auto&& function) {
    index_t startHe = _vertices[vertexIdx].halfEdge;
    if (startHe == null_index) return;
    index_t he = startHe;
    do {
        function(he);
        index_t twin = _halfEdges[he].twin;
        if (twin == null_index) break;
        he = _halfEdges[twin].next;
    } while (he != startHe);
}

inline void Mesh::processVertexKRing(index_t vertexIdx, int k, auto&& function) {
    std::vector<index_t> visited;
    visited.push_back(vertexIdx);
    std::vector<index_t> current = {vertexIdx};

    for(int i = 0; i < k; ++i) {
        std::vector<index_t> next;
        for(index_t v : current) {
            processIncidentEdges(v, [&](index_t he) {
                index_t neighbor = _halfEdges[_halfEdges[he].next].origin;
                // Agora o std::find funcionará corretamente com <algorithm>
                if(std::find(visited.begin(), visited.end(), neighbor) == visited.end()) {
                    visited.push_back(neighbor);
                    next.push_back(neighbor);
                    function(neighbor);
                }
            });
        }
        current = next;
    }
}

inline void Mesh::processFaceKRing(index_t faceIdx, int k, auto&& function) {
    if (k <= 0) return;
    std::vector<bool> visited(faceCount(), false);
    std::queue<std::pair<index_t, int>> q;
    
    q.push({faceIdx, 0});
    visited[faceIdx] = true;

    while(!q.empty()){
        auto [currF, level] = q.front(); q.pop();
        if(level >= k) continue;

        index_t he = _faces[currF].halfEdge;
        for(int i=0; i<3; ++i){
            index_t twin = _halfEdges[he].twin;
            if(twin != null_index){
                index_t neighborF = _halfEdges[twin].face;
                if(neighborF != null_index && !visited[neighborF]){
                    visited[neighborF] = true;
                    function(neighborF);
                    q.push({neighborF, level + 1});
                }
            }
            he = _halfEdges[he].next;
        }
    }
}

} // namespace tcii::cg

#endif