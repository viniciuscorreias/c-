/*
 * admin.c
 * Área do administrador: cadastra e exclui investidores e moedas,
 * consulta qualquer carteira e atualiza as cotações.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "admin.h"
#include "investidor.h"
#include "arquivos.h"
#include "moedas.h"
#include "utilitarios.h"

static int confirmar(const char *pergunta) {
    char resposta[8];

    lerTexto(pergunta, resposta, sizeof(resposta));
    return resposta[0] == 's' || resposta[0] == 'S';
}

/* Pede o CPF e devolve a posição do investidor, ou -1 se não existir */
static int pedirInvestidor(const Exchange *ex) {
    char cpf[TAM_CPF];
    int indice;

    lerCpf("CPF do investidor (somente numeros): ", cpf);
    indice = buscarInvestidor(ex, cpf);
    if (indice == -1)
        printf("Nenhum investidor com esse CPF.\n");
    return indice;
}

/* ---------- Login ---------- */

int loginAdmin(const Exchange *ex) {
    char cpf[TAM_CPF], senha[TAM_SENHA];
    int i;

    printf("\n=== LOGIN DO ADMINISTRADOR ===\n");
    lerCpf("CPF (somente numeros): ", cpf);
    lerSenha("Senha (6 digitos): ", senha);

    for (i = 0; i < ex->qtdAdmins; i++) {
        if (strcmp(ex->admins[i].cpf, cpf) == 0 && strcmp(ex->admins[i].senha, senha) == 0) {
            printf("\nBem-vindo(a), %s!\n", ex->admins[i].nome);
            return i;
        }
    }
    printf("\nCPF ou senha incorretos.\n");
    return -1;
}

/* ---------- Investidores ---------- */

static void cadastrarInvestidor(Exchange *ex) {
    Investidor novo;

    if (ex->qtdInvestidores >= MAX_INVESTIDORES) {
        printf("Limite de investidores atingido.\n");
        return;
    }

    memset(&novo, 0, sizeof(Investidor));   /* começa com todos os saldos zerados */

    do {
        lerTexto("Nome: ", novo.nome, TAM_NOME);
        if (strlen(novo.nome) == 0 || strchr(novo.nome, ';') != NULL)
            printf("Nome invalido (nao pode ficar vazio nem ter ';').\n");
    } while (strlen(novo.nome) == 0 || strchr(novo.nome, ';') != NULL);

    lerCpf("CPF (somente numeros, sem pontos e traco): ", novo.cpf);
    if (buscarInvestidor(ex, novo.cpf) != -1) {
        printf("Ja existe um investidor com esse CPF.\n");
        return;
    }

    lerSenha("Senha (6 digitos): ", novo.senha);

    ex->investidores[ex->qtdInvestidores] = novo;
    ex->qtdInvestidores++;
    salvarInvestidores(ex);
    salvarCarteiras(ex);

    printf("\nInvestidor %s cadastrado.\n", novo.nome);
}

static void excluirInvestidor(Exchange *ex) {
    char cpf[TAM_CPF];
    int indice, i;

    indice = pedirInvestidor(ex);
    if (indice == -1)
        return;

    mostrarSaldo(ex, indice);
    if (!confirmar("\nDeseja realmente excluir este investidor? (s/n): ")) {
        printf("Exclusao cancelada.\n");
        return;
    }

    strcpy(cpf, ex->investidores[indice].cpf);

    /* Puxa todos os seguintes uma posição para trás, cobrindo o excluído */
    for (i = indice; i < ex->qtdInvestidores - 1; i++)
        ex->investidores[i] = ex->investidores[i + 1];
    ex->qtdInvestidores--;

    salvarInvestidores(ex);
    salvarCarteiras(ex);
    apagarExtrato(cpf);

    printf("Investidor excluido.\n");
}

/* ---------- Criptomoedas ---------- */

static void cadastrarMoeda(Exchange *ex) {
    Moeda nova;
    char sigla[32];
    int i, valida;

    if (ex->qtdMoedas >= MAX_MOEDAS) {
        printf("Limite de moedas atingido.\n");
        return;
    }

    do {
        lerTexto("Nome da moeda: ", nova.nome, TAM_NOME);
        valida = strlen(nova.nome) > 0 && strchr(nova.nome, ';') == NULL;
        if (!valida)
            printf("Nome invalido.\n");
    } while (!valida);

    do {
        lerTexto("Sigla (ate 5 letras, ex.: SOL): ", sigla, sizeof(sigla));
        valida = strlen(sigla) > 0 && strlen(sigla) < TAM_SIGLA;
        for (i = 0; sigla[i] != '\0'; i++) {
            if (!isalpha((unsigned char) sigla[i]))
                valida = 0;
            sigla[i] = (char) toupper((unsigned char) sigla[i]);
        }
        if (!valida)
            printf("Use de 1 a 5 letras.\n");
    } while (!valida);
    strcpy(nova.sigla, sigla);

    if (buscarMoeda(ex, nova.sigla) != -1 || strcmp(nova.sigla, "REAL") == 0) {
        printf("Ja existe uma moeda com essa sigla.\n");
        return;
    }

    do {
        nova.cotacao = lerNumero("Cotacao inicial (R$): ");
    } while (nova.cotacao <= 0);

    do {
        nova.taxaCompra = lerNumero("Taxa de compra (%): ") / 100;
    } while (nova.taxaCompra < 0 || nova.taxaCompra >= 1);

    do {
        nova.taxaVenda = lerNumero("Taxa de venda (%): ") / 100;
    } while (nova.taxaVenda < 0 || nova.taxaVenda >= 1);

    /* A nova moeda entra no fim do vetor; todo mundo começa com 0 dela */
    for (i = 0; i < ex->qtdInvestidores; i++)
        ex->investidores[i].cripto[ex->qtdMoedas] = 0;
    ex->moedas[ex->qtdMoedas] = nova;
    ex->qtdMoedas++;

    salvarMoedas(ex);
    salvarCarteiras(ex);
    printf("\nMoeda %s (%s) cadastrada.\n", nova.nome, nova.sigla);
}

static void excluirMoeda(Exchange *ex) {
    int m, i, j;

    m = escolherMoeda(ex, "excluir");
    if (m == -1)
        return;

    /* Não deixa apagar uma moeda que alguém ainda tem, senão o dinheiro sumiria */
    for (i = 0; i < ex->qtdInvestidores; i++) {
        if (ex->investidores[i].cripto[m] > 0) {
            printf("Nao e possivel excluir: %s ainda tem %s na carteira.\n",
                   ex->investidores[i].nome, ex->moedas[m].nome);
            return;
        }
    }

    if (!confirmar("Deseja realmente excluir esta moeda? (s/n): ")) {
        printf("Exclusao cancelada.\n");
        return;
    }

    /* Remove do vetor de moedas e, em cada carteira, da mesma posição */
    for (j = m; j < ex->qtdMoedas - 1; j++)
        ex->moedas[j] = ex->moedas[j + 1];
    for (i = 0; i < ex->qtdInvestidores; i++) {
        for (j = m; j < ex->qtdMoedas - 1; j++)
            ex->investidores[i].cripto[j] = ex->investidores[i].cripto[j + 1];
        ex->investidores[i].cripto[ex->qtdMoedas - 1] = 0;
    }
    ex->qtdMoedas--;

    salvarMoedas(ex);
    salvarCarteiras(ex);
    printf("Moeda excluida.\n");
}

/* ---------- Consultas ---------- */

static void saldoDeInvestidor(const Exchange *ex) {
    int indice = pedirInvestidor(ex);
    if (indice != -1)
        mostrarSaldo(ex, indice);
}

static void extratoDeInvestidor(const Exchange *ex) {
    int indice = pedirInvestidor(ex);
    if (indice != -1)
        mostrarExtrato(ex, indice);
}

/* ---------- Menu ---------- */

void menuAdmin(Exchange *ex) {
    int opcao;

    do {
        printf("\n=== MENU DO ADMINISTRADOR ===\n");
        printf("1. Cadastrar novo investidor\n");
        printf("2. Excluir investidor\n");
        printf("3. Cadastrar nova criptomoeda\n");
        printf("4. Excluir criptomoeda\n");
        printf("5. Consultar saldo de um investidor\n");
        printf("6. Consultar extrato de um investidor\n");
        printf("7. Atualizar cotacao das criptomoedas\n");
        printf("8. Sair\n");
        opcao = lerInteiro("Escolha uma opcao: ");

        switch (opcao) {
            case 1: cadastrarInvestidor(ex); break;
            case 2: excluirInvestidor(ex);   break;
            case 3: cadastrarMoeda(ex);      break;
            case 4: excluirMoeda(ex);        break;
            case 5: saldoDeInvestidor(ex);   break;
            case 6: extratoDeInvestidor(ex); break;
            case 7: atualizarCotacoes(ex);   break;
            case 8: printf("Saindo da conta...\n"); break;
            default: printf("Opcao invalida.\n");
        }

        if (opcao != 8)
            pausar();
    } while (opcao != 8);
}
