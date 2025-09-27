#ifndef TOKENS_H
#define TOKENS_H

/* * A enumeração define um nome simbólico para cada tipo de token.
 * O analisador léxico retornará esses valores, e o analisador sintático 
 * os usará para identificar os tokens recebidos.
 * Começamos em 257 porque os valores de 0 a 256 são reservados para 
 * caracteres ASCII. O valor 0 é usado por padrão para indicar "fim de arquivo".
*/
typedef enum {
    ADD = 257,
    SUB,
    MUL,
    DIV,
    LT,
    LET,
    GET,
    GT,
    EQ,
    DIF,
    ASSIGN,
    SEMICOLON,
    COLON,
    OPENPAR,
    CLOSEPAR,
    OPENCOL,
    CLOSECOL,
    OPENCHA,
    CLOSECHA,
    NUM,
    ID,
    ERROR,
    IF,
    ELSE,
    INT,
    RETURN,
    VOID,
    WHILE
} TokenType;

#endif // TOKENS_H