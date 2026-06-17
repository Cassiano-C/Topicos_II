#include "TriangleMesh.h"
#include "Mesh.h"
#include "HEMeshDecoration.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <cmath>

// Mapeia o template
using Decoration = tcii::cg::HEMeshDecoration<tcii::cg::Vec3f, tcii::cg::Vec3f, tcii::cg::Vec3f, int>;

tcii::cg::Mesh* malhaGlobal = nullptr;
float anguloRotacao = 0.0f;

namespace tcii::cg {
    ObjectPtr<TriangleMesh> readOBJ(const char* filename);
}

// Função auxiliar para retornar a cor baseada na altura (1 a 5)
tcii::cg::Vec3f defineCor(int grupo) {
    switch (grupo) {
        case 1: return {0.0f, 1.0f, 1.0f}; // Azul
        case 2: return {0.0f, 1.0f, 0.0f}; // Verde
        case 3: return {1.0f, 1.0f, 0.0f}; // Amarelo
        case 4: return {1.0f, 0.5f, 0.0f}; // Laranja
        case 5: return {1.0f, 0.0f, 0.0f}; // Vermelho
        default: return {1.0f, 1.0f, 1.0f}; // Branco
    }
}

// Função que calcula o grupo de altura (1 a 5) baseado no Y do vértice
int defineGrupoY(float y, float yMin, float yMax) {
    if (std::abs(yMax - yMin) < 1e-5f) return 1; // Malha plana
    
    // Normaliza Y entre 0.0 e 1.0
    float t = (y - yMin) / (yMax - yMin);
    
    // Converte para um grupo de 1 a 5
    int grupo = static_cast<int>(t * 5.0f) + 1;
    return std::clamp(grupo, 1, 5); // Garante que seja estritamente entre 1 e 5
}

void Pipeline_Decoração(tcii::cg::Mesh& mesh) {
    tcii::cg::index_t nv = mesh.vertexCount();
    tcii::cg::index_t ne = mesh.halfEdgeCount();
    tcii::cg::index_t nf = mesh.faceCount();
    tcii::cg::index_t nb = mesh.boundaryCount();

    // Instancia o objeto usando o template
    auto decoracao = Decoration::New(nv, ne, nf, nb);

    // Encontra as coordenadas mínimas em Y dos vértices
    float yMin = std::numeric_limits<float>::max();
    float yMax = std::numeric_limits<float>::lowest();

    for (tcii::cg::index_t i = 0; i < nv; ++i) {
        float y = mesh.vertexPosition(i).y;
        if (y < yMin) yMin = y;
        if (y > yMax) yMax = y;
    }

    std::cout << "├─► Coordenada Y Mínima: " << yMin << "\n";
    std::cout << "└─► Coordenada Y Máxima: " << yMax << "\n";

    // ESTÁGIO 1: Percorre todos os vértices e os decora de acordo com a altura deles na malha
    //Muito alto —→ Pinta com a cor vermelha
    //Alto —→ Pinta com a cor laranja
    //Médio —→ Pinta com a cor amarela
    //Baixo —→ Pinta com a cor verde
    //Muito baixo —→ Pinta com a cor azul
    std::cout << "\n{ESTÁGIO 1} Decorar os Vértices:\n";
    for (tcii::cg::index_t i = 0; i < nv; ++i) {
        float y = mesh.vertexPosition(i).y;
        int grupo = defineGrupoY(y, yMin, yMax);
        tcii::cg::Vec3f cor = defineCor(grupo);
        
        decoracao->template setAttr<0>(i, cor);
    }

    mesh.setDecoration(tcii::cg::ObjectPtr<tcii::cg::SharedObject>(decoracao.get()));
    auto deco = mesh.getDecoration<Decoration>();
    if (deco) {
        std::cout << "┌┤Cores lidas nos 10 primeiros Vértices no modelo (r, g, b):\n";
        for (tcii::cg::index_t i = 0; i < std::min(nv, 9u); ++i) {
            tcii::cg::Vec3f cor = deco->template getAttr<0>(i);
            std::cout << "├─► Cor do Vertice " << i << ": "
                    << "(" << cor.x << ", " << cor.y << ", " << cor.z << ")\n";
        }
        tcii::cg::Vec3f cor = deco->template getAttr<0>(9);
        std::cout << "└─► Cor do Vertice 9: (" << cor.x << ", " << cor.y << ", " << cor.z << ")\n";
    }

    // ESTÁGIO 2: Percorre todas as semi-arestas e identifica se elas são arestas internas ou arestas de borda
    //Caso seja Interna —→ A semi-aresta seja decorada de acordo com sua altura na malha (Semelhante ao Estágio 1)
    //Caso seja de Borda —→ A semi-aresta é pintada de branco (No fim a malha fica com um contorno branco)
    std::cout << "\n{ESTÁGIO 2} Decorar as Semi-Arestas:\n";
    for (tcii::cg::index_t i = 0; i < ne; ++i) {
        // Verifica se pertence a um contorno comparando com a constante nativa
        if (mesh.halfEdgeFace(i) == tcii::cg::null_index) {
            decoracao->template setAttr<1>(i, tcii::cg::Vec3f{1.0f, 1.0f, 1.0f});
        } 
        else {
            // Aresta interna assume o padrão do grupo do seu vértice de origem
            tcii::cg::index_t vOrigem = mesh.halfEdgeOrigin(i);
            float y = mesh.vertexPosition(vOrigem).y;
            int grupo = defineGrupoY(y, yMin, yMax);
            tcii::cg::Vec3f cor = defineCor(grupo);
            
            decoracao->template setAttr<1>(i, cor);
        }
    }

    mesh.setDecoration(tcii::cg::ObjectPtr<tcii::cg::SharedObject>(decoracao.get()));
    deco = mesh.getDecoration<Decoration>();
    if (deco) {
        std::cout << "┌┤Cores lidas nas 10 primeiras Semi-Arestas no modelo (r, g, b):\n";
        for (tcii::cg::index_t i = 0; i < std::min(ne, 9u); ++i) {
            tcii::cg::Vec3f cor = deco->template getAttr<1>(i);
            std::cout << "├─► Cor da Semi-Aresta " << i << ": "
                    << "(" << cor.x << ", " << cor.y << ", " << cor.z << ")\n";
        }
        tcii::cg::Vec3f cor = deco->template getAttr<1>(9);
            std::cout << "└─► Cor da Semi-Aresta 9: (" << cor.x << ", " << cor.y << ", " << cor.z << ")\n";
    }

    // ESTÁGIO 3: Percorre todas as faces e as decora de acordo com a alturas delas na malha (Semelhante ao Estágio 1)
    std::cout << "\n{ESTÁGIO 3} Decorar as Faces:\n";
    for (tcii::cg::index_t i = 0; i < nf; ++i) {
        tcii::cg::index_t heIdx = mesh.faceHalfEdge(i);
        tcii::cg::index_t vOrigem = mesh.halfEdgeOrigin(heIdx);
        float y = mesh.vertexPosition(vOrigem).y;
        int grupo = defineGrupoY(y, yMin, yMax);
        tcii::cg::Vec3f cor = defineCor(grupo);
        
        decoracao->template setAttr<2>(i, cor);
    }

    mesh.setDecoration(tcii::cg::ObjectPtr<tcii::cg::SharedObject>(decoracao.get()));
    deco = mesh.getDecoration<Decoration>();
    if (deco) {
        std::cout << "┌┤Cores lidas nas 10 primeiras Faces no modelo (r, g, b):\n";
        for (tcii::cg::index_t i = 0; i < std::min(nf, 9u); ++i) {
            tcii::cg::Vec3f cor = deco->template getAttr<2>(i);
            std::cout << "├─► Cor da Face " << i << ": "
                    << "(" << cor.x << ", " << cor.y << ", " << cor.z << ")\n";
        }
        tcii::cg::Vec3f cor = deco->template getAttr<2>(9);
            std::cout << "└─► Cor da Face 9: (" << cor.x << ", " << cor.y << ", " << cor.z << ")\n";
    }
}

int main(int argc, char** argv)
{
    std::cout << "┌──────────────────────────────────────────────────────────────────────────────────┐\n";
    std::cout << "│                       PROVA 2 ─ Tópicos em Computação II                         │\n";
    std::cout << "│ Alunos: Ari Vargas Leal Filho, Cassiano Carvalho de Souza, Lucas Lacerda Arruda. │\n";
    std::cout << "└──────────────────────────────────────────────────────────────────────────────────┘\n";

    //Procura pelo arquivo .obj especificado na linha de comando, caso não encontre ele usa o "bunny.obj"
    std::string caminhoOBJ = argc > 1 ? argv[1] : "bunny.obj";
    std::cout << "\nCarregando o arquivo OBJ: " << caminhoOBJ << "\n";
    auto triangleMesh = tcii::cg::readOBJ(caminhoOBJ.c_str()); 
    if (!triangleMesh) {
        std::cerr << "Erro crítico: O arquivo " << caminhoOBJ << " não foi encontrado no diretório atual!\n";
        return 1;
    }

    std::cout << "Construindo a nova Mesh de Semi-Arestas...\n";
    tcii::cg::Mesh minhaMesh(*triangleMesh);
    
    // Faz o print no terminal para ver as informações da malha antes de abrir a janela gráfica
    std::cout << "Janela gráfica configurada! Abrindo viewport...\n";

    // Imprime a topologia
    minhaMesh.printTopology();

    std::cout << "─[TESTE 1] Arestas incidentes no Vertice 0:\n";
    minhaMesh.processIncidentEdges(0, [](auto edgeIdx) {
        std::cout << "──► Conectado a Edge: " << edgeIdx << "\n";
    });

    // Teste 2: Vertex k-Ring (Vizinhos até distância 2)
    std::cout << "\n┌[TESTE 2] Vertices no 2-Anel do Vertice 0:\n";
    int countV = 0;
    minhaMesh.processVertexKRing(0, 2, [&countV](auto vertexIdx) {
        std::cout << "├─► Vertice Vizinho: " << vertexIdx << "\n";
        countV++;
    });
    std::cout << "│\n";
    std::cout << "└─► Total de vertices vizinhos encontrados no 2-Anel: " << countV << "\n";

    int faceTeste = 0; // Escolha uma face para testar o k-ring
    // Teste 3: Face k-Ring (Faces vizinhas até distância 1)
    std::cout << "\n┌[TESTE 3] Faces adjacentes (" << minhaMesh.faceCount() << "-Anel) da Face " << faceTeste << ":\n";
    int countF = 0;
    minhaMesh.processFaceKRing(faceTeste, minhaMesh.faceCount(), [&countF](auto faceIdx) {
        std::cout << "├─► Face Vizinha: " << faceIdx << "\n";
        countF++;
    });
    std::cout << "│\n";
    std::cout << "└─► Total de faces vizinhas encontradas no " << minhaMesh.faceCount() << "-Anel: " << countF << "\n";

    std::cout << "\n┌[TESTE 4] Pipeline com 3 estágios de decoração: \n";
    Pipeline_Decoração(minhaMesh);

    return 0;
}
