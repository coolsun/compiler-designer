// $Id: astree.h,v 1.10 2016-10-06 16:42:35-07 - - $

#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
using namespace std;

#include "auxlib.h"

struct location {
  size_t filenr;
  size_t linenr;
  size_t offset;
};

struct astree {

  // Fields.
  int symbol;               // token code
  location lloc;            // source location
  const string* lexinfo;    // pointer to lexical information
  vector<astree*> children; // children of this n-way node

  // Functions.
  astree(astree* that);
  astree(int symbol, const location&, const char* lexinfo);
  ~astree();
  astree* adopt(astree* child1, astree* child2 = nullptr,
      astree* child3 = nullptr);
  astree* promote (astree* child);
  astree* adopt_sym(astree* child, int symbol);
  void update (int token, const char* info = nullptr);
  astree* copySelfToChild(const char* info = nullptr);
  void dump_node(FILE*);
  void dump_tree(FILE*, int depth = 0);
  static void dump(FILE* outfile, astree* tree);
  static void print(FILE* outfile, astree* tree, int depth = 0);
};

// ssun24 add start program
//extern astree* yyparse_astree;
astree* change_sym(astree* root, int symbol);

void destroy(astree* tree1, astree* tree2 = nullptr,
    astree* tree3 = nullptr, astree* tree4 = nullptr,
    astree* tree5 = nullptr, astree* tree6 = nullptr);

void errllocprintf(const location&, const char* format, const char*);

#endif

