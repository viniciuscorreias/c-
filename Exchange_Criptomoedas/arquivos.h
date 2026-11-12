/*
 * arquivos.h
 * Tudo que lê e grava os arquivos .txt fica neste módulo.
 */
#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include "tipos.h"

void carregarDados(Exchange *ex);

void salvarInvestidores(const Exchange *ex);
void salvarAdmins(const Exchange *ex);
void salvarMoedas(const Exchange *ex);
void salvarCarteiras(const Exchange *ex);
void salvarTudo(const Exchange *ex);

void registrarNoExtrato(const Exchange *ex, int indiceInvestidor,
                        char sinal, double valor, const char *tipo,
                        double cotacao, double taxa);
void mostrarExtrato(const Exchange *ex, int indiceInvestidor);
void apagarExtrato(const char *cpf);

#endif
