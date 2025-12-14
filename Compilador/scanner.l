%{
/*-----------------------------------------------------------------------------
 * SEÇÃO DE CÓDIGO C - Includes e Definições
 * 
 * Esta seção contém código C que será copiado diretamente para o arquivo
 * gerado pelo Flex. Aqui incluímos os headers necessários e definimos
 * macros importantes.
 *----------------------------------------------------------------------------*/

#include "ast.h"           /* Estruturas da Árvore Sintática Abstrata */
#include "parser.tab.h"    /* Tokens gerados pelo Bison (parser) */
#include <string.h>        /* Funções de manipulação de strings */

/*
 * yylineno: Variável global do Flex que mantém o número da linha atual.
 * Usamos extern porque ela é definida pelo próprio Flex.
 */
extern int yylineno;

/*
 * YY_USER_ACTION: Macro executada ANTES de cada ação do scanner.
 * Aqui atualizamos a localização (yylloc) para que o parser saiba
 * em qual linha cada token foi encontrado. Isso é essencial para
 * reportar erros com a linha correta.
 */
#define YY_USER_ACTION yylloc.first_line = yylineno;

%}

/*******************************************************************************
 * SEÇÃO DE DEFINIÇÕES - Expressões Regulares Nomeadas
 * 
 * Aqui definimos "atalhos" para expressões regulares que serão usadas
 * nas regras abaixo. Isso torna as regras mais legíveis e fáceis de manter.
 * 
 * digito        - Um único dígito de 0 a 9
 * letra         - Uma letra minúscula ou maiúscula
 * numero        - Um ou mais dígitos (número inteiro)
 * identificador - Letra seguida de letras ou dígitos
 * whitespace    - Um ou mais espaços ou tabs
 * quebralinha   - Uma ou mais quebras de linha
 ******************************************************************************/

digito        [0-9]
letra         [a-zA-Z]
numero        {digito}+
identificador {letra}({letra}|{digito})*
whitespace    [ \t]+
quebralinha   [\r\n]+

%%

    /***************************************************************************
     * SEÇÃO DE REGRAS - Padrões e Ações
     * 
     * Cada linha segue o formato: PADRÃO { AÇÃO }
     * 
     * Quando o scanner encontra texto que corresponde ao PADRÃO, ele executa
     * a AÇÃO associada. A ordem das regras importa: em caso de empate, a
     * primeira regra definida tem prioridade.
     * 
     * NOTA: Comentários nesta seção devem começar com espaço ou tab!
     ***************************************************************************/

    /* TRATAMENTO DE COMENTÁRIOS 
     * Comentários em C- usam o formato /* ... */
    /* O scanner lê caractere por caractere até encontrar a sequência */
"/*"          { char c1 = 0;
                char c2 = 0;
                do
                { c1 = c2;
                  c2 = input();
                  if (c2 == EOF) break;
                  if (c2 == '\n') yylineno++;
                } while (c1 != '*' || c2 != '/');
              }

    /* OPERADORES ARITMÉTICOS */
"+"           { return ADD; }
"-"           { return SUB; }
"*"           { return MUL; }
"/"           { return DIV; }

    /* OPERADORES RELACIONAIS (Comparação) */
"<="          { return LET; }
"<"           { return LT;  }
">="          { return GET; }
">"           { return GT;  }
"=="          { return EQ;  }
"!="          { return DIF; }

    /* OPERADOR DE ATRIBUIÇÃO E PONTUAÇÃO */
"="           { return ASSIGN; }
";"           { return SEMICOLON; }
","           { return COMMA; }

    /* DELIMITADORES (Parênteses, Colchetes, Chaves) */
"("           { return OPENPAR; }
")"           { return CLOSEPAR; }
"["           { return OPENCOL; }
"]"           { return CLOSECOL; }
"{"           { return OPENCHA; }
"}"           { return CLOSECHA; }

    /* PALAVRAS RESERVADAS (Keywords)
     * Devem vir ANTES das regras de identificador para ter prioridade */
"if"          { return IF; }
"else"        { return ELSE; }
"int"         { return INT; }
"return"      { return RETURN; }
"void"        { return VOID; }
"while"       { return WHILE; }
    
    /* NÚMERO: Sequência de dígitos
     * Passamos apenas o valor inteiro para o parser via yylval.ival
     */
{numero}      { yylval.ival = atoi(yytext); return NUM; }

    /* IDENTIFICADOR: Nome de variável ou função
     * Passamos a string (cópia) para o parser via yylval.sval
     */
{identificador} { yylval.sval = strdup(yytext); return ID; }

    /* CARACTERES IGNORADOS */
{whitespace}  { }
\n            { yylineno++; }

    /* ERRO LÉXICO - Caractere Não Reconhecido */
.             { printf("[Lexical Error] Line %d: Unrecognized character '%s'\n", yylineno, yytext); return ERRO; }

%%

/*******************************************************************************
 * SEÇÃO DE CÓDIGO DO USUÁRIO
 * 
 * Código C adicional que seria colocado após o scanner. Como o Flex já
 * fornece a função yywrap() por padrão, não precisamos defini-la aqui.
 ******************************************************************************/