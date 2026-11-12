/*
 * utilitarios.c
 * Leitura do teclado, validações e formatação de valores.
 *
 * Todas as leituras usam fgets em vez de scanf. Assim o programa não
 * entra em loop se o usuário digitar uma letra onde se espera número,
 * e não sobra '\n' no buffer atrapalhando a próxima leitura.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "utilitarios.h"

/* Lê uma linha inteira e tira o '\n' do final */
void lerTexto(const char *mensagem, char *destino, int tamanho) {
    char buffer[256];

    printf("%s", mensagem);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        /* Fim da entrada (ex.: Ctrl+D). Não tem mais o que ler. */
        printf("\nEntrada encerrada.\n");
        exit(0);
    }
    buffer[strcspn(buffer, "\r\n")] = '\0';

    strncpy(destino, buffer, tamanho - 1);
    destino[tamanho - 1] = '\0';
}

/* Insiste até o usuário digitar um número inteiro */
int lerInteiro(const char *mensagem) {
    char texto[64];
    char *fim;
    long valor;

    while (1) {
        lerTexto(mensagem, texto, sizeof(texto));
        valor = strtol(texto, &fim, 10);
        if (fim != texto && *fim == '\0')
            return (int) valor;
        printf("Digite apenas numeros inteiros.\n");
    }
}

/* Lê um número com casas decimais. Aceita "10,50" ou "10.50" */
double lerNumero(const char *mensagem) {
    char texto[64];
    char *fim;
    double valor;
    int i;

    while (1) {
        lerTexto(mensagem, texto, sizeof(texto));
        for (i = 0; texto[i] != '\0'; i++) {
            if (texto[i] == ',')
                texto[i] = '.';
        }
        valor = strtod(texto, &fim);
        if (fim != texto && *fim == '\0')
            return valor;
        printf("Valor invalido. Exemplo: 150,75\n");
    }
}

/* Verifica se o texto tem exatamente "quantidade" caracteres, todos dígitos */
int soDigitos(const char *texto, int quantidade) {
    int i;

    if ((int) strlen(texto) != quantidade)
        return 0;
    for (i = 0; i < quantidade; i++) {
        if (!isdigit((unsigned char) texto[i]))
            return 0;
    }
    return 1;
}

int cpfValido(const char *cpf) {
    return soDigitos(cpf, 11);
}

int senhaValida(const char *senha) {
    return soDigitos(senha, 6);
}

/* Pede o CPF até vir no formato certo: 11 números, sem ponto nem traço */
void lerCpf(const char *mensagem, char *cpf) {
    char digitado[64];

    while (1) {
        lerTexto(mensagem, digitado, sizeof(digitado));
        if (cpfValido(digitado)) {
            strcpy(cpf, digitado);   /* só copia depois de validar */
            return;
        }
        printf("CPF invalido. Digite os 11 numeros, sem pontos e sem traco.\n");
    }
}

/* Pede a senha até vir com 6 dígitos numéricos */
void lerSenha(const char *mensagem, char *senha) {
    char digitada[64];

    while (1) {
        lerTexto(mensagem, digitada, sizeof(digitada));
        if (senhaValida(digitada)) {
            strcpy(senha, digitada);
            return;
        }
        printf("A senha precisa ter exatamente 6 numeros.\n");
    }
}

/* 12345678900 -> 123.456.789-00 */
void formatarCpf(const char *cpf, char *saida) {
    sprintf(saida, "%.3s.%.3s.%.3s-%.2s", cpf, cpf + 3, cpf + 6, cpf + 9);
}

/* Escreve o número com vírgula no lugar do ponto: 580.00 -> "580,00" */
void comVirgula(double valor, int casas, char *saida) {
    char *ponto;

    sprintf(saida, "%.*f", casas, valor);
    ponto = strchr(saida, '.');
    if (ponto != NULL)
        *ponto = ',';
}

/* Data e hora no formato do extrato: 01-10-2026 12:50 */
void dataHoraAtual(char *saida) {
    time_t agora = time(NULL);
    struct tm *t = localtime(&agora);

    strftime(saida, 20, "%d-%m-%Y %H:%M", t);
}

void linha(void) {
    printf("----------------------------------------\n");
}

void pausar(void) {
    char lixo[8];
    lerTexto("\nPressione ENTER para continuar...", lixo, sizeof(lixo));
}
