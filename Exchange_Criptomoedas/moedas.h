/*
 * moedas.h
 * Funções sobre as criptomoedas: busca, listagem e cotação.
 */
#ifndef MOEDAS_H
#define MOEDAS_H

#include "tipos.h"

int buscarMoeda(const Exchange *ex, const char *sigla);
void mostrarCotacoes(const Exchange *ex);
int escolherMoeda(const Exchange *ex, const char *acao);
void atualizarCotacoes(Exchange *ex);

#endif
