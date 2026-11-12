/*
 * tipos.h
 * Estruturas e constantes usadas em todo o projeto da Exchange.
 */
#ifndef TIPOS_H
#define TIPOS_H

#define MAX_INVESTIDORES 100
#define MAX_ADMINS       10
#define MAX_MOEDAS       10

#define TAM_CPF    12   /* 11 dígitos + '\0' */
#define TAM_SENHA  7    /* 6 dígitos  + '\0' */
#define TAM_NOME   50
#define TAM_SIGLA  6
#define TAM_LINHA  512

/* Arquivos onde os dados ficam salvos */
#define ARQ_INVESTIDORES "investidores.txt"
#define ARQ_ADMINS       "admins.txt"
#define ARQ_MOEDAS       "moedas.txt"
#define ARQ_CARTEIRAS    "carteiras.txt"
/* O extrato de cada investidor fica em "extrato_<cpf>.txt" */

typedef struct {
    char sigla[TAM_SIGLA];   /* BTC, ETH, XRP... */
    char nome[TAM_NOME];     /* Bitcoin, Ethereum, Ripple... */
    double cotacao;          /* preço de 1 unidade em reais */
    double taxaCompra;       /* 0.02 = 2% */
    double taxaVenda;
} Moeda;

typedef struct {
    char cpf[TAM_CPF];
    char senha[TAM_SENHA];
    char nome[TAM_NOME];
    double reais;
    double cripto[MAX_MOEDAS];  /* cripto[i] = quanto ele tem da moeda i */
} Investidor;

typedef struct {
    char cpf[TAM_CPF];
    char senha[TAM_SENHA];
    char nome[TAM_NOME];
} Administrador;

/* Tudo que o programa precisa fica reunido aqui e é passado por ponteiro */
typedef struct {
    Investidor investidores[MAX_INVESTIDORES];
    int qtdInvestidores;

    Administrador admins[MAX_ADMINS];
    int qtdAdmins;

    Moeda moedas[MAX_MOEDAS];
    int qtdMoedas;
} Exchange;

#endif
