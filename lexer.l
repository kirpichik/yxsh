%{

#include <stdlib.h>

#define YYSTYPE char *

#include "parser_yacc.h"

%}

%%

&                                     return BACKGROUND;
\>>                                   return ADD_FILE;
\>                                    return OUT_FILE;
\<                                    return IN_FILE;
\|                                    return PIPE;
;                                     return SEPARATOR;
\n                                    return NEWLINE;
[ \t]+                                /* Ignore blanks. */
[a-zA-Z0-9]+ yylval = strdup(yytext); return WORD;

%%

