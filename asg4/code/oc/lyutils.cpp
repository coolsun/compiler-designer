// $Id: lyutils.cpp,v 1.6 2019-04-18 13:35:11-07 - - $

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auxlib.h"
#include "lyutils.h"

// ssun24
// add tokFile to print include file ino
FILE* tokFile;

bool lexer::interactive = true;
location lexer::lloc = { 0, 1, 0 };
size_t lexer::last_yyleng = 0;
vector<string> lexer::filenames;

// ssun24 get_yytname from parser.y
const char * get_yytname (int symbol);

astree* parser::root = nullptr;

const string* lexer::filename(int filenr) {
  return &lexer::filenames.at(filenr);
}

void lexer::newfilename(const string& filename) {
  lexer::lloc.filenr = lexer::filenames.size();
  lexer::filenames.push_back(filename);
}

void lexer::advance() {
  if (not interactive) {
    if (lexer::lloc.offset == 0) {
      printf(";%2zd.%3zd: ", lexer::lloc.filenr, lexer::lloc.linenr);
    }
    printf("%s", yytext);
  }
  lexer::lloc.offset += last_yyleng;
  last_yyleng = yyleng;
}

void lexer::newline() {
  ++lexer::lloc.linenr;
  lexer::lloc.offset = 0;
}

void lexer::badchar(unsigned char bad) {
  char buffer[16];
  sprintf(buffer, isgraph(bad) ? "%c" : "\\%03o", bad);
  /*
   errllocprintf (lexer::lloc, "invalid source character (%s)\n",
   buffer);
   */
  errprintf("%:%s: %d: invalid source character (%s)\n",
      lexer::filenames.back().c_str(), lexer::lloc.linenr, buffer);
}

void lexer::badident(char* bad) {
  errprintf("%:%s: %d: invalid identifier (%s)\n",
      lexer::filenames.back().c_str(), lexer::lloc.linenr, bad);
}

void lexer::badstring(char* bad) {
  errprintf("%:%s: %d: invalid string (%s)\n",
      lexer::filenames.back().c_str(), lexer::lloc.linenr, bad);
}

void lexer::include() {
  size_t linenr;
  static char filename[0x1000];
  assert(sizeof filename > strlen(yytext));
  int scan_rc = sscanf(yytext, "# %zu \"%[^\"]\"", &linenr, filename);

  if (scan_rc != 2) {
    errprintf("%s: invalid directive, ignored\n", yytext);
  } else {
    if (yy_flex_debug) {
      fprintf(stderr, "--included # %zd \"%s\"\n", linenr, filename);
    }
    lexer::lloc.linenr = linenr - 1;
    lexer::newfilename(filename);
  }
}

void lexer::printInclude() {
  fprintf(tokFile, "#%3lu \"%s\"\n", lexer::lloc.filenr,
      lexer::filename(lexer::lloc.filenr)->c_str());
}

int lexer::token(int symbol) {
  yylval = new astree(symbol, lexer::lloc, yytext);

  // ssun24 print out tokens to token file
  fprintf(tokFile, "%3lu  %2lu.%03lu %10s %s\n",
      lexer::lloc.filenr, lexer::lloc.linenr,
      lexer::lloc.offset, get_yytname(symbol), yytext);

  return symbol;
}

int lexer::badtoken(int symbol) {
  errllocprintf(lexer::lloc, "invalid token (%s)\n", yytext);
  return lexer::token(symbol);
}

void yyerror(const char* message) {
  assert(not lexer::filenames.empty());
  errllocprintf(lexer::lloc, "%s\n", message);
}

