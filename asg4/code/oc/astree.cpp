// $Id: astree.cpp,v 1.8 2019-04-21 17:13:03-07 - - $

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "astree.h"
#include "string_set.h"
#include "lyutils.h"

using namespace std;

astree::astree(astree* that) {
  symbol = that->symbol;
  lloc = that->lloc;
  lexinfo = that->lexinfo;
  // vector defaults to empty -- no children
}
astree::astree(int symbol_, const location& lloc_, const char* info) {
  symbol = symbol_;
  lloc = lloc_;
  lexinfo = string_set::intern(info);
  // vector defaults to empty -- no children
}

astree::~astree() {
  while (not children.empty()) {
    astree* child = children.back();
    children.pop_back();
    delete child;
  }
  if (yydebug) {
    fprintf(stderr, "Deleting astree (");
    astree::dump(stderr, this);
    fprintf(stderr, ")\n");
  }
}

astree* astree::adopt(astree* child1, astree* child2,
    astree* child3) {
  if (child1 != nullptr)
    children.push_back(child1);
  if (child2 != nullptr)
    children.push_back(child2);
  if (child3 != nullptr)
    children.push_back(child3);
  return this;
}

// ssun
// add this function to promote stmts to outer block
astree* astree::promote(astree* child) {
  if (child != nullptr)
    children.push_back(child);
  vector<astree*> thatChildren;
  while (child->children.size() > 0) {
    thatChildren.push_back(child->children.back());
    child->children.pop_back();
  }
  while (thatChildren.size() != 0) {
    children.push_back(thatChildren.back());
    thatChildren.pop_back();
  }
  return this;
}

astree* astree::adopt_sym(astree* child, int symbol_) {
  symbol = symbol_;
  return adopt(child);
}

void astree::dump_node(FILE* outfile) {
  fprintf(outfile, "%p->{%s %zd.%zd.%zd \"%s\":",
      static_cast<const void*>(this), parser::get_tname(symbol),
      lloc.filenr, lloc.linenr, lloc.offset, lexinfo->c_str());
  for (size_t child = 0; child < children.size(); ++child) {
    fprintf(outfile, " %p",
        static_cast<const void*>(children.at(child)));
  }
}

void astree::dump_tree(FILE* outfile, int depth) {
  fprintf(outfile, "%*s", depth * 3, "");
  dump_node(outfile);
  fprintf(outfile, "\n");
  for (astree* child : children)
    child->dump_tree(outfile, depth + 1);
  fflush(nullptr);
}

void astree::dump(FILE* outfile, astree* tree) {
  if (tree == nullptr)
    fprintf(outfile, "nullptr");
  else
    tree->dump_node(outfile);
}

void astree::print(FILE* outfile, astree* tree, int depth) {

  // ssun24
  // print '|' by level
  for (int i = 0; i < depth; ++i) {
    fprintf(outfile, "|  ");
  }
  // fprintf (outfile, "| %*s", depth * 3, "|");
  //fprintf(outfile, "; %*s", depth * 3, "");
  const char *tname = parser::get_tname(tree->symbol);
  if (strstr(tname, "TOK_") == tname)
    tname += 4;
  fprintf(outfile, "%s \"%s\" (%zd.%zd.%zd)\n", tname,
      tree->lexinfo->c_str(), tree->lloc.filenr, tree->lloc.linenr,
      tree->lloc.offset);
  for (astree* child : tree->children) {
    astree::print(outfile, child, depth + 1);
  }
}

/* ssun
 * update token and value
 */
void astree::update(int token, const char* info) {
  symbol = token;
  if (info != nullptr)
    lexinfo = string_set::intern(info);
}

/* ssun
 * copy the node to a child node
 * also set info to new value
 */
astree* astree::copySelfToChild(const char* info) {
  astree* newChild = new astree(this);
  if (info != nullptr)
    lexinfo = string_set::intern(info);
  this->adopt(newChild);
  return this;
}

astree* change_sym(astree* root, int symbol) {
  root->symbol = symbol;
  return root;
}

/* destroy
 * destroy up to four tree nodes.
 */
void destroy(astree* tree1, astree* tree2, astree* tree3,
    astree* tree4, astree* tree5, astree* tree6) {
  if (tree1 != nullptr)
    delete tree1;
  if (tree2 != nullptr)
    delete tree2;
  if (tree3 != nullptr)
    delete tree3;
  if (tree4 != nullptr)
    delete tree4;
  if (tree5 != nullptr)
    delete tree5;
  if (tree6 != nullptr)
    delete tree6;
}

void errllocprintf(const location& lloc, const char* format,
    const char* arg) {
  static char buffer[0x1000];
  assert(sizeof buffer > strlen(format) + strlen(arg));
  snprintf(buffer, sizeof buffer, format, arg);
  errprintf("%s:%zd.%zd: %s", lexer::filename(lloc.filenr),
      lloc.linenr, lloc.offset, buffer);
}
/* ssun24
 astree* new_parseroot() {
 return new astree (ROOT, {0, 0, 0}, "<<ROOT>>");
 */
