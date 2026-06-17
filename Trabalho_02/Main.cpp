#include "TriangleMesh.h"
#include "Mesh.h"
#include "HEMeshDecoration.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <cmath>
#include <GL/gl.h>
#include <GL/glu.h>      // Garante os helpers de Projeção
#include <GL/freeglut.h>

// Encapsula os tipos em ElementSoA para que o DecorationSet do professor funcione corretamente
using V_Attributes = tcii::cg::ElementSoA<tcii::cg::Vec3f>; // Vértice guarda 1 campo (Cor)
using E_Attributes = tcii::cg::ElementSoA<tcii::cg::Vec3f>; // Aresta guarda 1 campo (Cor)
using F_Attributes = tcii::cg::ElementSoA<tcii::cg::Vec3f>; // Face guarda 1 campo (Cor)
using B_Attributes = tcii::cg::ElementSoA<int>;             // Borda guarda 1 campo (ID)

using Decoration = tcii::cg::HEMeshDecoration<V_Attributes, E_Attributes, F_Attributes, B_Attributes>;

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

void Pipeline_Decoracao(tcii::cg::Mesh& mesh) {
    tcii::cg::index_t nv = mesh.vertexCount();
    tcii::cg::index_t ne = mesh.halfEdgeCount();
    tcii::cg::index_t nf = mesh.faceCount();
    tcii::cg::index_t nb = mesh.boundaryCount();

    // Instancia o objeto usando o template
    auto decoracao = Decoration::New(
        std::max(nv, 1u), 
        std::max(ne, 1u), 
        std::max(nf, 1u), 
        std::max(nb, 1u)
    );

    // Encontra as coordenadas mínimas e máximas em Y dos vértices
    float yMin = std::numeric_limits<float>::max();
    float yMax = std::numeric_limits<float>::lowest();

    for (tcii::cg::index_t i = 0; i < nv; ++i) {
        float y = mesh.vertexPosition(i).y;
        if (y < yMin) yMin = y;
        if (y > yMax) yMax = y;
    }

    std::cout << "├─► Coordenada Y Mínima: " << yMin << "\n";
    std::cout << "└─► Coordenada Y Máxima: " << yMax << "\n";

    // ESTÁGIO 1: Percorre todos os vértices e os decora de acordo com a altura
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

    // ESTÁGIO 2: Percorre todas as semi-arestas e identifica se são internas ou de borda
    std::cout << "\n{ESTÁGIO 2} Decorar as Semi-Arestas:\n";
    for (tcii::cg::index_t i = 0; i < ne; ++i) {
        if (mesh.halfEdgeFace(i) == tcii::cg::null_index) {
            // Caso seja de Borda —→ Cor Branca
            decoracao->template setAttr<1>(i, tcii::cg::Vec3f{1.0f, 1.0f, 1.0f});
        } 
        else {
            // Caso seja Interna —→ Cor de acordo com sua altura
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

    // ESTÁGIO 3: Percorre todas as faces e as decora de acordo com a altura
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
    mesh.setDecoration(tcii::cg::ObjectPtr<tcii::cg::SharedObject>(decoracao.get()));
}

// Configura a perspectiva para o objeto não achatar ou sumir ao mudar o tamanho da janela
void redimensionarJanela(int largura, int altura)
{
    if (altura == 0) altura = 1;
    float aspecto = (float)largura / (float)altura;

    glViewport(0, 0, largura, altura);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, aspecto, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
}

void desenharCena()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glLoadIdentity(); 

    // Posiciona a câmera em uma distância confortável (Z = 40) e ligeiramente inclinada para cima
    gluLookAt(0.0, 15.0, 40.0,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0);

    // Sistema de rotação contínua automática
    glRotatef(anguloRotacao, 0.0f, 1.0f, 0.0f);
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f); 

    // Configura o renderizador para renderizar as linhas da malha (Wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    if (malhaGlobal) {
        // Tenta recuperar os dados do contêiner de decoração anexado
        auto* deco = malhaGlobal->getDecoration<Decoration>();

        const auto& listaFaces = malhaGlobal->faces();
        const auto& listaVertices = malhaGlobal->vertices();
        const auto& listaHalfEdges = malhaGlobal->halfEdges();

        // Varre todas as faces processando as conexões via Half-Edge
        for (size_t fIdx = 0; fIdx < listaFaces.size(); ++fIdx)
        {
            unsigned he0 = listaFaces[fIdx].halfEdge;
            if (he0 == -1u) continue;

            unsigned nextHe = listaHalfEdges[he0].next;
            unsigned prevHe = listaHalfEdges[he0].prev;
            
            unsigned v0 = listaHalfEdges[he0].origin;
            unsigned v1 = listaHalfEdges[nextHe].origin;
            unsigned v2 = listaHalfEdges[prevHe].origin;

            glBegin(GL_TRIANGLES);
                // --- VÉRTICE 0 ---
                if (deco) {
                    tcii::cg::Vec3f c0 = deco->template getAttr<0>(v0);
                    glColor3f(c0.x, c0.y, c0.z); // Aplica a cor térmica salva
                } else {
                    glColor3f(0.0f, 1.0f, 0.4f); // Cor sólida padrão alternativa
                }
                glVertex3f(listaVertices[v0].position.x, listaVertices[v0].position.y, listaVertices[v0].position.z);
                
                // --- VÉRTICE 1 ---
                if (deco) {
                    tcii::cg::Vec3f c1 = deco->template getAttr<0>(v1);
                    glColor3f(c1.x, c1.y, c1.z);
                } else {
                    glColor3f(0.0f, 1.0f, 0.4f);
                }
                glVertex3f(listaVertices[v1].position.x, listaVertices[v1].position.y, listaVertices[v1].position.z);
                
                // --- VÉRTICE 2 ---
                if (deco) {
                    tcii::cg::Vec3f c2 = deco->template getAttr<0>(v2);
                    glColor3f(c2.x, c2.y, c2.z);
                } else {
                    glColor3f(0.0f, 1.0f, 0.4f);
                }
                glVertex3f(listaVertices[v2].position.x, listaVertices[v2].position.y, listaVertices[v2].position.z);
            glEnd();
        }
    }

    glutSwapBuffers(); 
}

void atualizarAnimacao(int valor)
{
    anguloRotacao += 1.0f; // Incremento suave de ângulo por frame
    if (anguloRotacao >= 360.0f) anguloRotacao -= 360.0f;

    glutPostRedisplay(); 
    glutTimerFunc(16, atualizarAnimacao, 0); 
}

int main(int argc, char** argv)
{
    std::cout << "┌──────────────────────────────────────────────────────────────────────────────────┐\n";
    std::cout << "│                       PROVA 2 ─ Tópicos em Computação II                         │\n";
    std::cout << "│ Alunos: Ari Vargas Leal Filho, Cassiano Carvalho de Souza, Lucas Lacerda Arruda. │\n";
    std::cout << "└──────────────────────────────────────────────────────────────────────────────────┘\n";

    // Procura pelo arquivo .obj passado, caso contrário recorre ao padrão seguro
    std::string caminhoOBJ = argc > 1 ? argv[1] : "bunny.obj";
    std::cout << "\nCarregando o arquivo OBJ: " << caminhoOBJ << "\n";
    auto triangleMesh = tcii::cg::readOBJ(caminhoOBJ.c_str()); 
    if (!triangleMesh) {
        std::cerr << "Erro crítico: O arquivo " << caminhoOBJ << " não foi encontrado no diretório atual!\n";
        return 1;
    }

    std::cout << "Construindo a nova Mesh de Semi-Arestas...\n";
    tcii::cg::Mesh minhaMesh(*triangleMesh);
    
    // Executa e imprime as verificações topológicas exigidas no terminal
    minhaMesh.printTopology();

    std::cout << "─[TESTE 1] Arestas incidentes no Vertice 0:\n";
    minhaMesh.processIncidentEdges(0, [](auto edgeIdx) {
        std::cout << "──► Conectado a Edge: " << edgeIdx << "\n";
    });

    std::cout << "\n┌[TESTE 2] Vertices no 2-Anel do Vertice 0:\n";
    int countV = 0;
    minhaMesh.processVertexKRing(0, 2, [&countV](auto vertexIdx) {
        std::cout << "├─► Vertice Vizinho: " << vertexIdx << "\n";
        countV++;
    });
    std::cout << "│\n";
    std::cout << "└─► Total de vertices vizinhos encontrados no 2-Anel: " << countV << "\n";

    int faceTeste = minhaMesh.faceCount() > 100 ? 100 : 0;
    std::cout << "\n┌[TESTE 3] Faces adjacentes (" << minhaMesh.faceCount() << "-Anel) da Face " << faceTeste << ":\n";
    int countF = 0;
    minhaMesh.processFaceKRing(faceTeste, minhaMesh.faceCount(), [&countF](auto faceIdx) {
        std::cout << "├─► Face Vizinha: " << faceIdx << "\n";
        countF++;
    });
    std::cout << "│\n";
    std::cout << "└─► Total de faces vizinhas encontradas: " << countF << "\n";

    std::cout << "\n┌[TESTE 4] Pipeline com 3 estágios de decoração: \n";
    Pipeline_Decoracao(minhaMesh);

    // Salva a referência na variável global para o loop do GLUT poder ler as decorações
    malhaGlobal = &minhaMesh; 

    // Inicialização da interface gráfica do OpenGL via GLUT
    std::cout << "\nJanela gráfica configurada! Abrindo viewport...\n";
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Trabalho 02 - Visualizador Mesh Decorada (Half-Edge)");

    glEnable(GL_DEPTH_TEST); 
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // Fundo grafite escuro fosco

    // Associa os callbacks essenciais para desenhar, redimensionar e animar
    glutDisplayFunc(desenharCena);
    glutReshapeFunc(redimensionarJanela); 
    glutTimerFunc(16, atualizarAnimacao, 0);

    glutMainLoop();

    return 0;
}