/*
 * utilitarios.h
 * Funções de apoio: leitura do teclado, validações e formatação.
 */
#ifndef UTILITARIOS_H
#define UTILITARIOS_H

void lerTexto(const char *mensagem, char *destino, int tamanho);
int lerInteiro(const char *mensagem);
double lerNumero(const char *mensagem);

int soDigitos(const char *texto, int quantidade);
int cpfValido(const char *cpf);
int senhaValida(const char *senha);

void lerCpf(const char *mensagem, char *cpf);
void lerSenha(const char *mensagem, char *senha);

void formatarCpf(const char *cpf, char *saida);
void comVirgula(double valor, int casas, char *saida);
void dataHoraAtual(char *saida);

void linha(void);
void pausar(void);

#endif
