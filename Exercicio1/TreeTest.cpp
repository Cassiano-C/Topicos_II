#include "AVL.h"
#include <cstdio>
#include <cmath>
#include <string>
#include <random>

using namespace tcii;

// ==========================================
// 1. ESTRUTURAS DE TESTE EXIGIDAS
// ==========================================

// [Exigência 5]: Um Agregado (Vec3)
struct Vec3 {
    float x, y, z;

    // Função auxiliar para calcular a magnitude do vetor (usada para comparação)
    float magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    void print() const {
        printf("Vec3(%.1f, %.1f, %.1f) [Mag: %.2f]\n", x, y, z, magnitude());
    }
};

// Functor de comparação para o Vec3 (compara pela magnitude)
struct Vec3Less {
    bool operator()(const Vec3& a, const Vec3& b) const {
        return a.magnitude() < b.magnitude();
    }
};


// ==========================================
// 2. FUNÇÕES AUXILIARES DE IMPRESSÃO
// ==========================================
void printInt(const int& i) {
    printf("%d ", i);
}

void printVec3(const Vec3& v) {
    v.print();
}

// ==========================================
// 3. PROGRAMA PRINCIPAL
// ==========================================
int main() {
    // ----------------------------------------------------
    // TESTE 1: Tipo Primitivo (int) com std::greater
    // ----------------------------------------------------
    puts("==================================================");
    puts("TESTE 1: Tipo Primitivo (int) com std::greater");
    puts("==================================================");
    
    avl::Tree<int, std::greater<int>> treeInt;
    treeInt.insert({5, 3, 7, 9, 8, 0, 4, 2, 1, 6, 10, 15, 12, 11, 14, 13});
    
    printf("Arvore inicial (tamanho %d): ", treeInt.size());
    for(auto i : treeInt) printInt(i);
    puts("");

    // Testando a Busca (find)
    puts("\n-> Testando find():");
    // Inicializa o gerador com uma semente aleatória baseada no dispositivo de hardware
    std::random_device rd;
    std::mt19937 gen(rd());

    // Define o intervalo fechado [1, 6], por exemplo, para um dado de 6 faces
    std::uniform_int_distribution<int> distrib(0, 20);

    // Gera o número randômico
    int numero_secreto = distrib(gen);
    auto itFind = treeInt.find(numero_secreto);
    if (itFind != treeInt.end()) {
        printf("Elemento %d encontrado via find()!\n", *itFind);
    } else {
        printf("Elemento %d não encontrado.\n", numero_secreto);
    }

    // Testando a Remoção (erase)
    puts("\n-> Testando erase() de nós com diferentes cenários:");
    for (int i = 0; i < 5; ++i) {
        numero_secreto = distrib(gen);
        printf("Removendo %d : %s\n", numero_secreto, treeInt.erase(numero_secreto) ? "Sucesso" : "Falha");
    }

    printf("\nArvore apos remocoes (tamanho %d): ", treeInt.size());
    for(auto i : treeInt) printInt(i);
    puts("");

    // Testando sua iteração reversa customizada
    printf("\nIteracao Reversa Customizada (--it): ");
    for (auto it = treeInt.rbegin(); it != treeInt.rend(); --it) {
        printInt(*it);
    }
    puts("\n");


    // ----------------------------------------------------
    // TESTE 2: Agregado (Vec3) com Custom Functor (std::less)
    // ----------------------------------------------------
    puts("==================================================");
    puts("TESTE 2: Agregado (Vec3) com Vec3Less (Ordem Crescente)");
    puts("==================================================");
    
    avl::Tree<Vec3, Vec3Less> treeVec;
    treeVec.insert(Vec3{5.0f, 0.0f, 0.0f});  // Mag: 5.0
    treeVec.insert(Vec3{1.0f, 1.0f, 1.0f});  // Mag: ~1.73
    treeVec.insert(Vec3{10.0f, 2.0f, 0.0f}); // Mag: ~10.20
    treeVec.insert(Vec3{0.0f, 3.0f, 0.0f});  // Mag: 3.0

    puts("Vetores em ordem crescente de magnitude:");
    for(auto v : treeVec) printVec3(v);

    // Testando remoção no Agregado
    puts("\n-> Removendo o vetor de magnitude 3.0...");
    treeVec.erase(Vec3{0.0f, 3.0f, 0.0f});
    
    puts("\nArvore de vetores apos remocao:");
    for(auto v : treeVec) printVec3(v);
    puts("");

    return 0;
}