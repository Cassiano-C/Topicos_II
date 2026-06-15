#ifndef __Mesh_h
#define __Mesh_h

#include "TriangleMesh.h"
#include <vector>

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

    // Métodos de Vizinhança (Tarefa A2)
    void processIncidentEdges(index_t vertexIdx, auto&& function)
    {
        index_t startHe = _vertices[vertexIdx].halfEdge;
        if (startHe == null_index) return;

        index_t currentHe = startHe;
        do
        {
            function(_halfEdges[currentHe].edge);

            index_t twinHe = _halfEdges[currentHe].twin;
            if (twinHe == null_index) break; 

            currentHe = _halfEdges[twinHe].next;

        } while (currentHe != startHe && currentHe != null_index);
    }

    void processVertexKRing(index_t vertexIdx, int k, auto&& function)
    {
        if (vertexIdx >= _vertices.size() || k < 1) return;

        // Vetor para rastrear quem já foi visitado
        std::vector<bool> visited(_vertices.size(), false);
        
        // Fila para a BFS armazenando pares: {indice_do_vertice, nivel_atual}
        std::vector<std::pair<index_t, int>> queue;
        size_t head = 0;

        // Inicializa com o vértice raiz
        visited[vertexIdx] = true;
        queue.push_back({vertexIdx, 0});

        while (head < queue.size())
        {
            auto [currV, currLevel] = queue[head++];

            // Se chegamos no limite do k-anel, não expandimos mais os vizinhos deste nível
            if (currLevel >= k) continue;

            // Navegar ao redor do vértice atual 'currV' para achar seus vizinhos diretos
            index_t startHe = _vertices[currV].halfEdge;
            if (startHe == null_index) continue;

            index_t currentHe = startHe;
            do
            {
                // O destino da semi-aresta gêmea é um vértice vizinho direto!
                index_t twinHe = _halfEdges[currentHe].twin;
                if (twinHe != null_index)
                {
                    index_t neighborV = _halfEdges[twinHe].origin;

                    if (!visited[neighborV])
                    {
                        visited[neighborV] = true;
                        int nextLevel = currLevel + 1;
                        
                        // Executa a função do usuário passando o vértice vizinho encontrado
                        function(neighborV);

                        queue.push_back({neighborV, nextLevel});
                    }
                }

                // Rotaciona para a próxima semi-aresta que sai de 'currV'
                if (twinHe == null_index) break;
                currentHe = _halfEdges[twinHe].next;

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

            // Uma face triangular tem exatamente 3 semi-arestas. Vamos pegar a primeira:
            index_t he0 = _faces[currF].halfEdge;
            if (he0 == null_index) continue;

            index_t he1 = _halfEdges[he0].next;
            index_t he2 = _halfEdges[he1].next;
            index_t faceHeIdxs[3] = { he0, he1, he2 };

            // Para cada uma das 3 arestas do triângulo, olha o vizinho do outro lado (twin)
            for (int i = 0; i < 3; ++i)
            {
                index_t twinHe = _halfEdges[faceHeIdxs[i]].twin;
                if (twinHe != null_index)
                {
                    index_t neighborF = _halfEdges[twinHe].face;

                    // Se a semi-aresta gêmea pertence a uma face real (não é borda) e não foi visitada
                    if (neighborF != null_index && !visited[neighborF])
                    {
                        visited[neighborF] = true;
                        int nextLevel = currLevel + 1;

                        // Executa a função passando a face vizinha encontrada
                        function(neighborF);

                        queue.push_back({neighborF, nextLevel});
                    }
                }
            }
        }
    }
    
    void printTopology() const;

private:
    std::vector<Vertex> _vertices;
    std::vector<Edge> _edges;
    std::vector<Face> _faces;
    std::vector<Boundary> _boundaries;
    std::vector<HalfEdge> _halfEdges;
};

} // namespace tcii::cg

#endif // __Mesh_h