/* https://www.quut.com/c/ANSI-C-grammar-l-1995.html */

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%option yylineno

%{
#include <stdio.h>
#include <assert.h>
#include "SafecParser.yacc.hpp"

int column = 0;
int lex_current_char = 0;
int lex_keyword_start_index = 0;

void count(void);
void comment(void);
int check_type(void);
%}

%%
"/*"			{ count(); comment(); }
"//"[^\n]*      { count(); /* consume //-comment */ }
"#"[^\n]*		{ count(); /* consume preprocessor directives */ }

"auto"			{ count(); return(AUTO); }
"break"			{ lex_keyword_start_index = lex_current_char; count(); return(BREAK); }
"case"			{ count(); return(CASE); }
"char"			{ count(); return(CHAR); }
"const"			{ count(); return(CONST); }
"continue"		{ lex_keyword_start_index = lex_current_char; count(); return(CONTINUE); }
"default"		{ count(); return(DEFAULT); }
"do"			{ count(); return(DO); }
"double"		{ count(); return(DOUBLE); }
"else"			{ count(); return(ELSE); }
"enum"			{ count(); return(ENUM); }
"extern"		{ count(); return(EXTERN); }
"float"			{ count(); return(FLOAT); }
"for"			{ count(); return(FOR); }
"goto"			{ count(); return(GOTO); }
"if"			{ count(); return(IF); }
"int"			{ count(); return(INT); }
"long"			{ count(); return(LONG); }
"register"		{ count(); return(REGISTER); }
"return"		{ lex_keyword_start_index = lex_current_char; count(); return(RETURN); }
"short"			{ count(); return(SHORT); }
"signed"		{ count(); return(SIGNED); }
"sizeof"		{ count(); return(SIZEOF); }
"static"		{ count(); return(STATIC); }
"struct"		{ count(); return(STRUCT); }
"switch"		{ count(); return(SWITCH); }
"typedef"		{ count(); return(TYPEDEF); }
"union"			{ count(); return(UNION); }
"unsigned"		{ count(); return(UNSIGNED); }
"void"			{ count(); return(VOID); }
"volatile"		{ count(); return(VOLATILE); }
"while"			{ count(); return(WHILE); }
"defer"			{ lex_keyword_start_index = lex_current_char; count(); return(SAFEC_DEFER); }

{L}({L}|{D})*		{ count(); yylval.tokenStrValue = strdup(yytext); return(check_type()); }

0[xX]{H}+{IS}?          { count(); yylval.tokenStrValue = strdup(yytext); return(CONSTANT); }
0{D}+{IS}?              { count(); yylval.tokenStrValue = strdup(yytext); return(CONSTANT); }
{D}+{IS}?               { count(); yylval.tokenStrValue = strdup(yytext); return(CONSTANT); }
L?'(\\.|[^\\'])+'       { count(); yylval.tokenStrValue = strdup(yytext); return(CONSTANT); }

{D}+{E}{FS}?            { count(); yylval.tokenStrValue = strdup(yytext); return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ count(); yylval.tokenStrValue = strdup(yytext); return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ count(); yylval.tokenStrValue = strdup(yytext); return(CONSTANT); }

L?\"(\\.|[^\\"])*\"     { count(); yylval.tokenStrValue = strdup(yytext); return(STRING_LITERAL); }

"..."			{ count(); return(ELLIPSIS); }
">>="			{ count(); return(RIGHT_ASSIGN); }
"<<="			{ count(); return(LEFT_ASSIGN); }
"+="			{ count(); return(ADD_ASSIGN); }
"-="			{ count(); return(SUB_ASSIGN); }
"*="			{ count(); return(MUL_ASSIGN); }
"/="			{ count(); return(DIV_ASSIGN); }
"%="			{ count(); return(MOD_ASSIGN); }
"&="			{ count(); return(AND_ASSIGN); }
"^="			{ count(); return(XOR_ASSIGN); }
"|="			{ count(); return(OR_ASSIGN); }
">>"			{ count(); return(RIGHT_OP); }
"<<"			{ count(); return(LEFT_OP); }
"++"			{ count(); return(INC_OP); }
"--"			{ count(); return(DEC_OP); }
"->"			{ count(); return(PTR_OP); }
"&&"			{ count(); return(AND_OP); }
"||"			{ count(); return(OR_OP); }
"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
";"			{ count(); return(';'); }
("{"|"<%")		{ count(); return('{'); }
("}"|"%>")		{ count(); return('}'); }
","			{ count(); return(','); }
":"			{ count(); return(':'); }
"="			{ count(); return('='); }
"("			{ count(); return('('); }
")"			{ count(); return(')'); }
("["|"<:")		{ count(); return('['); }
("]"|":>")		{ count(); return(']'); }
"."			{ count(); return('.'); }
"&"			{ count(); return('&'); }
"!"			{ count(); return('!'); }
"~"			{ count(); return('~'); }
"-"			{ count(); return('-'); }
"+"			{ count(); return('+'); }
"*"			{ count(); return('*'); }
"/"			{ count(); return('/'); }
"%"			{ count(); return('%'); }
"<"			{ count(); return('<'); }
">"			{ count(); return('>'); }
"^"			{ count(); return('^'); }
"|"			{ count(); return('|'); }
"?"			{ count(); return('?'); }

[ \t\v\n\f]		{ count(); }
.			{ assert(NULL == "bad character"); }

%%

int yywrap(void)
{
	return 1;
}

void comment(void)
{
 	char c, c1;
    int comment_length = 0;
 
 loop:
 	while ((c = input()) != '*' && c != 0)
	{
        comment_length += 1;
 		putchar(c);
	}

	if (c != 0)
	{
        comment_length += 1;
		putchar(c);
	}
 
 	if ((c1 = input()) != '/' && c != 0)
 	{
        comment_length -= 1;
 		unput(c1);
 		goto loop;
 	}
 
 	if (c != 0)
	{
        comment_length += 1;
 		putchar(c1);
	}

    lex_current_char += comment_length;
}

void count(void)
{
	int i;
	for (i = 0; yytext[i] != '\0'; i++)
	{
		if (yytext[i] == '\n')
		{
			column = 0;
		}
		else if (yytext[i] == '\t')
		{
			column += 8 - (column % 8);
		}
		else
		{
			column++;
		}
	}

	lex_current_char += i;

	// display the token
	ECHO;
}

int check_type(void)
{
/*
* pseudo code --- this is what it should check
*
*	if (yytext == type_name)
*		return(TYPE_NAME);
*
*	return(IDENTIFIER);
*/

/*
*	it actually will only return IDENTIFIER
*/

	return(IDENTIFIER);
}
