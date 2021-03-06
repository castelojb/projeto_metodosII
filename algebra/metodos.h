#ifndef METODOS_H
#define METODOS_H

#include <iostream>
#include <cmath>
#include "algebra.h"

namespace met{

/*************************************************************************************************************************
------------------------------------------------------- Retornos ---------------------------------------------------------
*************************************************************************************************************************/
	
	struct matrizLU{
		alg::matriz L, U;
		
		inline void operator = (matrizLU &LU);
	};
	
	struct autoValVet{
		float autoValor;
		alg::vetor autoVetor;

		inline void operator = (autoValVet &copia);
		inline bool operator == (autoValVet &valVet2);
	};

/*************************************************************************************************************************
-------------------------------------------------------- Funções ---------------------------------------------------------
*************************************************************************************************************************/
	
	inline autoValVet potencia (alg::matriz &mat, float erro);
	inline autoValVet potenciaInversa (alg::matriz &mat, float erro);
	inline std::vector<autoValVet> potenciaDeslocamento (alg::matriz &mat, float erro);
	inline matrizLU construirLU (alg::matriz &mat);
	inline alg::vetor substituicaoProgressiva (alg::matriz &mat, alg::vetor vet);
	inline alg::vetor substituicaoRetroativa (alg::matriz &mat, alg::vetor vet);
	inline alg::matriz inversaLU (matrizLU &matLU);

/*************************************************************************************************************************
---------------------------------------------------- Implementações ------------------------------------------------------
*************************************************************************************************************************/
	
	/*! Copia  estrutura de matrizes matrizes LU
	**  Parâmetros: A estrutura das matrizes LU que se deseja copiar
	**  Retorno: void
	*/
	void matrizLU::operator = (matrizLU &LU) {
		L = (LU.L);
		U = (LU.U);
	}

	/*! Copia as estrutura de autovalor e autovetor
	**  Parâmetros: A estrutura de autovalor e autovetor que se deseja copiar
	**  Retorno: void
	*/
	void autoValVet::operator = (autoValVet &copia) {
		int tam = copia.autoVetor.tam;
		autoValor = copia.autoValor;

		autoVetor = copia.autoVetor;
	}

	/*! Compara se duas estruturas de autovalor e autovetor tem os mesmos valores 
	**  Parâmetros: A estrutura de autovalor e autovetor que se deseja comparar
	**  Retorno: Verdadeiro se forem iguais, falso e forem diferentes
	*/
	bool autoValVet::operator == (autoValVet &valVet2) {
		int tam = autoVetor.tam;
		bool vetIgual = true;

		for(int i = 0; i < tam; i++){
			if(autoVetor[i] != valVet2.autoVetor[i]){
				vetIgual = false;
			}
		}

		if(vetIgual && autoValor == valVet2.autoValor){
			return true;
		}else{
			return false;
		}
	}
	
	/*! Metodo da potencia
	**  Parâmetros: A matriz para se calcular e o erro
	**  Retorno: O resultado do autovalor e autovetor na estrutura
	*/
	autoValVet potencia (alg::matriz &mat, float erro) {
		unsigned int tam = mat.tam;
		float *vet = (float*)malloc(sizeof(float) * mat.tam),
		      respoVal = INFINITY, anterior;
		
		vet[0] = 1;
		
		for(unsigned int i = 1; i < tam; i++){
			vet[i] = 0;
		}
		
		alg::vetor respoVet(mat.tam, vet), aux(mat.tam);
		
		do{
			anterior = respoVal;
			
			aux = mat * respoVet;
			respoVal = respoVet * aux;
			
			respoVet = aux;
			respoVet.unitario();
			
		}while((std::abs((respoVal - anterior) / respoVal)) > erro);
		
		autoValVet respo{respoVal, respoVet};
		
		return respo;
	}

	/*! Metodo da potencia inversa
	**  Parâmetros: A matriz para se calcular e o erro
	**  Retorno: O resultado do autovalor e autovetor na estrutura
	*/
	inline autoValVet potenciaInversa (alg::matriz &mat, float erro) {
		matrizLU LU = construirLU(mat);
		alg::matriz inversa = inversaLU(LU);
		autoValVet respo = potencia(inversa, erro);

		respo.autoValor = 1 / respo.autoValor;
		return respo;
	}

	/*! Metodo da potencia com deslocamento
	**  Parâmetros: A matriz para se calcular e o erro
	**  Retorno: O vetor com os resultados dos autovalores e autovetores nas estruturas
	*/
	std::vector<autoValVet> potenciaDeslocamento (alg::matriz &mat, float erro) {
		int tam = mat.tam;
		float intervalo;
		bool diferente;
		alg::matriz aux = mat;
		std::vector<autoValVet> respo;

		respo.push_back(potencia(mat, erro));
		respo.push_back(potenciaInversa(mat, erro));

		intervalo = (respo[0].autoValor - respo[1].autoValor) / tam;

		for(float i = respo[1].autoValor; (i < respo[0].autoValor) && (respo.size()) < tam; i += intervalo){
			aux - intervalo;

			autoValVet veri = potenciaInversa(aux, erro);

			diferente = true;
			for(int j = 0; j < respo.size() && diferente; j++){
				if(veri == respo[i]){
					diferente = false;
				}
			}

			if(diferente){
				respo.push_back(veri);
			}
		}

		return respo;
	}

	/*! Constroi as matrizes de decomposição LU
	**  Parâmetros: A matriz que se deseja decompor
	**  Retorno: A estrutura de matrizes de decomposição LU
	*/
	matrizLU construirLU (alg::matriz &mat) {
		unsigned int tam = mat.tam;
		alg::matriz L(tam, alg::identidade(mat.tam)), U(tam, alg::copiarMatriz(mat));
		float subtrair;
		
		for(unsigned int i = 0; i < tam; i++){
			for(unsigned int j = i + 1; j < tam; j++){
				subtrair = L[j][i] = (U[j][i] / U[i][i]);
				
				for(unsigned int k = i; k < tam; k++){
					U[j][k] -= subtrair * U[i][k];
				}

			}
		}
		
		matrizLU LU{L, U};
		
		return LU;
	}
	
	/*! Realiza o metodo da substituição seletiva para resolver Ax=b
	**  Parâmetros: A matriz A e o vetor b
	**  Retorno: O vetor x
	*/
	alg::vetor substituicaoProgressiva (alg::matriz &mat, alg::vetor vet) {
		if(mat.tam == vet.tam){
			int tam = vet.tam;
			alg::vetor resul(tam);
			
			for(int i = 0; i < tam; i++){

				for(int j = 0; j < i; j++){
					resul[i] += mat[i][j] * resul[j];
				}
				
				resul[i] = (vet[i] - resul[i])/ mat[i][i];
			
			}
			
			return resul;
		}
	}
	
	/*! Realiza o método da substuição retroativa para resolver Ax=b
	**  Parâmetros: A matriz A e o vetor b
	**  Retorno: O vetor x
	*/
	alg::vetor substituicaoRetroativa (alg::matriz &mat, alg::vetor vet) {
		if(mat.tam == vet.tam){
			int tam = vet.tam;
			alg::vetor resul(tam);
			
			for(int i = tam-1; i >= 0; i--){

				for(int j = tam-1; j > i; j--){
					resul[i] += mat[i][j] * resul[j];
				}
				
				resul[i] = (vet[i] - resul[i])/ mat[i][i];
			
			}
			
			return resul;
		}
	}
	
	/*! Acha a matriz inversa utilizando as matrizes de decomposição LU
	**  Parâmetros: A estrutura de de matrizes de decomposição LU
	**  Retorno: A matriz inversa
	*/
	alg::matriz inversaLU (matrizLU &matLU) {
		int tam = matLU.L.tam;
		alg::vetor aux(tam), iden(tam);
		alg::matriz matRespo(tam);

		for(int i = 0; i < tam; i++){
			iden[i] = 1;
			if(i-1 >= 0){
				iden[i-1] = 0;
			}
			
			aux = substituicaoRetroativa(matLU.U, substituicaoProgressiva(matLU.L, iden));

			for(int j = 0; j < tam; j++){
				matRespo[j][i] = aux[j];
			}

		}
		return matRespo;
	}
}

#endif //METODOS_H
