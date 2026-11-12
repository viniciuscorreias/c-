/*
 * main.c
 * Exchange de Criptomoedas - Projeto CCP230 (Desenvolvimento de Algoritmos)
 *
 * Ponto de entrada do programa. Carrega os dados dos arquivos e mostra
 * a tela inicial, onde se escolhe entrar como investidor ou administrador.
 *
 * Para compilar:  gcc *.c -o exchange
 * Para executar:  ./exchange   (no Windows: exchange.exe)
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tipos.h"
#include "arquivos.h"
#include "investidor.h"
#include "admin.h"
#include "utilitarios.h"

/* A struct é grande, então fica fora da pilha da main */
static Exchange exchange;

int main(void) {
    int opcao, indice;

    srand((unsigned) time(NULL));   /* semente diferente a cada execução */
    carregarDados(&exchange);

    do {
        printf("\n========================================\n");
        printf("       EXCHANGE DE CRIPTOMOEDAS\n");
        printf("========================================\n");
        printf("1. Entrar como investidor\n");
        printf("2. Entrar como administrador\n");
        printf("0. Encerrar programa\n");
        opcao = lerInteiro("Escolha uma opcao: ");

        switch (opcao) {
            case 1:
                indice = loginInvestidor(&exchange);
                if (indice != -1)
                    menuInvestidor(&exchange, indice);
                break;
            case 2:
                if (loginAdmin(&exchange) != -1)
                    menuAdmin(&exchange);
                break;
            case 0:
                printf("Ate logo!\n");
                break;
            default:
                printf("Opcao invalida.\n");
        }
    } while (opcao != 0);

    salvarTudo(&exchange);
    return 0;
}
