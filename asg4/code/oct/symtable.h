// $Id: astree.h,v 1.7 2016-10-06 16:13:39-07 - - $

#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#include <string>
#include <vector>
#include <bitset>
#include <unordered_map>
#include <iostream>
using namespace std;

#define GLOBAL 0

struct astree;
struct symbol;
struct location;

using namespace std;

enum { 
   ATTR_void, ATTR_int, ATTR_null, ATTR_string,
   ATTR_struct, ATTR_array, ATTR_function, ATTR_variable,
   ATTR_field, ATTR_typeid, ATTR_param, ATTR_lval, ATTR_const,
   ATTR_vreg, ATTR_vaddr, ATTR_prototype, ATTR_bitset_size,
};

using attr_bitset = bitset<ATTR_bitset_size>;

using symbol_table = unordered_map<string*, symbol*>;

#include "auxlib.h"
#include "lyutils.h"
#include "astree.h"

struct symbol {
   size_t filenr, linenr, offset;
   size_t blocknr = 0;
   attr_bitset attributes;
   vector<symbol*>* parameters = NULL;
   symbol_table* fields;
   string struct_name = "";
};

using symbol_entry = symbol_table::value_type;

void set_attributes (astree* node, symbol* sym);
symbol* setup_symbol (astree* node);
symbol* setup_function (astree* node);
bool type_check (astree* node);
bool semantic_analysis (astree* node, FILE* out);
void setup_symtable();
bool find_var(string* name);
bool in_scope_find_var(string* name);
bool check_return (symbol* sym1, symbol* sym2);
void print_attributes (symbol* sym, string* name, FILE* out);
symbol* get_symbol (astree* node);
bool matching (astree* node1, 
        astree* node2, 
        string struct1, 
        string struct2);
#endif

