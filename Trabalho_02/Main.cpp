#include "TriangleMesh.h"
#include "Mesh.h"

tcii::cg::Mesh* malhaGlobal = nullptr;
float anguloRotacao = 0.0f;

namespace tcii::cg {
    ObjectPtr<TriangleMesh> readOBJ(const char* filename);
}

int main(int argc, char** argv)
{
    std::string caminhoOBJ = argc > 1 ? argv[1] : "bunny.obj"; // Tente usar o argumento da linha de comando, senão use "bunny.obj"
    std::cout << "Carregando o arquivo OBJ: " << caminhoOBJ << "\n";
    auto triangleMesh = tcii::cg::readOBJ(caminhoOBJ.c_str()); 
    if (!triangleMesh) {
        std::cerr << "Erro crítico: O arquivo " << caminhoOBJ << " não foi encontrado no diretório atual!\n";
        return 1;
    }

    std::cout << "Construindo a nova Mesh de Semi-Arestas...\n";
    tcii::cg::Mesh minhaMesh(*triangleMesh);
    
    // Faz o print no terminal para ver as imformações da malha antes de abrir a janela gráfica
    std::cout << "Janela gráfica configurada! Abrindo viewport...\n";
    // 3. Imprime a topologia (descomenta se quiseres ver a tabela gigante de novo)
    minhaMesh.printTopology();

    std::cout << "\n[TESTE 1] Arestas incidentes no Vertice 0:\n";
    minhaMesh.processIncidentEdges(0, [](auto edgeIdx) {
        std::cout << " -> Conectado a Edge: " << edgeIdx << "\n";
    });

    // Teste 2: Vertex k-Ring (Vizinhos até distância 2)
    std::cout << "\n[TESTE 2] Vertices no 2-Anel do Vertice 0:\n";
    int countV = 0;
    minhaMesh.processVertexKRing(0, 2, [&countV](auto vertexIdx) {
        std::cout << " -> Vertice Vizinho: " << vertexIdx << "\n";
        countV++;
    });
    std::cout << "Total de vertices vizinhos encontrados no 2-Anel: " << countV << "\n";

    int faceTeste = 100; // Escolha uma face para testar o k-ring
    // Teste 3: Face k-Ring (Faces vizinhas até distância 1)
    std::cout << "\n[TESTE 3] Faces adjacentes (" << minhaMesh.faceCount() << "-Anel) da Face " << faceTeste << ":\n";
    int countF = 0;
    minhaMesh.processFaceKRing(faceTeste, minhaMesh.faceCount(), [&countF](auto faceIdx) {
        std::cout << " -> Face Vizinha: " << faceIdx << "\n";
        countF++;
    });
    std::cout << "Total de faces vizinhas encontradas no " << minhaMesh.faceCount() << "-Anel: " << countF << "\n";

    return 0;
}