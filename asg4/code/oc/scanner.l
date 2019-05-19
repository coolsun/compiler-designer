/* $Id: lexer.l,v 1.9 2019-04-18 13:33:21-07 - - $ */

%{

#include "lyutils.h"

#define YY_USER_ACTION  { lexer::advance(); }

%}

%option 8bit
%option debug
%option nobackup
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option warn
/*%option verbose*/

TOK_IF         (if)
TOK_ELSE       (else)
TOK_WHILE      (while)
TOK_RETURN     (return)
TOK_ALLOC      (alloc)
TOK_NULLPTR    (nullptr)
TOK_NOT        (not)
TOK_INT        (int)
TOK_STRING     (string)
TOK_STRUCT     (struct)
TOK_ARRAY      (array)
TOK_PTR        (ptr)
TOK_VOID       (void)

LETTER         [A-Za-z_]
DIGIT          [0-9]
TOK_IDENT      ({LETTER}({LETTER}|{DIGIT})*)

TOK_CHARCON    ('([^\\'\n]|\\[\\'"0nt])')
TOK_STRINGCON  (\"([^\\"\n]|\\[\\'"0nt])*\")
TOK_INTCON     ({DIGIT}+)

BAD_IDENT      ({DIGIT}+{LETTER}({LETTER}|{DIGIT})*)
BAD_CHAR       ('([^\\'\n]|\\[\\'"0nt]?)[^']?)
BAD_STRING     (\"([^\\"\n]|\\[\\'"0nt])*[^\"]?)
%%

"#".*    { lexer::include(); lexer::printInclude();}
[ \t]+         { }
\n             { lexer::newline(); }

{TOK_INT}      { return lexer::token (TOK_INT); }
{TOK_IF}       { return lexer::token (TOK_IF); }
{TOK_ELSE}     { return lexer::token (TOK_ELSE); }
{TOK_WHILE}    { return lexer::token (TOK_WHILE); }
{TOK_RETURN}   { return lexer::token (TOK_RETURN); }
{TOK_ALLOC}    { return lexer::token (TOK_ALLOC); }
{TOK_NULLPTR}  { return lexer::token (TOK_NULLPTR); }
{TOK_NOT}      { return lexer::token (TOK_NOT); }
{TOK_STRING}   { return lexer::token (TOK_STRING); }
{TOK_STRUCT}   { return lexer::token (TOK_STRUCT); }
{TOK_ARRAY}    { return lexer::token (TOK_ARRAY); }
{TOK_PTR}      { return lexer::token (TOK_PTR); }
{TOK_VOID}     { return lexer::token (TOK_VOID); }

{TOK_IDENT}    { return lexer::token (TOK_IDENT); }
{TOK_INTCON}   { return lexer::token (TOK_INTCON); } 
{TOK_CHARCON}   { return lexer::token (TOK_CHARCON); } 
{TOK_STRINGCON} { return lexer::token (TOK_STRINGCON); }
"+"             { return lexer::token ('+'); }
"-"             { return lexer::token ('-'); }
"*"             { return lexer::token ('*'); }
"/"             { return lexer::token ('/'); }
"%"             { return lexer::token ('%'); }
"!"             { return lexer::token ('!'); }
"."             { return lexer::token ('.'); }
"="             { return lexer::token ('='); }
"=="            { return lexer::token (TOK_EQ); }
"!="            { return lexer::token (TOK_NE); }
"<"             { return lexer::token ('<'); }
"<="            { return lexer::token (TOK_LE); }
">"             { return lexer::token ('>'); }
">="            { return lexer::token (TOK_GE); }
"->"            { return lexer::token (TOK_PTR); }
","             { return lexer::token (','); }
";"             { return lexer::token (';'); }
"("             { return lexer::token ('('); }
")"             { return lexer::token (')'); }
"["             { return lexer::token ('['); }
"]"             { return lexer::token (']'); }
"{"             { return lexer::token ('{'); }
"}"             { return lexer::token ('}'); }
{BAD_STRING}    { lexer::badstring (yytext); }
{BAD_IDENT}     { lexer::badident (yytext); }
.               { lexer::badchar (*yytext); }

%%

