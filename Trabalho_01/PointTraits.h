/*
Alunos: Ari Vargas Leal Filho, Cassiano Carvalho de Souza, Lucas Lacerda Arruda.

Na execução deste trabalho, todos os objetivos foram concluídos integralmente:
Construção da BBST de dimensão 1D: Lucas (Build e Query);
Construção da BBST de dimensão D (2D, 3D, ...): Cassiano (Build) e Ari (Query).
A implementação rodou e devolveu corretamente todas as respostas dos casos de testes executados.

Link do vídeo (YouTube): https://youtu.be/9mgXtaBgcpY
OBS: O vídeo tem mais de 13 minutos, pois explicamos em detalhes a lógica usada nas funções.
*/

#ifndef __PointTraits_h
#define __PointTraits_h

#include <cstddef>

namespace tcii::cg
{ // begin namespace tcii::cg

template <typename P> struct PointTraits;

template <typename P> 
inline constexpr size_t point_dim_v = PointTraits<P>::dim;

} // end namesapce tcii::cg

#endif // __PointTraits_h
