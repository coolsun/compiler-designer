#include "symtable.h"

symbol_table structs;
unordered_map<string, symbol_table*> fields;
symbol_table identifiers;

int next_block = 0;
vector<symbol_table*> symbol_stack;
int block_num = 0;
vector<int> block_stack;

#define TOK_NEWSTRING 10002 
#define TOK_NEWARRAY 10003 
#define TOK_INDEX 10004 
#define TOK_EXC   10005 
#define TOK_PROTOTYPE 10005 
#define TOK_RETURNVOID 10006 

#define TOK_RET TOK_RETURN
#define TOK_NEW TOK_ALLOC
#define TOK_NEG NEG
#define TOK_POS POS 
#define TOK_GEQ TOK_GE 
#define TOK_LEQ TOK_LE 
#define TOK_NEQ TOK_NE 

void set_attributes (astree* node, symbol* sym) {
   sym->filenr = node->lloc.filenr;
   sym->linenr = node->lloc.linenr;
   sym->offset = node->lloc.offset;
   sym->blocknr = block_stack.back();
   switch (node->symbol) {
      case TOK_INT: {
         sym->attributes[ATTR_int] = 1;
         break;
      }
      case TOK_STRING: {
         sym->attributes[ATTR_string] = 1;
         break;
      }
      case TOK_VOID: {
         sym->attributes[ATTR_void] = 1;
         break;
      }
      case TOK_NULLPTR: {
         sym->attributes[ATTR_null] = 1; // TBD: nullptr
         break;
      }
      case TOK_TYPE_ID: {
         sym->attributes[ATTR_struct] = 1;
         sym->struct_name = string(*(node->lexinfo));
         break;
      }
   }
}

symbol* setup_symbol (astree* node) {
   symbol* sym = new symbol;
   set_attributes(node, sym);
   sym->attributes[ATTR_lval] = 1;
   sym->attributes[ATTR_variable] = 1;
   return sym;
}

symbol* setup_function (astree* node) {
   symbol* sym = new symbol;
   if(node->children[0]->symbol != TOK_ARRAY) {
      set_attributes(node->children[0], sym);
   } else {
      set_attributes(node->children[0]->children[0], sym);
   }
   sym->attributes[ATTR_function] = 1;

   return sym;
}

bool find_var (string* name) {
   bool found = false;
   for(int i = symbol_stack.size()-1; i >= 0; --i) {
      if(symbol_stack[i] == nullptr) {
         continue;
      }
     if(symbol_stack[i]->find(name) != symbol_stack[i]->end()) {
         if(symbol_stack[i]->at(name)->attributes[ATTR_function] == 0){
            found = true;
            break;
         }
      }
   }
   if(identifiers.find(name) != identifiers.end()) found = true;
   return found;
}

bool in_scope_find_var (string* name) {
   bool found = false;
   symbol_table* back = symbol_stack.back();
   if(back != nullptr) {
      if(back->find(name) != back->end()) {
         if(!back->at(name)->attributes[ATTR_function]){
            found = true;
         }
      }
   }
   return found;
}

void print_attributes (symbol* sym, string* name, FILE* out) {
   attr_bitset attr = sym->attributes;
   string list;

   for(uint i = 0; i < block_stack.size()-1; ++i) {
      fprintf(out, "\t");
   }
   if (attr[ATTR_field] == 0) list += "{"+to_string(sym->blocknr)+"} ";

   if (attr[ATTR_field] == 1) {
      list += "field {" + sym->struct_name + "} ";
   } if (attr[ATTR_struct] == 1) {
      list += "struct \"" + sym->struct_name + "\" ";
   } if (attr[ATTR_int] == 1) {
      list += "int ";
   } if (attr[ATTR_string] == 1) {
      list += "string ";
   } if (attr[ATTR_variable] == 1) {
      list += "variable ";
   } if (attr[ATTR_null] == 1) {
      list += "null ";
   } if (attr[ATTR_function] == 1) {
      list += "function ";
   } if (attr[ATTR_lval] == 1) {
      list += "lval ";
   } if (attr[ATTR_param] == 1) {
      list += "param ";
   } if (attr[ATTR_const] == 1) {
      list += "const ";
   } if (attr[ATTR_vreg] == 1) {
      list += "vreg ";
   } if (attr[ATTR_vaddr] == 1) {
      list += "vaddr ";
   } if (attr[ATTR_void] == 1) {
      list += "void ";
   }

   fprintf(out, "%s (%zu.%zu.%zu) %s\n", 
      (*name).c_str(), sym->filenr, sym->linenr, 
      sym->offset, list.c_str());
}

bool check_return (symbol* sym1, symbol* sym2) {
   attr_bitset bit1 = sym1->attributes;
   attr_bitset bit2 = sym2->attributes;
   
   if((bit1[ATTR_int] && bit2[ATTR_int])
      || (bit1[ATTR_void] && bit2[ATTR_void])
      || (bit1[ATTR_string] && bit2[ATTR_string]))
         return false;
   return true; 
}

symbol* get_symbol (astree* node) {
   string* name = const_cast<string*>(node->lexinfo);
   for(int i = symbol_stack.size()-1; i >= 0; --i) {
      if(symbol_stack[i] == nullptr) {
         continue;
      }
      if(symbol_stack[i]->find(name) != symbol_stack[i]->end()) {
         if(symbol_stack[i]->at(name)->attributes[ATTR_function] == 0){
            return symbol_stack[i]->at(name);
         }
      }
   }
   return NULL;
}

bool matching(attr_bitset attr1, attr_bitset attr2, 
                              string struct1, string struct2) {
   if((attr1[ATTR_null] && attr2[ATTR_struct])
   || (attr1[ATTR_struct] && attr2[ATTR_null])
   || (attr1[ATTR_null] && attr2[ATTR_string])
   || (attr1[ATTR_string] && attr2[ATTR_null])
   || (attr1[ATTR_null] && attr2[ATTR_array])
   || (attr1[ATTR_array] && attr2[ATTR_null]))
     return true;

   if ((attr1[ATTR_int] != attr2[ATTR_int])
   || (attr1[ATTR_string] != attr2[ATTR_string])
   || (attr1[ATTR_array] != attr2[ATTR_array])
   || (attr1[ATTR_struct] != attr2[ATTR_struct])
   || (attr1[ATTR_void] != attr2[ATTR_void])
   || ((attr1[ATTR_struct] != attr2[ATTR_struct]) 
      &&(struct1 != struct2)))
      return false;

   return true;
}

bool type_check (astree* node) {
   switch (node->symbol) {
      case TOK_TYPE_ID: {
            string* name = const_cast<string*>(node->lexinfo);
            if(structs.find(name) == structs.end()) {
               break;
            }
            symbol* str = structs.at(name);
            node->attributes[ATTR_struct] = 1;
            node->struct_name = str->struct_name;
         break;
      }
      case TOK_VOID: {
         if(!(node->parent->symbol == TOK_FUNCTION
            || node->parent->symbol == TOK_PROTOTYPE)) {
            cerr << "Cannot have void in non-function" << endl;
            return false;
         }
         break;
      }
      case TOK_VARDECL: {
         astree* n;
         if(node->children[0]->symbol != TOK_ARRAY)
            n = node->children[0]->children[0];
         else
            n = node->children[0]->children[1];

         if(!matching(n->attributes,
            node->children[1]->attributes,
            n->struct_name, node->children[1]->struct_name)) {
            cerr << "Incompatible types" << endl;
            return false;
         }
        break;
      }
      case TOK_RETURNVOID: {
         node->attributes[ATTR_void] = 1;
         astree* n = node;
         while (n != NULL && n->symbol != TOK_FUNCTION) {
            n = n->parent;
         }
         if (n == NULL) {
            cerr << "Cannot have return outside of a function" << endl;
            return false;
         }
         if(!n->children[0]->children[0]->attributes[ATTR_void]) {
            cerr << "Incompatible return type" << endl;
            return false;
         }
         break;
      }
      case TOK_RET: {
         astree* n = node;
         while (n != NULL && n->symbol != TOK_FUNCTION) {
            n = n->parent;
         }
         if (n == NULL) {
            cerr << "Cannot have return outside of a function" << endl;
            return false;
         }
         if(!matching(n->children[0]->children[0]->attributes,
            node->children[0]->attributes,
            n->children[0]->children[0]->struct_name,
            node->children[0]->struct_name)) {
            cerr << "Incompatible return type" << endl;
            return false;
         }
         break;
      }
      case '=': {
         if(!matching(node->children[0]->attributes,
            node->children[1]->attributes,
            node->children[0]->struct_name,
            node->children[1]->struct_name)
            || !node->children[0]->attributes[ATTR_lval]) {
            cerr << "Incorrect assignment" << endl;
            return false;
         }
         node->attributes[ATTR_string] =
             node->children[0]->attributes[ATTR_string];
         node->attributes[ATTR_int] =
             node->children[0]->attributes[ATTR_int];
         node->attributes[ATTR_struct] =
             node->children[0]->attributes[ATTR_struct];
         node->attributes[ATTR_array] =
             node->children[0]->attributes[ATTR_array];
         node->struct_name = node->children[0]->struct_name;
         node->attributes[ATTR_vreg] = 1;
         break;
      }
      case TOK_EXC:
      case TOK_POS:
      case TOK_NEG: {
         if(!node->children[0]->attributes[ATTR_int]
            && node->children[0]->attributes[ATTR_array]){
            cerr << "Cannot use UNOP on this type" << endl;
            return false;
         }

         node->attributes[ATTR_int] = 1;
         node->attributes[ATTR_vreg] = 1;
         break;
      }
      case TOK_NEW: {
         node->attributes = node->children[0]->attributes;
         node->struct_name = node->children[0]->struct_name;
         node->attributes[ATTR_vreg] = 1;
         break;
      }
      case TOK_CALL: {
         vector<astree*> params;
         for(uint i = 1; i < node->children.size(); ++i) {
            params.push_back(node->children[i]);
         }

         string* name = const_cast<string*>(
            node->children[0]->lexinfo);

         symbol* sym;
         if(identifiers.find(name) != identifiers.end()) {
            sym = identifiers.at(name);
         } else {
            cerr << "Function not declared" << endl;
            return false;
         }

         if(params.size() != sym->parameters->size()) {
            cerr << "Incorrect number of parameters given" << endl;
            return false;
         }
         if(params.size() > 0) {
            for(uint i = 0; i < params.size()-1; ++i) {
               if(!matching(params[i]->attributes,
                     sym->parameters->at(i)->attributes,
                     params[i]->struct_name,
                     sym->parameters->at(i)->struct_name)) {
                  cerr << "Function parameters do not match" << endl;
                  return false;
               }
            }
         }
         node->attributes = sym->attributes;
         node->attributes[ATTR_vreg] = 1;
         node->attributes[ATTR_function] = 1;
         node->struct_name = sym->struct_name;
         break;
      }
      case '-':
      case '*':
      case '/':
      case '%':
      case '+': {
         astree* node1 = node->children[0];
         astree* node2 = node->children[1];
         if(!(node1->attributes[ATTR_int]
            && node2->attributes[ATTR_int])
            && (node1->attributes[ATTR_array]
            || node2->attributes[ATTR_array])) {
            cerr << "Incompatible BINOP types" << endl;
            return false;
         }
         node->attributes[ATTR_int] = 1;
         node->attributes[ATTR_vreg] = 1;
         break;
      }
      case TOK_EQ:
      case TOK_NEQ:
      case '<':
      case TOK_LEQ:
      case '>':
      case TOK_GEQ: {
         if(!matching(node->children[0]->attributes,
            node->children[1]->attributes, 
            node->children[0]->struct_name,
            node->children[1]->struct_name)) {
            cerr << "Invalid comparison" << endl;
            return false;
         }

         node->attributes[ATTR_int] = 1;
         node->attributes[ATTR_vreg] = 1;
         break;
      }
      case TOK_NEWSTRING: {
         if(!node->children[1]->attributes[ATTR_int]) {
            cerr << "Size is not integer" << endl;
            return false;
         }
         node->attributes[ATTR_vreg] = 1;
         node->attributes[ATTR_string] = 1;
         break;
      }
      case TOK_IDENT: {
         string* name = const_cast<string*>(node->lexinfo);
         symbol* sym = get_symbol(node);
         if(sym == NULL) {
            if(identifiers.find(name) != identifiers.end()) {
               sym = identifiers[name];
            } else {
               cerr << "Undeclared identifier" << endl;
               return false;
            }
         }
         node->attributes = sym->attributes;
         node->struct_name = sym->struct_name;
         break;
      }
      case TOK_INDEX: {
         if(!node->children[1]->attributes[ATTR_int]) {
            cerr << "Cannot access array without integer";
            return false;
         }

         if(!(node->children[0]->attributes[ATTR_string])
            && !(node->children[0]->attributes[ATTR_array])) {
            cerr << "Not proper type for array access" << endl;
            return false;
         }
         if(node->children[0]->attributes[ATTR_string]
            && !node->children[0]->attributes[ATTR_array]) {
            node->attributes[ATTR_int] = 1;
         }
         else {
            node->attributes[ATTR_int] =
                node->children[0]->attributes[ATTR_int];
            node->attributes[ATTR_string] =
                node->children[0]->attributes[ATTR_string];
            node->attributes[ATTR_struct] =
                node->children[0]->attributes[ATTR_struct];
         }

         node->attributes[ATTR_array] = 0;
         node->attributes[ATTR_vaddr] = 1;
         node->attributes[ATTR_lval] = 1;
         node->attributes[ATTR_variable] = 0;
         node->struct_name = node->children[1]->struct_name;
         break;
      }
      case '.': {
         // ADD CHECKS FOR STRUCT COMPLETENESS
         if(!node->children[0]->attributes[ATTR_struct]) {
            cerr << "Invalid usage of field selection";
            return false;
         }

         string name = (node->children[0]->struct_name);
         if(fields.find(name) == fields.end()) {
            cerr << "Cannot reference field of undefined struct"
                << endl;
            return false;
         }
         symbol_table* str = fields.at(name);

         string* field_name = const_cast<string*>(
            node->children[1]->lexinfo);
         
         node->attributes = str->at(field_name)->attributes;
         node->struct_name = str->at(field_name)->struct_name;

         node->attributes[ATTR_field] = 0;
         node->attributes[ATTR_vaddr] = 1;
         node->attributes[ATTR_lval] = 1;
         node->attributes[ATTR_variable] = 1;
         break;
      }
      case TOK_NULLPTR: {
         node->attributes[ATTR_null] = 1; // nullptr
         node->attributes[ATTR_const] = 1;
         break;
      }
      case TOK_STRINGCON: {
         node->attributes[ATTR_const] = 1;
         node->attributes[ATTR_string] = 1;
         break;
      }
      case TOK_INTCON: {
         node->attributes[ATTR_const] = 1;
         node->attributes[ATTR_int] = 1;
         break;
      }
      case TOK_CHARCON: {
         node->attributes[ATTR_const] = 1;
         node->attributes[ATTR_int] = 1;
         break;
      }
      case TOK_INT: {
         node->attributes[ATTR_int] = 1;
         break;
      }
      case TOK_STRING: {
         node->attributes[ATTR_string] = 1;
         break;
      }
      case TOK_NEWARRAY: {
         attr_bitset attrs = node->children[0]->attributes;
         if(attrs[ATTR_void]
            || attrs[ATTR_array]
            || attrs[ATTR_void]
            || (attrs[ATTR_struct]
               && node->children[0]->struct_name == "")) {
            cerr << "Invalid array type" << endl;
            return false;
         }
         if(attrs[ATTR_int])
            node->attributes[ATTR_int] = 1;
         else if(attrs[ATTR_string])
            node->attributes[ATTR_string] = 1;

         node->attributes[ATTR_vreg] = 1;
         node->attributes[ATTR_array] = 1;
         break;
      }
      default: break;
   }
   return true;
}

bool semantic_analysis (astree* node, FILE* out) {
   
   node->blocknr = block_stack.back();

   // Pre-order stuff
   switch (node->symbol) {
      case TOK_BLOCK: {
         block_stack.push_back(next_block++);
         symbol_stack.push_back(nullptr);
         break;
      }
      case TOK_FUNCTION: {
         string* name = const_cast<string*>(
            node->children[0]->children[1]->lexinfo);

         if(node->children[0]->symbol == TOK_ARRAY) {
            name = const_cast<string*>(
               node->children[0]->children[1]->lexinfo);
         }
        
         if(identifiers.find(name) != identifiers.end()) {
            if(identifiers[name]->attributes[ATTR_function] == 1) {
               cerr << "Function already declared" << endl;
               break;
            }
         }
         if(symbol_stack.back() == nullptr) {
            symbol_stack.pop_back();
            symbol_stack.push_back(new symbol_table);
         }

         symbol* sym = setup_function(node);
         astree* tree0 = node->children[0];
         print_attributes(sym, name, out);

         if(tree0->symbol != TOK_ARRAY) {
            tree0->children[0]->attributes = sym->attributes;
            tree0->children[0]->struct_name = sym->struct_name;
         } else {
            tree0->children[1]->attributes = sym->attributes;
            tree0->children[1]->struct_name = sym->struct_name;
         }
         sym->blocknr = GLOBAL;

         block_stack.push_back(next_block++);

         sym->parameters = new vector<symbol*>;
         symbol_stack.push_back(new symbol_table);

         // Setup parameters
         for(uint i = 0; i < node->children[1]->children.size(); ++i) {
            astree* current = node->children[1]->children[i];
            symbol* param;
            if(current->symbol == TOK_ARRAY)
               param = setup_symbol(current->children[0]);
            else
               param = setup_symbol(current);

            param->attributes[ATTR_param] = 1;
            sym->parameters->push_back(param);

            if(current->symbol != TOK_ARRAY) {
               current->children[0]->attributes = param->attributes;
               current->children[0]->blocknr = param->blocknr;
               current->children[0]->struct_name = param->struct_name;
            } else {
               param->attributes[ATTR_array] = 1;
               current->children[1]->attributes = param->attributes;
               current->children[1]->blocknr = param->blocknr;
               current->children[1]->struct_name = param->struct_name;
            }

            string* param_name;

            if(current->symbol == TOK_ARRAY)
               param_name = const_cast<string*>(
               current->children[1]->lexinfo);
            else
               param_name = const_cast<string*>(
               current->children[0]->lexinfo);

            symbol_entry param_entry (param_name, param);

            symbol_stack.back()->insert(param_entry);
            print_attributes (param, param_name, out);
         }

         // Checking if prototype and function params/return match
         if(identifiers.find(name) != identifiers.end()) {
            if(identifiers[name]->attributes[ATTR_prototype] == 1) {
               if(identifiers[name]->parameters != sym->parameters) {
                  cerr << "Mismatched parameters " << endl;
               }
               if(check_return(identifiers[name], sym) == false) {
                  cerr << "Mismatched return types" << endl;
               }
            }
         }

         symbol_entry entry (name, sym);

         symbol_stack.back()->insert(entry);

         identifiers.insert(entry);

         fprintf(out, "\n");
         break;
      }
      case TOK_PROTOTYPE: {
         astree* t0 = node->children[0];
         string* name = const_cast<string*>(t0->children[0]->lexinfo);
         if(t0->symbol == TOK_ARRAY)
            name = const_cast<string*>(t0->children[1]->lexinfo);

         if(identifiers.find(name) != identifiers.end()) {
            if(identifiers[name]->attributes[ATTR_function] == 1) {
               cerr << "Prototype already declared" << endl;
               break;
            }
         }

         if(symbol_stack.back() == nullptr) {
            symbol_stack.pop_back();
            symbol_stack.push_back(new symbol_table);
         }

         symbol* sym = setup_function(node);
         sym->attributes[ATTR_prototype] = 1;

         block_stack.push_back(next_block++);

         sym->parameters = new vector<symbol*>;
         if(t0->symbol == TOK_ARRAY) {
            sym->attributes[ATTR_array] = 1;
            t0->children[1]->attributes = sym->attributes;
            t0->children[0]->attributes[ATTR_array] = 1;
         }
         else {
            t0->children[0]->attributes = sym->attributes;
         }

         symbol_stack.push_back(new symbol_table);
         astree* t1 = node->children[1];
         for(uint i = 0; i < t1->children.size(); ++i) {
            astree* current = node->children[1]->children[i];
            symbol* param = setup_symbol(t1->children[i]);
            param->attributes[ATTR_param] = 1;
            sym->parameters->push_back(param);
            current->children[0]->attributes = param->attributes;
            current->children[0]->blocknr = param->blocknr;
            current->children[0]->struct_name = param->struct_name;

            string* param_name = const_cast<string*>(
               current->children[0]->lexinfo);

            symbol_entry param_entry (param_name, param);

            symbol_stack.back()->insert(param_entry);
            print_attributes (param, param_name, out);
         }
         symbol_entry entry (name, sym);

         symbol_stack.back()->insert(entry);
         identifiers.insert(entry);

         break;
      }
      case TOK_STRUCT: {
         string* name =const_cast<string*>(node->children[0]->lexinfo);
         symbol* sym = new symbol;
         sym->attributes[ATTR_struct] = 1;
         sym->struct_name = *name;
         set_attributes(node, sym);
         sym->blocknr = GLOBAL;
         node->attributes = sym->attributes;
         node->struct_name = sym->struct_name;

         symbol_table* tbl = new symbol_table;
         bool valid = true;

         symbol_entry entry (name, sym);
         structs.insert(entry);
         print_attributes(sym, name, out);
        
         // Check to make sure variables within struct can be declared
         if(node->children.size() > 1) {
            for(uint i = 0; 
               i < node->children[1]->children.size(); 
               ++i) {
               
               astree* current = node->children[1]->children[i];
               if(current->symbol == TOK_TYPE_ID) {
                  if(structs.find(name) == structs.end()) {
                     cerr << "No struct with given name" << endl;
                     valid = false;
                     break;
                  }
               }
               if(valid == false) break;
               fprintf(out, "\t");

               // if variable is valid add it as a field to struct
               string* field_name = const_cast<string*>(
                  current->children[0]->lexinfo);

               if(current->symbol == TOK_ARRAY)
                  field_name = const_cast<string*>(
                     current->children[1]->lexinfo);

               symbol* sym1 = new symbol;
               if(current->symbol == TOK_ARRAY) {
                  set_attributes(current->children[0], sym1);
                  sym->attributes[ATTR_array] = 1;
               }
               else
                  set_attributes(current, sym1);
               sym1->attributes[ATTR_field] = 1;
               sym1->struct_name = *(const_cast<string*>(
                  current->lexinfo));
               
               sym1->attributes[ATTR_field] = 1;
               sym1->blocknr = GLOBAL;
               symbol_entry field (field_name, sym1);
               tbl->insert(field);

               print_attributes(sym1, field_name, out);
               current->children[0]->attributes = sym1->attributes;
               current->children[0]->struct_name = *name;
            }
         }
         pair <string, symbol_table*> str (*name, tbl);
         fields.insert(str);
         sym->fields = tbl;

         fprintf(out, "\n");
         break;
      }
      case TOK_VARDECL: {

         if(symbol_stack.back() == nullptr) {
            symbol_stack.back() = new symbol_table;
         }


         string* name = const_cast<string*>(
            node->children[0]->children[0]->lexinfo);

         if(node->children[0]->symbol == TOK_ARRAY)
            name = const_cast<string*>(
               node->children[0]->children[1]->lexinfo);

         // Check if variable has already been declared in this scope
         if(in_scope_find_var(name)) {
            cerr << "Variable already declared in this scope" << endl;
            return false;
         }

         symbol* sym = new symbol; // This fixed it?
         if(node->children[1]->symbol == TOK_IDENT) {
            string* find_name = const_cast<string*>(
               node->children[1]->lexinfo);

            if(!find_var(find_name)) {
               cerr << "Variable assignment not valid" << endl;
               break;
            }
            symbol* assignee = get_symbol(node->children[1]);

            set_attributes(node->children[0], sym);
            sym->attributes = assignee->attributes;

            sym->struct_name = assignee->struct_name;
            sym->parameters = assignee->parameters;
            sym->fields = assignee->fields;
         }
         else {
            if(node->children[0]->symbol == TOK_ARRAY) {
               sym = setup_symbol(node->children[0]->children[0]);
               sym->attributes[ATTR_array] = 1;
            }
            else
               sym = setup_symbol(node->children[0]);
         }

         symbol_entry entry (name, sym);
         symbol_stack.back()->insert(entry);
         astree* t0 = node->children[0];
         if(t0->symbol == TOK_ARRAY) {
            t0->children[1]->attributes = sym->attributes;
            t0->children[1]->struct_name = sym->struct_name;
            t0->children[0]->attributes[ATTR_array] = 1;
         } else {
            t0->children[0]->attributes = sym->attributes;
            t0->children[0]->struct_name = sym->struct_name;
         }
         print_attributes(sym, name, out);
         break;
      }
      default: break;
   }

   for (astree* child: node->children) {
      bool ret = semantic_analysis(child, out);
      if(ret == false) return false;
   }

   //bool rc = type_check(node);
   // For debugging
   // cout << "RC: " << rc << endl;
   // cout << parser::get_tname(node->symbol) << endl;
   // cout << *(const_cast<string*>(node->lexinfo)) << endl;
   //if(rc == false) return false;

   // After children nodes
   switch (node->symbol) {
      case TOK_IDENT: {
         string* name = const_cast<string*>(node->lexinfo);
         if(find_var(name) == false) {
            cerr << "Variable not declared previously" << endl;
         }
         break;
      }
      default: break;
   }
   
   // Exit block
   if(node->symbol == TOK_BLOCK
      || node->symbol == TOK_FUNCTION
      || node->symbol == TOK_PROTOTYPE) {

      symbol_stack.pop_back();
      block_stack.pop_back();
   }

   return true;
}

void setup_symtable() {
   symbol_stack.push_back(new symbol_table);
   block_stack.push_back(next_block++);
}

