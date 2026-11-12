/*
 * investidor.h
 * Login e operações da área do investidor.
 */
#ifndef INVESTIDOR_H
#define INVESTIDOR_H

#include "tipos.h"

int buscarInvestidor(const Exchange *ex, const char *cpf);
void mostrarSaldo(const Exchange *ex, int indice);

int loginInvestidor(const Exchange *ex);
void menuInvestidor(Exchange *ex, int indice);

#endif
