// $Id: lyutils.h,v 1.7 2019-04-18 13:33:21-07 - - $

#ifndef __UTILS_H__
#define __UTILS_H__

// Lex and Yacc interface utility.

#include <string>
#include <vector>
using namespace std;

#include <stdio.h>

#include "astree.h"
#include "auxlib.h"

#define YYEOF 0

extern FILE* yyin;
extern char* yytext;
extern int yy_flex_debug;
extern int yydebug;
extern int yyleng;

// ssun24 add start program
extern astree* yyparse_astree;

bool is_defined_token (int symbol);
const char* get_yytname (int symbol);

int yylex();
int yylex_destroy();
int yyparse();
void yyerror(const char* message);

struct lexer {
  static bool interactive;
  static location lloc;
  static size_t last_yyleng;
  static vector<string> filenames;
  static const string* filename(int filenr);
  static void newfilename(const string& filename);
  static void advance();
  static void newline();
  static void badchar(unsigned char bad);
  static void badident(char* bad);
  static void badstring(char* bad);
  static void include();

  // ssun24
  // function to print inclule file
  static void printInclude();

  static int token(int symbol);
  static int badtoken(int symbol);
};

struct parser {
  static astree* root;
  static const char* get_tname(int symbol);
};

#define YYSTYPE_IS_DECLARED
typedef astree* YYSTYPE;
#include "yyparse.h"

#endif

