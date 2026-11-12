/*
 * moedas.c
 * Busca, listagem e atualização da cotação das criptomoedas.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "moedas.h"
#include "arquivos.h"
#include "utilitarios.h"

/* Devolve a posição da moeda no vetor, ou -1 se ela não existir.
   A comparação ignora maiúsculas e minúsculas (btc == BTC). */
int buscarMoeda(const Exchange *ex, const char *sigla) {
    int i, j;

    for (i = 0; i < ex->qtdMoedas; i++) {
        const char *atual = ex->moedas[i].sigla;
        for (j = 0; atual[j] != '\0' && sigla[j] != '\0'; j++) {
            if (toupper((unsigned char) atual[j]) != toupper((unsigned char) sigla[j]))
                break;
        }
        if (atual[j] == '\0' && sigla[j] == '\0')
            return i;
    }
    return -1;
}

void mostrarCotacoes(const Exchange *ex) {
    char preco[30];
    int i;

    printf("\n   Moeda        Cotacao (R$)     Compra  Venda\n");
    for (i = 0; i < ex->qtdMoedas; i++) {
        const Moeda *m = &ex->moedas[i];
        comVirgula(m->cotacao, 2, preco);
        printf("%d) %-10s %15s     %4.1f%%  %4.1f%%\n", i + 1, m->nome, preco,
               m->taxaCompra * 100, m->taxaVenda * 100);
    }
}

/* Mostra as moedas numeradas e devolve o índice escolhido (ou -1 para cancelar) */
int escolherMoeda(const Exchange *ex, const char *acao) {
    char mensagem[80];
    int opcao;

    mostrarCotacoes(ex);
    printf("0) Cancelar\n");
    sprintf(mensagem, "\nQual moeda deseja %s? ", acao);

    while (1) {
        opcao = lerInteiro(mensagem);
        if (opcao == 0)
            return -1;
        if (opcao >= 1 && opcao <= ex->qtdMoedas)
            return opcao - 1;
        printf("Opcao invalida.\n");
    }
}

/*
 * Cada moeda varia aleatoriamente entre -5% e +5% do valor anterior.
 * rand() % 1001 dá um inteiro de 0 a 1000. Dividindo por 10000 fica
 * de 0.0000 a 0.1000, e tirando 0.05 fica de -0.05 a +0.05.
 */
void atualizarCotacoes(Exchange *ex) {
    char antes[30], depois[30];
    double variacao;
    int i;

    printf("\nAtualizando cotacoes...\n\n");
    for (i = 0; i < ex->qtdMoedas; i++) {
        Moeda *m = &ex->moedas[i];

        variacao = (rand() % 1001) / 10000.0 - 0.05;
        comVirgula(m->cotacao, 2, antes);
        m->cotacao = m->cotacao * (1 + variacao);
        comVirgula(m->cotacao, 2, depois);

        printf("%-10s R$ %14s  ->  R$ %14s  (%+.2f%%)\n",
               m->nome, antes, depois, variacao * 100);
    }
    salvarMoedas(ex);
}
