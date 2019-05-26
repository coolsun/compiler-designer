%{
// $Id: parser.y,v 1.21 2019-04-15 15:41:31-07 - - $
// Dummy parser for scanner project.

#include <cassert>

#include "lyutils.h"
#include "astree.h"

%}

%debug
%defines
%error-verbose
%token-table
%verbose

//from 08
%destructor { destroy($$); } <>
%printer { astree::dump (yyoutput, $$); } <>
%initial-action {
   parser::root = new astree (TOK_ROOT, {0, 0, 0}, "");
}

%token TOK_VOID TOK_INT TOK_STRING
%token TOK_IF TOK_ELSE TOK_WHILE TOK_RETURN TOK_STRUCT
%token TOK_NULLPTR TOK_ARRAY TOK_ARROW TOK_ALLOC TOK_PTR
%token TOK_EQ TOK_NE TOK_LE TOK_GE TOK_NOT
%token TOK_IDENT TOK_INTCON TOK_CHARCON TOK_STRINGCON
%token TOK_ROOT TOK_BLOCK TOK_CALL TOK_INITDECL

// ssun add pseudo tokens
%token TOK_PARAM TOK_TYPE_ID TOK_VARDECL TOK_FUNCTION
%token TOK_INDEX


%right  TOK_IF TOK_ELSE
%right  '='
%left   TOK_EQ TOK_NE '<' TOK_LE '>' TOK_GE
%left   '+' '-'
%left   '*' '/' '%'
%right  '^'
%right  POS NEG TOK_NOT
%left   TOK_PTR '[' TOK_CALL TOK_ALLOC

// ssun


%start start

%%

start   : program               { $$ = $1 = nullptr; }
        ;

program : program structdef     { $$ = $1->adopt($2);  }
        | program vardecl       { $$ = $1->adopt($2);  }
        | program function      { $$ = $1->adopt($2);  }
        | program error ';'     { destroy($3); $$ = $1; }
        | program ';'           { destroy($2); $$ = $1; }
        |                       { $$ = parser::root; }
        ;
structdef : TOK_STRUCT TOK_IDENT structdecls  '}' ';'
                                { destroy($4, $5);
                                  $$ = $1->adopt($2); 
                                  $$->adopt_children($3);
                                  destroy($3);}
          | TOK_STRUCT TOK_IDENT '{' '}' ';' 
                                { destroy($3, $4, $5);
                                  $$ = $1->adopt($2); }
          ;
type  : plaintype        { $$=new astree($1);
                           $$->update(TOK_TYPE_ID, ""); 
                           $$->adopt($1);}
      | TOK_ARRAY '<' plaintype '>' 
                         { destroy($2, $4);
                           $$ = $1->adopt($3); }
plaintype: TOK_VOID        { $$ = $1; }
         | TOK_INT         { $$ = $1; }
         | TOK_STRING      { $$ = $1; }
         | TOK_PTR '<' TOK_STRUCT TOK_IDENT '>' 
                           { destroy($2, $3, $5);
                             $$ = $1->adopt($4); }
                   
function: identdecl params block { $$=new astree($1);
                                   $$->update(TOK_FUNCTION, "");
                                   $$->adopt($1, $2, $3); }
        ;
params  : identdecls ')'        { destroy($2);
                                  $1->update(TOK_PARAM);
                                  $$ = $1; }
        | '(' ')'               { destroy($2); $1->update(TOK_PARAM);
                                  $$ = $1; }
        ;
identdecls  : identdecls ',' identdecl  { destroy($2); 
                                          $$=$1->adopt($3); }
            | '(' identdecl             { $$ = $1->adopt($2); }
            ;
identdecl   : type TOK_IDENT            { $$=$1->adopt($2); }
structdecls : structdecls structdecl    { $$=$1->adopt($2); }
            | '{' structdecl            { $$ = $1->adopt($2); }
            ;
structdecl  : type TOK_IDENT ';'        { destroy($3); 
                                          $$=$1->adopt($2); }

block   : stmts '}'           { destroy($2); 
                                $1->update(TOK_BLOCK);
                                $$ = $1; }
        | '{' '}'             { destroy($2); $1->update(TOK_BLOCK); 
                                $$ = $1; }
        | ';'                 { $1->update(TOK_BLOCK); $$ = $1; }
        ;
stmts   : stmts stmt          { $$ = $1->adopt($2); }
        | '{' stmt            { $$ = $1->adopt($2); }


stmt    : vardecl             { $$ = $1; }
        | block               { $$ = $1; }
        | while               { $$ = $1; }
        | ifelse              { $$ = $1; }
        | return              { $$ = $1; }
        | expr ';'            { destroy($2); $$ = $1; }
        ;

vardecl : identdecl '=' expr ';'    { destroy($4);
                                      $2->update(TOK_VARDECL);
                                      $$ = $2->adopt($1, $3); }
        | identdecl                 { $$ = $1; }
        ;
while   : TOK_WHILE '(' expr ')' stmt 
                                   { destroy($2, $4);
                                     $$ = $1->adopt($3, $5); }
ifelse  : TOK_IF '(' expr ')' stmt %prec TOK_IF
								   { destroy($2, $4);
                                     $$ = $1->adopt($3, $5); 
                                   }
        | TOK_IF '(' expr ')' stmt TOK_ELSE stmt 
                                   { destroy($2, $4, $6);
                                     $$ = $1->adopt($3, $5, $7); 
                                   }
        
return  : TOK_RETURN expr ';'   { destroy($3); 
                                  $$ = $1-> adopt($2); }
        | TOK_RETURN ';'        { destroy($2); $$ = $1; }

// change to TOK_IDENT and TOK_INT
expr    : expr TOK_EQ expr      { $$ = $2->adopt($1, $3); }
        | expr TOK_LE expr      { $$ = $2->adopt($1, $3); }
        | expr TOK_GE expr      { $$ = $2->adopt($1, $3); }
        | expr TOK_NE expr      { $$ = $2->adopt($1, $3); }
        | expr '<' expr         { $$ = $2->adopt($1, $3); }
        | expr '>' expr         { $$ = $2->adopt($1, $3); }
        | expr '=' expr         { $$ = $2->adopt($1, $3); }
        | expr '+' expr         { $$ = $2->adopt($1, $3); }
        | expr '-' expr         { $$ = $2->adopt($1, $3); }
        | expr '*' expr         { $$ = $2->adopt($1, $3); }
        | expr '/' expr         { $$ = $2->adopt($1, $3); }
        | expr '%' expr         { $$ = $2->adopt($1, $3); }
        | '+' expr %prec POS    { $$ = $1->adopt_sym($2, POS); }
        | '-' expr %prec NEG    { $$ = $1->adopt_sym($2, NEG); }
        | TOK_NOT expr %prec TOK_NOT    
                                { $$ = $1->adopt_sym($2, NEG); }
        | allocator             { $$ = $1; }
        | call                  { $$ = $1; }
        | '(' expr ')'          { destroy($1, $3); $$ = $2; }
        | variable              { $$ = $1; }
        | constant              { $$ = $1; }
        ;
allocator : TOK_ALLOC '<' TOK_STRING '>' '(' expr ')'
                                { destroy($2, $4, $5, $7);
                                  $$ = $1->adopt($3, $6); }
         | TOK_ALLOC '<' TOK_STRUCT TOK_IDENT '>' '(' ')'
                { destroy($2, $3, $5, $6, $7);
                  $$ = $1->adopt($4); }
         | TOK_ALLOC '<' TOK_ARRAY '<' plaintype '>' '>' '(' expr ')'
                { destroy($2, $4, $6, $7, $8, $10);
                  $3->adopt($5);
                  $$ = $1->adopt($3, $9); }  
call     : TOK_IDENT call_params { $$ = new astree($1);
                                   $$->update(TOK_CALL, "(");
                                   $$->adopt($1); 
		        	   $$->adopt_children($2); 
                                   destroy($2);}
         ;
variable : TOK_IDENT               { $$ = $1; }
         | expr '[' expr  ']'      { destroy($4); 
                                     $2->update(TOK_INDEX);
                                     $$ = $2->adopt($1, $3); }
         | expr TOK_PTR TOK_IDENT  
                                   { $$ = $2->adopt($1, $3); }
         ;
call_params  : exprs ')'           { destroy($2);
                                     $$ = $1; }
             | '(' ')'             { destroy($2); 
                                     $$ = $1; }
             ;

exprs    : exprs ',' expr   { destroy($2); 
                              $$ = $1->adopt($3); }
         | '(' expr         { $$ = $1->adopt($2); }
                
constant : TOK_INTCON       { $$ = $1; }
         | TOK_CHARCON      { $$ = $1; }
         | TOK_STRINGCON    { $$ = $1; }
         | TOK_NULLPTR      { $$ = $1; }

%%

const char *parser::get_tname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}

const char *get_yytname (int symbol) {
   return yytname [YYTRANSLATE (symbol)];
}



bool is_defined_token (int symbol) {
   return YYTRANSLATE (symbol) > YYUNDEFTOK;
}

