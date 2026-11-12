/*
 * arquivos.c
 * Leitura e gravação dos dados em arquivos de texto.
 *
 * Cada arquivo guarda um registro por linha, com os campos separados
 * por ponto e vírgula. Exemplo de investidores.txt:
 *
 *     12345678900;123456;Gabriela Biondi;580.00
 *
 * A estratégia é simples: ao abrir o programa tudo é carregado para a
 * memória (struct Exchange). Toda vez que algo muda, o arquivo
 * correspondente é regravado por inteiro. Assim nada se perde se o
 * programa for fechado.
 */
#include <stdio.h>
#include <string.h>
#include "arquivos.h"
#include "moedas.h"
#include "utilitarios.h"

/* ---------- Dados de exemplo, usados só na primeira execução ---------- */

static void criarMoedasPadrao(Exchange *ex) {
    Moeda padrao[3] = {
        {"BTC", "Bitcoin",  350000.00, 0.02, 0.03},
        {"ETH", "Ethereum",  16108.00, 0.01, 0.02},
        {"XRP", "Ripple",        2.57, 0.01, 0.01}
    };
    int i;

    for (i = 0; i < 3; i++)
        ex->moedas[i] = padrao[i];
    ex->qtdMoedas = 3;
    salvarMoedas(ex);
}

static void criarAdminPadrao(Exchange *ex) {
    strcpy(ex->admins[0].cpf, "00000000000");
    strcpy(ex->admins[0].senha, "000000");
    strcpy(ex->admins[0].nome, "Administrador");
    ex->qtdAdmins = 1;
    salvarAdmins(ex);
}

static void criarInvestidorPadrao(Exchange *ex) {
    Investidor *novo = &ex->investidores[0];

    memset(novo, 0, sizeof(Investidor));
    strcpy(novo->cpf, "12345678900");
    strcpy(novo->senha, "123456");
    strcpy(novo->nome, "Gabriela Biondi");
    novo->reais = 0.0;
    ex->qtdInvestidores = 1;
    salvarInvestidores(ex);
    salvarCarteiras(ex);
}

/* ---------- Carregamento ---------- */

static void carregarMoedas(Exchange *ex) {
    FILE *arq = fopen(ARQ_MOEDAS, "r");
    char linhaArq[TAM_LINHA];
    Moeda m;

    ex->qtdMoedas = 0;
    if (arq == NULL) {
        criarMoedasPadrao(ex);
        return;
    }
    while (fgets(linhaArq, sizeof(linhaArq), arq) != NULL && ex->qtdMoedas < MAX_MOEDAS) {
        if (sscanf(linhaArq, "%5[^;];%49[^;];%lf;%lf;%lf",
                   m.sigla, m.nome, &m.cotacao, &m.taxaCompra, &m.taxaVenda) == 5) {
            ex->moedas[ex->qtdMoedas] = m;
            ex->qtdMoedas++;
        }
    }
    fclose(arq);
}

static void carregarAdmins(Exchange *ex) {
    FILE *arq = fopen(ARQ_ADMINS, "r");
    char linhaArq[TAM_LINHA];
    Administrador a;

    ex->qtdAdmins = 0;
    if (arq == NULL) {
        criarAdminPadrao(ex);
        return;
    }
    while (fgets(linhaArq, sizeof(linhaArq), arq) != NULL && ex->qtdAdmins < MAX_ADMINS) {
        if (sscanf(linhaArq, "%11[^;];%6[^;];%49[^\r\n]", a.cpf, a.senha, a.nome) == 3) {
            ex->admins[ex->qtdAdmins] = a;
            ex->qtdAdmins++;
        }
    }
    fclose(arq);
}

static void carregarInvestidores(Exchange *ex) {
    FILE *arq = fopen(ARQ_INVESTIDORES, "r");
    char linhaArq[TAM_LINHA];
    Investidor inv;

    ex->qtdInvestidores = 0;
    if (arq == NULL) {
        criarInvestidorPadrao(ex);
        return;
    }
    while (fgets(linhaArq, sizeof(linhaArq), arq) != NULL && ex->qtdInvestidores < MAX_INVESTIDORES) {
        memset(&inv, 0, sizeof(Investidor));   /* zera os saldos de cripto */
        if (sscanf(linhaArq, "%11[^;];%6[^;];%49[^;];%lf",
                   inv.cpf, inv.senha, inv.nome, &inv.reais) == 4) {
            ex->investidores[ex->qtdInvestidores] = inv;
            ex->qtdInvestidores++;
        }
    }
    fclose(arq);
}

/* carteiras.txt guarda uma linha por investidor e moeda: cpf;sigla;quantidade */
static void carregarCarteiras(Exchange *ex) {
    FILE *arq = fopen(ARQ_CARTEIRAS, "r");
    char linhaArq[TAM_LINHA];
    char cpf[TAM_CPF], sigla[TAM_SIGLA];
    double quantidade;
    int i, m;

    if (arq == NULL)
        return;   /* ninguém tem cripto ainda */

    while (fgets(linhaArq, sizeof(linhaArq), arq) != NULL) {
        if (sscanf(linhaArq, "%11[^;];%5[^;];%lf", cpf, sigla, &quantidade) != 3)
            continue;
        m = buscarMoeda(ex, sigla);
        if (m == -1)
            continue;
        for (i = 0; i < ex->qtdInvestidores; i++) {
            if (strcmp(ex->investidores[i].cpf, cpf) == 0) {
                ex->investidores[i].cripto[m] = quantidade;
                break;
            }
        }
    }
    fclose(arq);
}

void carregarDados(Exchange *ex) {
    /* As moedas vêm antes das carteiras, porque a carteira
       precisa saber em que posição está cada sigla. */
    carregarMoedas(ex);
    carregarAdmins(ex);
    carregarInvestidores(ex);
    carregarCarteiras(ex);
}

/* ---------- Gravação ---------- */

void salvarInvestidores(const Exchange *ex) {
    FILE *arq = fopen(ARQ_INVESTIDORES, "w");
    int i;

    if (arq == NULL) {
        printf("Erro: nao foi possivel gravar %s\n", ARQ_INVESTIDORES);
        return;
    }
    for (i = 0; i < ex->qtdInvestidores; i++) {
        const Investidor *inv = &ex->investidores[i];
        fprintf(arq, "%s;%s;%s;%.2f\n", inv->cpf, inv->senha, inv->nome, inv->reais);
    }
    fclose(arq);
}

void salvarAdmins(const Exchange *ex) {
    FILE *arq = fopen(ARQ_ADMINS, "w");
    int i;

    if (arq == NULL) {
        printf("Erro: nao foi possivel gravar %s\n", ARQ_ADMINS);
        return;
    }
    for (i = 0; i < ex->qtdAdmins; i++)
        fprintf(arq, "%s;%s;%s\n", ex->admins[i].cpf, ex->admins[i].senha, ex->admins[i].nome);
    fclose(arq);
}

void salvarMoedas(const Exchange *ex) {
    FILE *arq = fopen(ARQ_MOEDAS, "w");
    int i;

    if (arq == NULL) {
        printf("Erro: nao foi possivel gravar %s\n", ARQ_MOEDAS);
        return;
    }
    for (i = 0; i < ex->qtdMoedas; i++) {
        const Moeda *m = &ex->moedas[i];
        fprintf(arq, "%s;%s;%.2f;%.4f;%.4f\n",
                m->sigla, m->nome, m->cotacao, m->taxaCompra, m->taxaVenda);
    }
    fclose(arq);
}

void salvarCarteiras(const Exchange *ex) {
    FILE *arq = fopen(ARQ_CARTEIRAS, "w");
    int i, m;

    if (arq == NULL) {
        printf("Erro: nao foi possivel gravar %s\n", ARQ_CARTEIRAS);
        return;
    }
    for (i = 0; i < ex->qtdInvestidores; i++) {
        for (m = 0; m < ex->qtdMoedas; m++) {
            fprintf(arq, "%s;%s;%.8f\n", ex->investidores[i].cpf,
                    ex->moedas[m].sigla, ex->investidores[i].cripto[m]);
        }
    }
    fclose(arq);
}

void salvarTudo(const Exchange *ex) {
    salvarMoedas(ex);
    salvarAdmins(ex);
    salvarInvestidores(ex);
    salvarCarteiras(ex);
}

/* ---------- Extrato ---------- */

static void nomeArquivoExtrato(const char *cpf, char *nomeArquivo) {
    sprintf(nomeArquivo, "extrato_%s.txt", cpf);
}

/*
 * Acrescenta uma linha no extrato do investidor, no mesmo formato do
 * enunciado. Depois da operação, grava também os saldos de todas as
 * moedas, para dar para acompanhar a carteira linha a linha.
 */
void registrarNoExtrato(const Exchange *ex, int indiceInvestidor,
                        char sinal, double valor, const char *tipo,
                        double cotacao, double taxa) {
    const Investidor *inv = &ex->investidores[indiceInvestidor];
    char nomeArquivo[40], dataHora[20];
    FILE *arq;
    int m;

    nomeArquivoExtrato(inv->cpf, nomeArquivo);
    arq = fopen(nomeArquivo, "a");   /* "a" = acrescenta no final */
    if (arq == NULL) {
        printf("Erro: nao foi possivel gravar o extrato.\n");
        return;
    }

    dataHora[0] = '\0';
    dataHoraAtual(dataHora);
    fprintf(arq, "%s %c %10.2f %-4s CT: %-10.2f TX: %.2f REAL: %-10.2f",
            dataHora, sinal, valor, tipo, cotacao, taxa, inv->reais);
    for (m = 0; m < ex->qtdMoedas; m++)
        fprintf(arq, " %s: %-10.6f", ex->moedas[m].sigla, inv->cripto[m]);
    fprintf(arq, "\n");

    fclose(arq);
}

void mostrarExtrato(const Exchange *ex, int indiceInvestidor) {
    const Investidor *inv = &ex->investidores[indiceInvestidor];
    char nomeArquivo[40], cpfFormatado[15], linhaArq[TAM_LINHA];
    FILE *arq;

    formatarCpf(inv->cpf, cpfFormatado);
    printf("\nNome: %s\n", inv->nome);
    printf("CPF: %s\n\n", cpfFormatado);

    nomeArquivoExtrato(inv->cpf, nomeArquivo);
    arq = fopen(nomeArquivo, "r");
    if (arq == NULL) {
        printf("Nenhuma operacao registrada ainda.\n");
        return;
    }
    while (fgets(linhaArq, sizeof(linhaArq), arq) != NULL)
        printf("%s", linhaArq);
    fclose(arq);
}

void apagarExtrato(const char *cpf) {
    char nomeArquivo[40];

    nomeArquivoExtrato(cpf, nomeArquivo);
    remove(nomeArquivo);
}
