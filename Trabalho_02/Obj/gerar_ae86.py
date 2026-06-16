# Gerador Avançado de Modelo Low-Poly do Toyota AE86
# Versão com Escala Aumentada (x10) e Rodas 3D Reais (estritamente triângulos)

def criar_ae86_completo():
    # Multiplicador para o modelo não ficar pequeno na tela
    escala = 2.0

    # 1. VÉRTICES DA CARROCERIA (Multiplicados pela escala)
    vertices = [
        # --- Chassi / Base Inferior (0 a 3) ---
        [-1.0 * escala, 0.2 * escala,  2.5 * escala],  # 0: Frente Inf Esq
        [ 1.0 * escala, 0.2 * escala,  2.5 * escala],  # 1: Frente Inf Dir
        [-1.0 * escala, 0.2 * escala, -2.5 * escala],  # 2: Traseira Inf Esq
        [ 1.0 * escala, 0.2 * escala, -2.5 * escala],  # 3: Traseira Inf Dir
        
        # --- Linha do Capô / Linha de Cintura (4 a 9) ---
        [-1.0 * escala, 0.6 * escala,  2.4 * escala],  # 4: Para-choque Dianteiro Esq
        [ 1.0 * escala, 0.6 * escala,  2.4 * escala],  # 5: Para-choque Dianteiro Dir
        [-1.0 * escala, 0.7 * escala,  1.2 * escala],  # 6: Base do Para-brisa Esq
        [ 1.0 * escala, 0.7 * escala,  1.2 * escala],  # 7: Base do Para-brisa Dir
        [-1.0 * escala, 0.7 * escala, -2.4 * escala],  # 8: Traseira Linha Cintura Esq
        [ 1.0 * escala, 0.7 * escala, -2.4 * escala],  # 9: Traseira Linha Cintura Dir
        
        # --- Teto / Cockpit (10 a 13) ---
        [-0.8 * escala, 1.2 * escala,  0.2 * escala],  # 10: Topo do Para-brisa Esq
        [ 0.8 * escala, 1.2 * escala,  0.2 * escala],  # 11: Topo do Para-brisa Dir
        [-0.8 * escala, 1.2 * escala, -0.8 * escala],  # 12: Topo do Vidro Traseiro Esq
        [ 0.8 * escala, 1.2 * escala, -0.8 * escala],  # 13: Topo do Vidro Traseiro Dir
    ]

    # 2. CONFIGURAÇÃO DAS RODAS COMO CUBOS 3D (Vértices 14 em diante)
    # Posições dos centros das 4 rodas (Z=Frente/Trás, X=Esq/Dir)
    centros_rodas = [
        (-1.0, 0.3,  1.4),  # Dianteira Esquerda
        ( 1.0, 0.3,  1.4),  # Dianteira Direita
        (-1.0, 0.3, -1.4),  # Traseira Esquerda
        ( 1.0, 0.3, -1.4)   # Traseira Direita
    ]

    triangulos_rodas = []
    
    # Para cada roda, vamos gerar 8 vértices formando um bloco/cubo
    for r_idx, (cx, cy, cz) in enumerate(centros_rodas):
        v_start = len(vertices)
        w = 0.15 * (escala/2)  # Largura da roda
        r = 0.30 * (escala/2)  # Raio/Altura da roda
        
        # 8 Vértices do cubo da roda
        vertices.extend([
            [(cx - w) * escala, (cy - r) * escala, (cz + r) * escala], # 0
            [(cx + w) * escala, (cy - r) * escala, (cz + r) * escala], # 1
            [(cx + w) * escala, (cy + r) * escala, (cz + r) * escala], # 2
            [(cx - w) * escala, (cy + r) * escala, (cz + r) * escala], # 3
            [(cx - w) * escala, (cy - r) * escala, (cz - r) * escala], # 4
            [(cx + w) * escala, (cy - r) * escala, (cz - r) * escala], # 5
            [(cx + w) * escala, (cy + r) * escala, (cz - r) * escala], # 6
            [(cx - w) * escala, (cy + r) * escala, (cz - r) * escala], # 7
        ])
        
        # Faces do cubo da roda mapeadas estritamente em triângulos
        cubo_triangulos = [
            [0, 1, 2], [0, 2, 3], # Frente
            [4, 6, 5], [4, 7, 6], # Trás
            [0, 3, 7], [0, 7, 4], # Esquerda
            [1, 5, 6], [1, 6, 2], # Direita
            [3, 2, 6], [3, 6, 7], # Topo
            [0, 4, 5], [0, 5, 1]  # Base
        ]
        
        # Ajusta os índices locais da roda para os índices globais do arquivo OBJ
        for t in cubo_triangulos:
            triangulos_rodas.append([t[0] + v_start, t[1] + v_start, t[2] + v_start])

    # 3. FACES DA CARROCERIA (Índices baseados em 0)
    triangulos_carroceria = [
        # Capô Dianteiro
        [5, 6, 4], [5, 7, 6],
        # Frente / Grade
        [1, 4, 0], [1, 5, 4],
        # Para-brisa
        [7, 10, 6], [7, 11, 10],
        # Teto
        [11, 12, 10], [11, 13, 12],
        # Vidro Traseiro (Hatchback caidinho do AE86)
        [13, 8, 12], [13, 9, 8],
        # Traseira
        [9, 2, 8], [9, 3, 2],
        
        # Lateral Esquerda
        [4, 6, 0], [6, 2, 0], [6, 8, 2],
        [6, 10, 8], [10, 12, 8],
        
        # Lateral Direita
        [5, 1, 7], [7, 1, 3], [7, 3, 9],
        [7, 9, 11], [11, 9, 13],
        
        # Fundo do Carro
        [0, 2, 1], [1, 2, 3]
    ]

    # Une todas as faces
    todas_as_faces = triangulos_carroceria + triangulos_rodas

    # Gravação do arquivo .obj final
    with open("ae86_lowpoly.obj", "w") as f:
        f.write("# Toyota Sprinter Trueno AE86 - Escala Corrigida com Rodas 3D\n\n")
        
        for v in vertices:
            f.write(f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f}\n")
            
        f.write("\n")
        
        for t in todas_as_faces:
            # +1 porque o formato OBJ começa a contar no índice 1
            f.write(f"f {t[0]+1} {t[1]+1} {t[2]+1}\n")

    print("Sucesso! O modelo 'ae86_lowpoly.obj' foi gerado com tamanho corrigido e rodas 3D.")

if __name__ == "__main__":
    criar_ae86_completo()