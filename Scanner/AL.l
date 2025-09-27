%{
#include <stdio.h>
#include "tokens.h"

const char* getTokenName(TokenType token);

%}

digito        [0-9]
letra         [a-zA-Z]
numero        {digito}+
identificador {letra}({letra}|{digito})*
whitespace    [ \t]+
quebralinha   [\r\n]+

%%

"/*"          { char c1;
                char c2;
                do
                { c1 = c2;
                  c2 = input();
                  if (c2 == EOF) break;
                } while (c1 != '*' || c2 != '/');
              }

"+"           { fprintf(yyout, "ADD ");       return ADD; }
"-"           { fprintf(yyout, "SUB ");       return SUB; }
"*"           { fprintf(yyout, "MUL ");       return MUL; }
"/"           { fprintf(yyout, "DIV ");       return DIV; }
"<="          { fprintf(yyout, "LET ");       return LET; }
"<"           { fprintf(yyout, "LT ");        return LT;  }
">="          { fprintf(yyout, "GET ");       return GET; }
">"           { fprintf(yyout, "GT ");        return GT;  }
"=="          { fprintf(yyout, "EQ ");        return EQ;  }
"!="          { fprintf(yyout, "DIF ");       return DIF; }
"="           { fprintf(yyout, "ASSIGN ");    return ASSIGN; }
";"           { fprintf(yyout, "SEMICOLON "); return SEMICOLON; }
","           { fprintf(yyout, "COLON ");     return COLON; }
"("           { fprintf(yyout, "OPENPAR ");   return OPENPAR; }
")"           { fprintf(yyout, "CLOSEPAR ");  return CLOSEPAR; }
"["           { fprintf(yyout, "OPENCOL ");   return OPENCOL; }
"]"           { fprintf(yyout, "CLOSECOL ");  return CLOSECOL; }
"{"           { fprintf(yyout, "OPENCHA ");   return OPENCHA; }
"}"           { fprintf(yyout, "CLOSECHA ");  return CLOSECHA; }

"if"          { fprintf(yyout, "IF ");        return IF; }
"else"        { fprintf(yyout, "ELSE ");      return ELSE; }
"int"         { fprintf(yyout, "INT ");       return INT; }
"return"      { fprintf(yyout, "RETURN ");    return RETURN; }
"void"        { fprintf(yyout, "VOID ");      return VOID; }
"while"       { fprintf(yyout, "WHILE ");     return WHILE; }
    
{numero}      { fprintf(yyout, "NUM ");       return NUM; }
{identificador} { fprintf(yyout, "ID ");        return ID;  }
{whitespace}  { }
{quebralinha} { ECHO; }
.             { fprintf(yyout, "ERROR ");     return ERROR; }

%%

int main(int argc, char *argv[]) {
  TokenType currentToken;
  FILE *f_in, *f_out;
  f_out = fopen("output","w");  
  yyout = f_out;

  if (argc == 2)
  {
    if(f_in = fopen(argv[1],"r"))  yyin = f_in;
    else  perror(argv[0]);
  }
  else  yyin = stdin;

  while ((currentToken = yylex())) {
      printf("Token: %-15s Lexema: \"%s\"\n", getTokenName(currentToken), yytext);
  }
  
  if (argc > 1) {
      fclose(yyin);
  }

  return 0;
}

const char* getTokenName(TokenType token) {
    switch (token) {
        case ADD: return "ADD";
        case SUB: return "SUB";
        case MUL: return "MUL";
        case DIV: return "DIV";
        case LT:  return "LT";
        case LET: return "LET";
        case GET: return "GET";
        case GT:  return "GT";
        case EQ:  return "EQ";
        case DIF: return "DIF";
        case ASSIGN: return "ASSIGN";
        case SEMICOLON: return "SEMICOLON";
        case COLON: return "COLON";
        case OPENPAR: return "OPENPAR";
        case CLOSEPAR: return "CLOSEPAR";
        case OPENCOL: return "OPENCOL";
        case CLOSECOL: return "CLOSECOL";
        case OPENCHA: return "OPENCHA";
        case CLOSECHA: return "CLOSECHA";
        case NUM: return "NUM";
        case ID:  return "ID";
        case ERROR: return "ERROR";
        default: return "UNKNOWN_TOKEN";
    }
}