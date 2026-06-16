#ifndef __Mesh_h
#define __Mesh_h

#include "TriangleMesh.h"
#include <vector>
#include <unordered_map>
#include <utility>
#include <iostream>
#include <iomanip>
#include <GL/gl.h>

namespace tcii::cg
{

using index_t = unsigned;
constexpr index_t null_index = -1u; // Representa o ponteiro nulo/vazio

struct HalfEdge 
{
    index_t origin = null_index;    // Vértice de onde ela parte 
    index_t twin = null_index;      // Semi-aresta gêmea oposta 
    index_t edge = null_index;      // Aresta cheia à qual pertence 
    index_t face = null_index;      // Face à qual pertence (null_index se for contorno) 
    index_t next = null_index;      // Próxima semi-aresta no laço 
    index_t prev = null_index;      // Semi-aresta anterior no laço 
};

struct Vertex
{
    Vec3f position;                 // Coordenadas espaciais (x, y, z) [cite: 20]
    index_t halfEdge = null_index;  // Uma semi-aresta qualquer que parte dele 
};

struct Edge
{
    index_t halfEdges[2] = { null_index, null_index }; // As duas semi-arestas gêmeas 
};

struct Face
{
    index_t halfEdge = null_index;  // Uma semi-aresta qualquer do seu laço 
};

struct Boundary
{
    index_t halfEdge = null_index;  // Uma semi-aresta qualquer do seu laço de borda [cite: 23]
};

class Mesh
{
public:
    // Construtor principal (Tarefa A2) [cite: 36, 44]
    Mesh(const TriangleMesh& triangleMesh);

    // Iteradores (Tarefa A2)
    const std::vector<Vertex>& vertices() const { return _vertices; }
    const std::vector<Edge>& edges() const { return _edges; }
    const std::vector<Face>& faces() const { return _faces; }
    const std::vector<Boundary>& boundaries() const { return _boundaries; }
    // ADICIONE ESTA LINHA ABAIXO PARA LIBERAR O ACESSO DE LEITURA:
    const std::vector<HalfEdge>& halfEdges() const { return _halfEdges; }

    void processIncidentEdges(index_t vertexIdx, auto&& function)
    {
    if (vertexIdx >= _vertices.size()) return;

    index_t startHe = _vertices[vertexIdx].halfEdge;
    if (startHe == null_index) return;

    index_t currentHe = startHe;
    do
    {
        // Executa a função na aresta cheia (Edge) associada
        if (_halfEdges[currentHe].edge != null_index) {
            function(_halfEdges[currentHe].edge);
        }

        // Navegação ultra segura ao redor do vértice de origem:
        // Pega a anterior no triângulo (prev) e vai para a gêmea dela (twin)
        index_t prevHe = _halfEdges[currentHe].prev;
        currentHe = _halfEdges[prevHe].twin;

    } while (currentHe != startHe && currentHe != null_index);
    }

    void processVertexKRing(index_t vertexIdx, int k, auto&& function)
    {
    if (vertexIdx >= _vertices.size() || k < 1) return;

    std::vector<bool> visited(_vertices.size(), false);
    std::vector<std::pair<index_t, int>> queue;
    size_t head = 0;

    // O vértice inicial conta como visitado para não ser processado como vizinho dele mesmo
    visited[vertexIdx] = true;
    queue.push_back({vertexIdx, 0});

    while (head < queue.size())
    {
        auto [currV, currLevel] = queue[head++];

        if (currLevel >= k) continue;

        index_t startHe = _vertices[currV].halfEdge;
        if (startHe == null_index) continue;

        index_t currentHe = startHe;
        do
        {
            // O vértice vizinho está no destino da semi-aresta que sai de currV.
            // O destino da currentHe é a origem da sua próxima (next).
            index_t nextHe = _halfEdges[currentHe].next;
            index_t neighborV = _halfEdges[nextHe].origin;

            if (neighborV != null_index && !visited[neighborV])
            {
                visited[neighborV] = true;
                function(neighborV);
                queue.push_back({neighborV, currLevel + 1});
            }

            // Rotaciona de forma segura usando o operador de rotação padrão de Half-Edge
            index_t prevHe = _halfEdges[currentHe].prev;
            currentHe = _halfEdges[prevHe].twin;

        } while (currentHe != startHe && currentHe != null_index);
    }
    }

    void processFaceKRing(index_t faceIdx, int k, auto&& function)
    {
    if (faceIdx >= _faces.size() || k < 1) return;

    std::vector<bool> visited(_faces.size(), false);
    std::vector<std::pair<index_t, int>> queue;
    size_t head = 0;

    visited[faceIdx] = true;
    queue.push_back({faceIdx, 0});

    while (head < queue.size())
    {
        auto [currF, currLevel] = queue[head++];

        if (currLevel >= k) continue;

        index_t he0 = _faces[currF].halfEdge;
        if (he0 == null_index) continue;

        index_t he1 = _halfEdges[he0].next;
        index_t he2 = _halfEdges[he1].next;
        index_t faceHeIdxs[3] = { he0, he1, he2 };

        for (int i = 0; i < 3; ++i)
        {
            index_t twinHe = _halfEdges[faceHeIdxs[i]].twin;
            if (twinHe != null_index)
            {
                index_t neighborF = _halfEdges[twinHe].face;

                // Garante que só visita se for uma face real (não contorno/borda)
                if (neighborF != null_index && !visited[neighborF])
                {
                    visited[neighborF] = true;
                    function(neighborF);
                    queue.push_back({neighborF, currLevel + 1});
                }
            }
        }
    }
    }

    void printTopology() const;
    void renderGL() const;

private:
    std::vector<Vertex> _vertices;
    std::vector<Edge> _edges;
    std::vector<Face> _faces;
    std::vector<Boundary> _boundaries;
    std::vector<HalfEdge> _halfEdges;
};

} // namespace tcii::cg

#endif // __Mesh_h