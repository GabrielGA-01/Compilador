%{
#include "stdio.h"
}%

digito [0-9]
letra [a-zA-Z]
numero {digito}+
identificador {letra}+

%%
"+" 			{return "+";}
"-"  			{return "-";}
"*" 			{return "*";}
"/" 			{return "/";}
"<"			{return "<";}
"<=" 			{return "<=";}
">=" 			{return ">=";}
">" 			{return ">";}
"=="			{return "==";}
"!="			{return "!=";}
"="			{return "=";}
";"			{return ";";}
"," 			{return ",";}
"(" 			{return "(";}
")" 			{return ")";}
"[" 			{return "[";}
"]" 			{return "]";}
"{" 			{return "{";}
"}" 			{return "}";}
"/*" 			{return "/*";}
"*/" 			{return "*/";}
numero			{return NUM;}
identificador		{return ID;}
%%

int main(int argc, char *argv[])
{
   FILE *f_in, *f_out;
  f_out = fopen("teste_s","w");  
  yyout = f_out;

  if (argc == 2)
  {
    if(f_in = fopen(argv[1],"r"))  yyin = f_in;
    else  perror(argv[0]);
  }
  else  yyin = stdin;

  currentToken = yylex();
  printf("%s", currentToken);
  
  return(0);
  
}

