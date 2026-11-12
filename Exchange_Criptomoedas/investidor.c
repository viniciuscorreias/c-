/*
 * investidor.c
 * Tudo que o investidor pode fazer depois de entrar no sistema.
 */
#include <stdio.h>
#include <string.h>
#include "investidor.h"
#include "arquivos.h"
#include "moedas.h"
#include "utilitarios.h"

/* Margem para comparar números com casas decimais sem erro de arredondamento */
#define MARGEM 0.0000001

/* Devolve a posição do investidor no vetor, ou -1 se o CPF não existir */
int buscarInvestidor(const Exchange *ex, const char *cpf) {
    int i;

    for (i = 0; i < ex->qtdInvestidores; i++) {
        if (strcmp(ex->investidores[i].cpf, cpf) == 0)
            return i;
    }
    return -1;
}

void mostrarSaldo(const Exchange *ex, int indice) {
    const Investidor *inv = &ex->investidores[indice];
    char cpfFormatado[15], valor[40];
    int m;

    formatarCpf(inv->cpf, cpfFormatado);
    printf("\nNome: %s\n", inv->nome);
    printf("CPF: %s\n\n", cpfFormatado);

    comVirgula(inv->reais, 2, valor);
    printf("Reais: %s\n", valor);
    for (m = 0; m < ex->qtdMoedas; m++) {
        comVirgula(inv->cripto[m], 6, valor);
        printf("%s: %s\n", ex->moedas[m].nome, valor);
    }
}

/* Várias opções pedem a senha de novo antes de mostrar ou mexer na carteira */
static int confirmarSenha(const Exchange *ex, int indice) {
    char senha[TAM_SENHA];

    lerSenha("Confirme sua senha (6 digitos): ", senha);
    if (strcmp(senha, ex->investidores[indice].senha) == 0)
        return 1;

    printf("Senha incorreta.\n");
    return 0;
}

static int confirmar(const char *pergunta) {
    char resposta[8];

    lerTexto(pergunta, resposta, sizeof(resposta));
    return resposta[0] == 's' || resposta[0] == 'S';
}

/* ---------- Login ---------- */

int loginInvestidor(const Exchange *ex) {
    char cpf[TAM_CPF], senha[TAM_SENHA];
    int indice;

    printf("\n=== LOGIN DO INVESTIDOR ===\n");
    lerCpf("CPF (somente numeros): ", cpf);
    lerSenha("Senha (6 digitos): ", senha);

    indice = buscarInvestidor(ex, cpf);
    if (indice == -1 || strcmp(ex->investidores[indice].senha, senha) != 0) {
        printf("\nCPF ou senha incorretos.\n");
        return -1;
    }

    printf("\nBem-vindo(a), %s!\n", ex->investidores[indice].nome);
    return indice;
}

/* ---------- Operações ---------- */

static void consultarSaldo(const Exchange *ex, int indice) {
    if (confirmarSenha(ex, indice))
        mostrarSaldo(ex, indice);
}

static void consultarExtrato(const Exchange *ex, int indice) {
    if (confirmarSenha(ex, indice))
        mostrarExtrato(ex, indice);
}

static void depositar(Exchange *ex, int indice) {
    Investidor *inv = &ex->investidores[indice];
    double valor;

    valor = lerNumero("Valor do deposito (R$): ");
    if (valor <= 0) {
        printf("O valor precisa ser maior que zero.\n");
        return;
    }

    inv->reais += valor;
    salvarInvestidores(ex);
    registrarNoExtrato(ex, indice, '+', valor, "REAL", 0.0, 0.0);

    printf("\nDeposito realizado.\n");
    mostrarSaldo(ex, indice);
}

static void sacar(Exchange *ex, int indice) {
    Investidor *inv = &ex->investidores[indice];
    char saldo[30];
    double valor;

    comVirgula(inv->reais, 2, saldo);
    printf("Saldo disponivel: R$ %s\n", saldo);
    valor = lerNumero("Valor do saque (R$): ");

    if (valor <= 0) {
        printf("O valor precisa ser maior que zero.\n");
        return;
    }
    if (valor > inv->reais + MARGEM) {
        printf("Saldo insuficiente. O saldo nao pode ficar negativo.\n");
        return;
    }

    inv->reais -= valor;
    salvarInvestidores(ex);
    registrarNoExtrato(ex, indice, '-', valor, "REAL", 0.0, 0.0);

    printf("\nSaque realizado.\n");
    mostrarSaldo(ex, indice);
}

/*
 * Na compra o investidor diz quantos reais quer investir.
 * A taxa é cobrada por cima desse valor, então o total que sai da
 * carteira é valor + taxa. Ex.: investir R$ 100 em XRP (taxa 1%)
 * custa R$ 101 e rende 100 / cotação unidades de XRP.
 */
static void comprarCripto(Exchange *ex, int indice) {
    Investidor *inv = &ex->investidores[indice];
    Moeda *moeda;
    char texto[40];
    double valor, taxa, total, quantidade;
    int m;

    if (!confirmarSenha(ex, indice))
        return;

    m = escolherMoeda(ex, "comprar");
    if (m == -1)
        return;
    moeda = &ex->moedas[m];

    comVirgula(inv->reais, 2, texto);
    printf("\nSeu saldo: R$ %s\n", texto);
    valor = lerNumero("Quantos reais deseja investir? R$ ");
    if (valor <= 0) {
        printf("O valor precisa ser maior que zero.\n");
        return;
    }

    taxa = valor * moeda->taxaCompra;
    total = valor + taxa;
    quantidade = valor / moeda->cotacao;

    if (total > inv->reais + MARGEM) {
        comVirgula(total, 2, texto);
        printf("Saldo insuficiente: com a taxa, a compra custa R$ %s.\n", texto);
        return;
    }

    linha();
    comVirgula(quantidade, 6, texto);
    printf("Voce recebe: %s %s\n", texto, moeda->sigla);
    comVirgula(taxa, 2, texto);
    printf("Taxa (%.0f%%): R$ %s\n", moeda->taxaCompra * 100, texto);
    comVirgula(total, 2, texto);
    printf("Total debitado: R$ %s\n", texto);
    linha();

    if (!confirmar("Confirmar compra? (s/n): ")) {
        printf("Compra cancelada.\n");
        return;
    }

    inv->reais -= total;
    inv->cripto[m] += quantidade;
    salvarInvestidores(ex);
    salvarCarteiras(ex);
    registrarNoExtrato(ex, indice, '+', valor, moeda->sigla, moeda->cotacao, moeda->taxaCompra);

    printf("\nCompra realizada.\n");
    mostrarSaldo(ex, indice);
}

/*
 * Na venda o investidor diz quantas unidades quer vender.
 * Ele recebe (quantidade x cotação) menos a taxa de venda.
 */
static void venderCripto(Exchange *ex, int indice) {
    Investidor *inv = &ex->investidores[indice];
    Moeda *moeda;
    char texto[40];
    double quantidade, bruto, taxa, liquido;
    int m;

    if (!confirmarSenha(ex, indice))
        return;

    m = escolherMoeda(ex, "vender");
    if (m == -1)
        return;
    moeda = &ex->moedas[m];

    if (inv->cripto[m] <= 0) {
        printf("Voce nao tem %s para vender.\n", moeda->nome);
        return;
    }

    comVirgula(inv->cripto[m], 6, texto);
    printf("\nVoce tem: %s %s\n", texto, moeda->sigla);
    quantidade = lerNumero("Quantidade que deseja vender: ");
    if (quantidade <= 0) {
        printf("A quantidade precisa ser maior que zero.\n");
        return;
    }
    if (quantidade > inv->cripto[m] + MARGEM) {
        printf("Quantidade maior que o seu saldo. O saldo nao pode ficar negativo.\n");
        return;
    }

    bruto = quantidade * moeda->cotacao;
    taxa = bruto * moeda->taxaVenda;
    liquido = bruto - taxa;

    linha();
    comVirgula(bruto, 2, texto);
    printf("Valor da venda: R$ %s\n", texto);
    comVirgula(taxa, 2, texto);
    printf("Taxa (%.0f%%): R$ %s\n", moeda->taxaVenda * 100, texto);
    comVirgula(liquido, 2, texto);
    printf("Voce recebe: R$ %s\n", texto);
    linha();

    if (!confirmar("Confirmar venda? (s/n): ")) {
        printf("Venda cancelada.\n");
        return;
    }

    inv->cripto[m] -= quantidade;
    if (inv->cripto[m] < MARGEM)
        inv->cripto[m] = 0;   /* evita sobrar um "0,0000000001" */
    inv->reais += liquido;
    salvarInvestidores(ex);
    salvarCarteiras(ex);
    registrarNoExtrato(ex, indice, '-', bruto, moeda->sigla, moeda->cotacao, moeda->taxaVenda);

    printf("\nVenda realizada.\n");
    mostrarSaldo(ex, indice);
}

/* ---------- Menu ---------- */

void menuInvestidor(Exchange *ex, int indice) {
    int opcao;

    do {
        printf("\n=== MENU DO INVESTIDOR ===\n");
        printf("1. Consultar saldo\n");
        printf("2. Consultar extrato\n");
        printf("3. Depositar\n");
        printf("4. Sacar\n");
        printf("5. Comprar criptomoedas\n");
        printf("6. Vender criptomoedas\n");
        printf("7. Atualizar cotacao\n");
        printf("8. Sair\n");
        opcao = lerInteiro("Escolha uma opcao: ");

        switch (opcao) {
            case 1: consultarSaldo(ex, indice);   break;
            case 2: consultarExtrato(ex, indice); break;
            case 3: depositar(ex, indice);        break;
            case 4: sacar(ex, indice);            break;
            case 5: comprarCripto(ex, indice);    break;
            case 6: venderCripto(ex, indice);     break;
            case 7: atualizarCotacoes(ex);        break;
            case 8: printf("Saindo da conta...\n"); break;
            default: printf("Opcao invalida.\n");
        }

        if (opcao != 8)
            pausar();
    } while (opcao != 8);
}
