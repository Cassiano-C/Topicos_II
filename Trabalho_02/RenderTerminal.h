#ifndef __RenderTerminal_h
#define __RenderTerminal_h

#include "Mesh.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>

namespace tcii::cg
{

inline void renderizarMeshNoTerminal(const Mesh& mesh, float anguloGraus = 45.0f)
{
    // Tamanho da tela do terminal virtual
    const int LARGURA = 80;
    const int ALTURA = 40;
    
    // Matriz de caracteres (tela) e Z-buffer para controle de profundidade
    std::vector<std::vector<char>> tela(ALTURA, std::vector<char>(LARGURA, ' '));
    std::vector<std::vector<float>> zBuffer(ALTURA, std::vector<float>(LARGURA, -1e9f));

    // Converte o ângulo para radianos para fazer a rotação
    float rad = anguloGraus * M_PI / 180.0f;
    float cosA = std::cos(rad);
    float sinA = std::sin(rad);

    // Encontrar os limites do objeto para centralizar e escalonar automaticamente
    float minX = 1e9f, maxX = -1e9f;
    float minY = 1e9f, maxY = -1e9f;
    float minZ = 1e9f, maxZ = -1e9f;

    for (const auto& v : mesh.vertices())
    {
        minX = std::min(minX, v.position.x); maxX = std::max(maxX, v.position.x);
        minY = std::min(minY, v.position.y); maxY = std::max(maxY, v.position.y);
        minZ = std::min(minZ, v.position.z); maxZ = std::max(maxZ, v.position.z);
    }

    float centroX = (minX + maxX) / 2.0f;
    float centroY = (minY + maxY) / 2.0f;
    float centroZ = (minZ + maxZ) / 2.0f;

    // Fator de escala para caber na tela do terminal
    float tamanhoMax = std::max({maxX - minX, maxY - minY, maxZ - minZ});
    if (tamanhoMax == 0) tamanhoMax = 1.0f;
    float escala = std::min(LARGURA, ALTURA * 2) / tamanhoMax * 0.7f;

    // Paleta de caracteres por "intensidade/proximidade"
    const char deusesASCII[] = ".:-=+*#%@";
    int numGraus = sizeof(deusesASCII) - 1;

    // Projeta cada vértice na tela
    for (const auto& v : mesh.vertices())
    {
        // 1. Centraliza o ponto na origem (0,0,0)
        float x = v.position.x - centroX;
        float y = v.position.y - centroY;
        float z = v.position.z - centroZ;

        // 2. Rotaciona o objeto no eixo Y para podermos vê-lo girar
        float rotX = x * cosA - z * sinA;
        float rotZ = x * sinA + z * cosA;
        float rotY = y; // Mantém o Y fixo

        // 3. Projeção Ortográfica simples + Escala + Ajuste de Proporção de Caracteres do Terminal
        // Multiplicamos o Y por 0.5f porque os caracteres do terminal são mais altos do que largos
        int telaX = static_cast<int>(LARGURA / 2 + rotX * escala);
        int telaY = static_cast<int>(ALTURA / 2 - rotY * escala * 0.5f);

        // Verifica se o ponto projetado está dentro dos limites da tela
        if (telaX >= 0 && telaX < LARGURA && telaY >= 0 && telaY < ALTURA)
        {
            // Z-buffer: Só desenha se este ponto estiver mais perto do observador que o anterior
            if (rotZ > zBuffer[telaY][telaX])
            {
                zBuffer[telaY][telaX] = rotZ;

                // Mapeia a profundidade (Z) para um caractere da paleta
                float normZ = (rotZ - (-tamanhoMax)) / (2.0f * tamanhoMax);
                int idxChar = static_cast<int>(normZ * numGraus);
                if (idxChar < 0) idxChar = 0;
                if (idxChar >= numGraus) idxChar = numGraus - 1;

                tela[telaY][telaX] = deusesASCII[idxChar];
            }
        }
    }

    // 4. Imprime a tela renderizada no terminal
    std::cout << "\033[H"; // Move o cursor para o topo do terminal (evita piscar a tela)
    std::cout << "========================================================================\n";
    std::cout << "               RENDERIZADOR ASCII 3D - VISUALIZANDO O F-16              \n";
    std::cout << "========================================================================\n";
    for (int y = 0; y < ALTURA; ++y)
    {
        for (int x = 0; x < LARGURA; ++x)
        {
            std::cout << tela[y][x];
        }
        std::cout << "\n";
    }
    std::cout << "========================================================================\n";
    std::cout << "Angulo de rotacao atual: " << anguloGraus << " graus\n";
}

} // namespace tcii::cg

#endif // __RenderTerminal_h