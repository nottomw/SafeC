// Additional parser helper functions.

#include <stdio.h>

extern int yylineno;
extern int column;

void yyerror(const char *str)
{
    fflush(stdout);
    printf("\n\nPARSING ERROR: %s (line: %d, column: %d)\n\n", str, yylineno, column);
    fflush(stdout);
}
