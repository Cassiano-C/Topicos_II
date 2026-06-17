#include "Mesh.h"

namespace tcii::cg
{

// Estrutura para podermos usar std::pair como chave no unordered_map
struct PairHash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;  
    }
};

Mesh::Mesh(const TriangleMesh& triangleMesh)
{
    auto nv = triangleMesh.data().vertexCount();
    auto nt = triangleMesh.data().triangleCount();

    _vertices.reserve(nv);
    _faces.reserve(nt);
    _halfEdges.reserve(nt * 3);
    _edges.reserve(nt * 3 / 2); // Estimativa de arestas para malha fechada

    // 1. Inicializar apenas os vértices
    for(index_t i = 0; i < nv; ++i)
    {
        Vertex v;
        v.position = triangleMesh.data().vertex(i);
        v.halfEdge = null_index; // Será preenchido quando criarmos as semi-arestas
        _vertices.push_back(v);
    }

    // Mapa temporário: chave é (menor_v, maior_v), valor é o índice da semi-aresta que já está lá
    std::unordered_map<std::pair<index_t, index_t>, index_t, PairHash> edgeMap;

    // 2. Construir as Faces, Semi-Arestas e conectar Gêmeas/Arestas Cheias
    for(index_t i = 0; i < nt; ++i)
    {
        const auto& tri = triangleMesh.data().triangle(i);
        
        index_t faceIdx = _faces.size();
        _faces.push_back({ null_index }); // Cria a estrutura da face

        index_t firstHeIdx = _halfEdges.size(); // Guarda onde começam as 3 semi-arestas deste triângulo

        // Criar as 3 semi-arestas básicas do triângulo
        for(int j = 0; j < 3; ++j)
        {
            HalfEdge he;
            he.origin = tri[j];
            he.face = faceIdx;
            _halfEdges.push_back(he);

            // Garantir que o vértice saiba de pelo menos uma semi-aresta que sai dele
            _vertices[tri[j]].halfEdge = firstHeIdx + j;
        }

        // Conectar o laço interno do triângulo (Next e Prev)
        for(int j = 0; j < 3; ++j)
        {
            index_t current = firstHeIdx + j;
            index_t next = firstHeIdx + ((j + 1) % 3);
            index_t prev = firstHeIdx + ((j + 2) % 3);

            _halfEdges[current].next = next;
            _halfEdges[current].prev = prev;
        }

        // Aponta a face para a primeira semi-aresta dela
        _faces[faceIdx].halfEdge = firstHeIdx;

        // 3. Casamento de Gêmeas (Twin) e criação de Edges
        for(int j = 0; j < 3; ++j)
        {
            index_t currentHeIdx = firstHeIdx + j;
            index_t v0 = _halfEdges[currentHeIdx].origin;
            index_t v1 = _halfEdges[_halfEdges[currentHeIdx].next].origin; // Vértice de destino

            // Criar uma chave padronizada onde o menor índice vem primeiro
            auto edgeKey = std::make_pair(std::min(v0, v1), std::max(v0, v1));

            auto it = edgeMap.find(edgeKey);
            if (it == edgeMap.end())
            {
                // Primeira vez que vemos essa aresta! 
                // Guardamos ela no mapa esperando que a sua gêmea apareça em outro triângulo
                edgeMap[edgeKey] = currentHeIdx;
            }
            else
            {
                // Achamos a gêmea! (it->second é a semi-aresta antiga)
                index_t twinHeIdx = it->second;

                // Conectar uma na outra via twin
                _halfEdges[currentHeIdx].twin = twinHeIdx;
                _halfEdges[twinHeIdx].twin = currentHeIdx;

                // Criar a aresta cheia (Edge) unindo as duas
                index_t edgeIdx = _edges.size();
                _edges.push_back({ {twinHeIdx, currentHeIdx} });

                // Avisar as duas semi-arestas a qual Edge elas pertencem
                _halfEdges[currentHeIdx].edge = edgeIdx;
                _halfEdges[twinHeIdx].edge = edgeIdx;

                // Remover do mapa pois essa aresta já foi completamente resolvida (par fechado)
                edgeMap.erase(it);
            }
        }
    }

    // --- CÓDIGO CORRIGIDO PARA O FECHAMENTO DE BORDAS NO MESH.CPP ---
    
    // Um mapa para ligar diretamente: [Vértice de Origem] -> [Índice da HalfEdge de Borda]
    std::unordered_map<index_t, index_t> boundary_vMap;

    // 1. Criar as semi-arestas virtuais para quem sobrou no mapa
    for (auto& [key, internalHeIdx] : edgeMap)
    {
        index_t v0 = _halfEdges[internalHeIdx].origin;
        index_t nextInternalHe = _halfEdges[internalHeIdx].next;
        index_t v1 = _halfEdges[nextInternalHe].origin; // Destino da interna

        // Criar a semi-aresta de borda (fictícia) no sentido oposto: v1 -> v0
        index_t externalHeIdx = _halfEdges.size();
        HalfEdge externalHe;
        externalHe.origin = v1;          
        externalHe.face = null_index;    // Borda não tem face real
        _halfEdges.push_back(externalHe);

        // Casar os gêmeos (Twins)
        _halfEdges[internalHeIdx].twin = externalHeIdx;
        _halfEdges[externalHeIdx].twin = internalHeIdx;

        // Criar a estrutura Edge correspondente
        index_t edgeIdx = _edges.size();
        _edges.push_back({ {internalHeIdx, externalHeIdx} });

        _halfEdges[internalHeIdx].edge = edgeIdx;
        _halfEdges[externalHeIdx].edge = edgeIdx;

        // Mapeia que a semi-aresta de borda 'externalHeIdx' parte do vértice 'v1'
        boundary_vMap[v1] = externalHeIdx;
    }

    // 2. Costurar os laços de bordas de forma linear e ultra segura
    for (auto& [originV, externalHeIdx] : boundary_vMap)
    {
        // O destino desta semi-aresta de borda é a origem da sua twin interna (v0)
        index_t twinIdx = _halfEdges[externalHeIdx].twin;
        index_t destV = _halfEdges[twinIdx].origin;

        // A PRÓXIMA semi-aresta de borda será aquela que começa exatamente onde esta termina (destV)
        auto it = boundary_vMap.find(destV);
        if (it != boundary_vMap.end())
        {
            index_t nextExternalHeIdx = it->second;
            _halfEdges[externalHeIdx].next = nextExternalHeIdx;
            _halfEdges[nextExternalHeIdx].prev = externalHeIdx;
        }
    }

    // 3. Registrar os contornos (Boundaries) oficiais da malha
    std::vector<bool> visitedHe(_halfEdges.size(), false);
    for (auto& [originV, externalHeIdx] : boundary_vMap)
    {
        if (visitedHe[externalHeIdx]) continue;

        // Encontrámos um laço de contorno fechado!
        _boundaries.push_back({ externalHeIdx });

        index_t currentHe = externalHeIdx;
        do
        {
            visitedHe[currentHe] = true;
            currentHe = _halfEdges[currentHe].next;
        } while (currentHe != externalHeIdx && currentHe != null_index);
    }

    // 4. Correção crucial de navegação: 
    // Garanta que se um vértice estiver na borda, o seu 'halfEdge' aponte para a de borda.
    // Isso evita travamentos ou asserções inválidas nas funções de navegação da Tarefa A2.
    for (auto& [originV, externalHeIdx] : boundary_vMap)
    {
        _vertices[originV].halfEdge = externalHeIdx;
    }

}

void Mesh::printTopology() const
{
    std::cout << "\n==================================================\n";
    std::cout << "          ESTRUTURA TOPOLÓGICA DA MALHA           \n";
    std::cout << "==================================================\n";

    // 1. Imprimir Vértices (Controlo de Amostragem: mostra os 10 primeiros)
    std::cout << "\n[VÉRTICES] Total: " << _vertices.size() << "\n";
    std::cout << std::setw(6) << "Idx" << " | " 
              << std::setw(25) << "Posição (X, Y, Z)" << " | " 
              << std::setw(12) << "HalfEdge Sai" << "\n";
    std::cout << "--------------------------------------------------\n";
    size_t limitV = std::min(_vertices.size(), size_t(10));
    for (size_t i = 0; i < limitV; ++i)
    {
        const auto& v = _vertices[i];
        std::cout << std::setw(6) << i << " | "
                  << "(" << std::setw(6) << v.position.x << ", " 
                  << std::setw(6) << v.position.y << ", " 
                  << std::setw(6) << v.position.z << ") | ";
        if (v.halfEdge == null_index) std::cout << "null\n";
        else std::cout << std::setw(12) << v.halfEdge << "\n";
    }
    if (_vertices.size() > 10) std::cout << "... (" << _vertices.size() - 10 << " mais)\n";

    // 2. Imprimir Faces (mostra as 5 primeiras)
    std::cout << "\n[FACES] Total: " << _faces.size() << "\n";
    std::cout << std::setw(6) << "Idx" << " | " << std::setw(15) << "HalfEdge Membro" << "\n";
    std::cout << "--------------------------------------\n";
    size_t limitF = std::min(_faces.size(), size_t(5));
    for (size_t i = 0; i < limitF; ++i)
    {
        std::cout << std::setw(6) << i << " | " << std::setw(15) << _faces[i].halfEdge << "\n";
    }
    if (_faces.size() > 5) std::cout << "... (" << _faces.size() - 5 << " mais)\n";

    // 3. Imprimir Semi-Arestas (O mais importante! Mostra as primeiras 15, ou seja, 5 triângulos)
    std::cout << "\n[SEMI-ARESTAS (HALF-EDGES)] Total: " << _halfEdges.size() << "\n";
    std::cout << std::setw(6) << "Idx" << " | " 
              << std::setw(8) << "Origin" << " | " 
              << std::setw(8) << "Twin" << " | " 
              << std::setw(8) << "Next" << " | " 
              << std::setw(8) << "Prev" << " | " 
              << std::setw(8) << "Face" << " | "
              << std::setw(8) << "Edge" << "\n";
    std::cout << "----------------------------------------------------------------------\n";
    size_t limitHe = std::min(_halfEdges.size(), size_t(15));
    for (size_t i = 0; i < limitHe; ++i)
    {
        const auto& he = _halfEdges[i];
        std::cout << std::setw(6) << i << " | "
                  << std::setw(8) << he.origin << " | ";
        
        if (he.twin == null_index) std::cout << std::setw(8) << "null" << " | ";
        else std::cout << std::setw(8) << he.twin << " | ";

        std::cout << std::setw(8) << he.next << " | "
                  << std::setw(8) << he.prev << " | ";

        if (he.face == null_index) std::cout << std::setw(8) << "BORDER" << " | ";
        else std::cout << std::setw(8) << he.face << " | ";

        std::cout << std::setw(8) << he.edge << "\n";
    }
    if (_halfEdges.size() > 15) std::cout << "... (" << _halfEdges.size() - 15 << " mais)\n";

    // 4. Imprimir Contornos de Bordas (Boundaries)
    std::cout << "\n[CONTORNOS DE BORDA (BOUNDARIES)] Total: " << _boundaries.size() << "\n";
    if (_boundaries.empty())
    {
        std::cout << "Nenhum contorno detetado (Malha Fechada!).\n";
    }
    else
    {
        for (size_t i = 0; i < _boundaries.size(); ++i)
        {
            std::cout << "Borda " << i << " começa na HalfEdge externa: " << _boundaries[i].halfEdge << "\n";
        }
    }
    std::cout << "==================================================\n\n";
}

} // namespace tcii::cg