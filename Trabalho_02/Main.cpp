#include "TriangleMesh.h"
#include "Mesh.h"
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>      // Garante os helpers de Projeção
#include <GL/freeglut.h>

tcii::cg::Mesh* malhaGlobal = nullptr;
float anguloRotacao = 0.0f;

namespace tcii::cg {
    ObjectPtr<TriangleMesh> readOBJ(const char* filename);
}

// 1. ADICIONADO: Configura a perspectiva para o objeto não sumir ao mudar a janela
void redimensionarJanela(int largura, int altura)
{
    if (altura == 0) altura = 1;
    float aspecto = (float)largura / (float)altura;

    glViewport(0, 0, largura, altura);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Cria um campo de visão 3D confortável (Ângulo, Proporção, Perto, Longe)
    gluPerspective(45.0, aspecto, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
}

void desenharCena()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glLoadIdentity(); 

    // Movemos a câmera ligeiramente para trás (Z = 40) e para cima (Y = 15) 
    // para garantir que modelos grandes ou pequenos entrem no enquadramento
    gluLookAt(0.0, 15.0, 40.0,  0.0, 0.0, 0.0,  0.0, 1.0, 0.0);

    // Rotação automática contínua
    glRotatef(anguloRotacao, 0.0f, 1.0f, 0.0f);
    glRotatef(15.0f, 1.0f, 0.0f, 0.0f); // Inclina um pouco para vermos o caça por cima

    // Modo Wireframe (Apenas Linhas)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // Altera para Branco ou Verde brilhante para destacar no fundo preto
    glColor3f(0.0f, 1.0f, 0.4f); 

    if (malhaGlobal) {
        malhaGlobal->renderGL();
    }

    glutSwapBuffers(); 
}

void atualizarAnimacao(int valor)
{
    anguloRotacao += 1.0f; // Velocidade suave de rotação
    if (anguloRotacao >= 360.0f) anguloRotacao -= 360.0f;

    glutPostRedisplay(); 
    glutTimerFunc(16, atualizarAnimacao, 0); 
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
    malhaGlobal = &minhaMesh; 

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Trabalho 02 - Visualizador Mesh (Half-Edge)");

    glEnable(GL_DEPTH_TEST); 
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // Fundo grafite escuro

    // Registra os Callbacks essenciais
    glutDisplayFunc(desenharCena);
    glutReshapeFunc(redimensionarJanela); // <--- REGISTRADO AQUI
    glutTimerFunc(16, atualizarAnimacao, 0);

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

    // Teste 3: Face k-Ring (Faces vizinhas até distância 1)
    std::cout << "\n[TESTE 3] Faces adjacentes (1-Anel) da Face 0:\n";
    minhaMesh.processFaceKRing(0, 1, [](auto faceIdx) {
        std::cout << " -> Face Vizinha: " << faceIdx << "\n";
    });

    glutMainLoop();


    return 0;
}