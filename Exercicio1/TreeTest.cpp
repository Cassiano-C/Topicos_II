#include "AVL.h"
#include <cstdio>
#include <cmath>
#include <string>

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

// [Exigência 5]: Uma classe com destrutor personalizado
class TestElement {
private:
    int* _id;

public:
    // Construtor padrão
    TestElement(int id) {
        _id = new int(id);
    }

    // Construtor de cópia (Garante cópia profunda segura)
    TestElement(const TestElement& other) {
        if (other._id) {
            _id = new int(*other._id);
        } else {
            _id = nullptr;
        }
    }

    // Operador de atribuição por cópia (Essencial para o método erase/Hibbard)
    TestElement& operator=(const TestElement& other) {
        if (this != &other) {
            delete _id; // Libera a memória antiga antes de copiar
            if (other._id) {
                _id = new int(*other._id);
            } else {
                _id = nullptr;
            }
        }
        return *this;
    }

    // Destrutor personalizado
    ~TestElement() {
        if (_id) {
            printf("[Destrutor] Elemento ID %d foi desalocado da memoria.\n", *_id);
            delete _id;
            _id = nullptr;
        }
    }

    int getId() const { 
        return _id ? *_id : -1; 
    }

    void print() const {
        if (_id) {
            printf("TestElement(ID: %d)\n", *_id);
        }
    }
};

// Functor de comparação para a classe TestElement
struct ElementLess {
    bool operator()(const TestElement& a, const TestElement& b) const {
        return a.getId() < b.getId();
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

void printElement(const TestElement& e) {
    e.print();
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
    treeInt.insert({5, 3, 7, 9, 8, 0, 4, 2, 1});
    
    printf("Arvore inicial (tamanho %d): ", treeInt.size());
    for(auto i : treeInt) printInt(i);
    puts("");

    // Testando a Busca (find)
    puts("\n-> Testando find():");
    auto itFind = treeInt.find(4);
    if (itFind != treeInt.end()) {
        printf("Elemento %d encontrado via find()!\n", *itFind);
    } else {
        puts("Elemento não encontrado.");
    }

    // Testando a Remoção (erase)
    puts("\n-> Testando erase() de nós com diferentes cenários:");
    printf("Removendo 0 (No folha): %s\n", treeInt.erase(0) ? "Sucesso" : "Falha");
    printf("Removendo 7 (No com dois filhos): %s\n", treeInt.erase(7) ? "Sucesso" : "Falha");
    printf("Tentando remover 99 (Inexistente): %s\n", treeInt.erase(99) ? "Sucesso" : "Falha");

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


    // ----------------------------------------------------
    // TESTE 3: Classe com Destrutor Personalizado
    // ----------------------------------------------------
    puts("==================================================");
    puts("TESTE 3: Classe com Destrutor Personalizado (Limpeza)");
    puts("==================================================");
    
    {
        puts("-> Criando uma árvore local dentro de um escopo {...}");
        avl::Tree<TestElement, ElementLess> treeElement;
        
        treeElement.insert(TestElement(10));
        treeElement.insert(TestElement(20));
        treeElement.insert(TestElement(5));

        puts("\nElementos na arvore:");
        for(auto& e : treeElement) printElement(e);

        puts("\n-> Removendo um unico elemento (ID: 10):");
        treeElement.erase(TestElement(10)); 
        // O destrutor deve rodar imediatamente para o nó removido!

        puts("\n-> Saindo do escopo do bloco. A arvore sera destruida inteira agora:");
    }
    // Ao fechar a chave acima, o destrutor de treeElement é chamado,
    // limpando a raiz e disparando recursivamente o destrutor de todos os nós restantes (5 e 20).
    puts("-> Arvore destruida com sucesso e memoria desalocada!\n");

    return 0;
}