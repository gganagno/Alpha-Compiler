	/* included in parser.y */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define SIZE 997
#define HASH_MUL 65599

#define EXPAND_SIZE 1024
#define CURR_SIZE	(total*sizeof(quad))
#define NEW_SIZE	(EXPAND_SIZE*sizeof(quad)+CURR_SIZE) 

#define RED   "\033[1m\033[31m"
#define GREEN   "\033[1m\033[32m"
#define RESET "\x1B[0m"
#define BLUE  "\033[01;34m"


extern int yylineno;
extern char *yytext;
extern FILE *yyin;





typedef enum scopespace_t {

	PROGRAMVAR,
	FUNCTIONLOCAL,
	FORMALARG

} scopespace_t;


typedef enum symbol_t { 
	VAR_S,
	PROGRAMFUNC_S,
	LIBRARYFUNC_S
} symbol_t;



typedef enum stmt_type { 

	ONLY_STMT,
	IF_STMT,
	LOOP_STMT,
	BOOL_STMT,
	BREAK_STMT,
	CONTINUE_STMT,
	RETURN_STMT,
	FUNCDEF_STMT,
	INVALID_STMT
} stmt_type;



typedef enum expr_t {

	VAR_E = 0,
	TABLEITEM_E,

	PROGRAMFUNC_E,
	LIBRARYFUNC_E,

	ARITHEXPR_E,
	BOOLEXPR_E,
	ASSIGNEXPR_E,
	NEWTABLE_E,

	CONSTNUM_E,
	CONSTBOOL_E,
	CONSTSTRING_E,

	NIL_E

} expr_t;


typedef enum iopcode {
	ASSIGN,
	ADD,			
	SUB,
	MUL,			
	DIVISION,		
	MOD,
	UMINUS,			
	JUMP,			
	IF_EQ,			
	IF_NOTEQ,
	IF_LESSEQ,		
	IF_GREATEREQ,	
	IF_LESS,
	IF_GREATER,		
	CALL,			
	PARAM,			
	FUNCSTART,		
	FUNCEND,		
	TABLECREATE,
	TABLEGETELEM,	
	TABLESETELEM,
	NOP,			
	GETRETVAL,
	RET
} iopcode;






typedef struct symbol {
	unsigned		isActive;
	symbol_t 		type;
	char*			name;
	scopespace_t	space;
	unsigned		offset;
	unsigned		scope;
	unsigned		line;
	unsigned	taddress;

	union Addr{
		unsigned 		iaddress;
		char *			lib_addr;
	}addr;
	
	unsigned		total_locals;


	struct symbol *next;
	struct symbol *scope_link_next;
	struct List_node * return_list;
} symbol;



typedef struct method_struct {
	unsigned 		is_method;
	char * 			methodname;

} method_t;




typedef struct expr {
	expr_t			type;
	symbol*			sym;
	struct expr*	index;
	double 			numConst;
	char*			strConst;
	unsigned char 	boolConst;

	unsigned for_test;
	unsigned for_enter;

	struct expr* 	next;
	struct expr* 	elist;
	struct List_node * true_list;
	struct List_node * false_list;

	struct method_struct * method;

} expr;




typedef struct List_node {
	unsigned label;
	unsigned is_skip;
	struct List_node * next;

}list_node;




typedef struct  Stmt{

	struct expr * expr;
	unsigned value;
	stmt_type type;
	struct List_node * break_list;
	struct List_node * continue_list;
	struct Stmt * next;
	
}stmt;



typedef struct quad {
	iopcode		op;
	expr* 		result;
	expr*		arg1;
	expr*		arg2;
	unsigned	label;
	unsigned	line;
	unsigned	taddress;
}quad;



typedef struct Stack {

	int offset;
	struct Stack * next;

}stack_offset_t;


typedef struct stackexpr {

	expr * expr;
	struct stackexpr * next;

}stack_expr_t;


typedef struct Stack_list {
	unsigned loop_num;
	list_node * break_list;
	list_node * continue_list;
	struct Stack_list * next;

}stack_bc_t;




int yyerror(char *yaccmsg);

int yylex(void);



// syntax_funct.c
unsigned int get_scope(symbol * entry);

const char * get_name(symbol * entry);

unsigned int get_line(symbol * entry);

unsigned int sym_table_hash(const char *key);

void sym_table_init(void);

symbol * lookup_hashtable(char * name, unsigned int scope);

int scope_link_lookup(const char * new_entry, unsigned int scope) ;

symbol * scope_link_lookup_ret(const char * new_entry, unsigned int scope) ;

symbol * scope_link_lookup_scope_and_space(unsigned int scope , scopespace_t space);

void scope_link_init();

void add_dummy(unsigned int scope);

int have_dummy(unsigned int scope);

int scope_link_insert(symbol * new_entry);

int hide_scope(unsigned int scope);

int is_lib_func(const char * func_name);

symbol*  sym_table_insert(char * name, unsigned int line, unsigned int scope, scopespace_t space, symbol_t type) ;

void insert_lib_func();

void init_all();



// intermediate_funct.c
scopespace_t curr_scope_space(void);

unsigned curr_scope_offset (void);

void inc_curr_scope_offset (void);

void restore_curr_scope_offset (unsigned n);

void enter_scope_space (void);

void reset_formalargs_offset(void); 

void reset_functionlocals_offset (void);

void exit_scope_space (void); 

int check_uminus(expr * expr);

unsigned next_quad_label(void);

void patch_label(unsigned quadNo, unsigned label);

void backpatch(list_node * head, unsigned label);

void expand (void);

void emit (iopcode op, expr * arg1, expr * arg2, expr  *result, unsigned label, unsigned line );

expr* newexpr (expr_t t);

expr *  emit_if_tableitem (expr * arg);

expr* lvalue_expr (symbol* sym);

stmt * newstmt_expr (expr * exp, stmt_type type);

stmt * newstmt_symbol (symbol * sym, stmt_type type);

stmt * new_stmt ( stmt_type type);

stmt * new_break_stmt(list_node * break_list);

stmt * new_continue_stmt(list_node * continue_list);

expr* newexpr_const_string (char* s);

expr* newexpr_const_num (double x);

expr* newexpr_const_bool (int x);

void reset_tmp();

unsigned is_tmp_name(char * s);

unsigned int is_tmp_expr( expr * e);

char * new_tmp_name();

char * new_tmp_funcname();

symbol * new_tmp();

symbol * new_symbol(char * name, symbol_t type, unsigned line);

expr * member_item(expr * lvalue, char * name);

expr * make_call(expr* lvalue, expr * elist);

int true_test(expr * exp);

list_node * new_list_node(unsigned label);

list_node * merge(list_node * list_a, list_node * list_b);

stack_offset_t * push(stack_offset_t * stack, int local_offset);

void push_bc(stack_bc_t * new);

int  pop(stack_offset_t * stack);

stack_bc_t * pop_bc();

char * op_to_text(iopcode op);

char* expr_to_text (expr_t expr);

char* scopespace_to_text (scopespace_t space);

char* symbol_to_text (symbol_t space);

void print_symbol_table();

void print_scope_link();

void print_list(char * type, list_node * head);

void print_stack(stack_offset_t * top);

void print_loop_stack();

void print_arg(expr * arg,FILE * fd);

void print_arg_example(expr * arg);

void print_quads(void);

void print_example(void);


extern quad* quads;

quad * quads = (quad*) 0;




symbol ** symbol_table = NULL;

symbol * scope_link_list = NULL;

stack_offset_t * function_locals_stack = NULL;

stack_offset_t * loop_counter_stack = NULL;

stack_bc_t * loop_bc_stack = NULL;

stack_expr_t * param_stack = NULL;


unsigned scope = 0;

unsigned funct = 0;

unsigned func_name = 1;

unsigned error_cnt = 0;

unsigned ret_ctr = 0;

unsigned total = 0;

unsigned currQuad = 0;

unsigned programVarOffset = 0;

unsigned functionLocalOffset = 0;

unsigned formalArgOffset = 0;

unsigned loop_counter = 0;

unsigned scopeSpaceCounter = 1;

unsigned tmp_counter = 0;



unsigned currProcessedQuad = 0;




unsigned consts_newstring (char* s);

double consts_newnumber(double n);

unsigned libfuncs_newused (char* s);

unsigned userfuncs_new (unsigned iaddress, unsigned locals, char * name);

typedef enum VMopcode{
	ASSIGN_V = 0,	
	ADD_V,			
	SUB_V,
	MUL_V,			
	DIV_V,			
	MOD_V,
	UMINUS_V,		
	JUMP_V,			
	JEQ_V,			
	JNE_V,
	JLE_V,
	JGE_V,			
	JLT_V,			
	JGT_V,		
	CALL_V,			
	PUSHARG_V,		
	ENTERFUNC_V,
	EXITFUNC_V,
	NEWTABLE_V,		
	TABLEGETELEM_V,	
	TABLESETELEM_V,
	NOP_V

}vmopcode;

typedef enum VMarg_t {
	LABEL_A		=0,
	GLOBAL_A	=1,
	FORMAL_A	=2,
	LOCAL_A		=3,
	NUMBER_A	=4,
	STRING_A	=5,
	BOOL_A		=6,
	NIL_A		=7,
	USERFUNC_A	=8,
	LIBFUNC_A	=9,
	RETVAL_A	=10,
	INVALID_A   =11
}vmarg_t;

typedef struct VMarg {
	
	vmarg_t type;
	unsigned val;
}vmarg;

typedef struct Instruction {
	vmopcode	opcode;
	vmarg 		result;
	vmarg 		arg1;
	vmarg 		arg2;
	unsigned 	label;
	unsigned 	src_line;

}instruction;

typedef struct Userfunc {
	unsigned	address;
	unsigned	localSize;
	char*		id;
}userfunc;




//const pinakes 
double*		num_consts;

unsigned	total_num_consts;

unsigned	curr_num_consts;



char**		string_consts;

unsigned	total_string_consts;

unsigned	curr_string_consts;



char**		named_libfuncs;

unsigned	total_named_libfuncs;

unsigned	curr_named_libfuncs;


userfunc*	user_funcs;

unsigned	total_user_funcs;

unsigned	curr_user_funcs;





typedef struct incomplete_jump {

	unsigned 			instrNo;
	unsigned 			iaddress;
	struct incomplete_jump * next;

}incomplete_jump;

incomplete_jump* 	ij_head = (incomplete_jump*) 0;
unsigned 			ij_total = 0;

void add_incomplete_jump (unsigned instrNo, unsigned iaddress);





extern instruction* instructions;

instruction * instructions = (instruction*) 0;
unsigned total_target = 0;
unsigned int currInstruction = 0;

#define CURR_SIZE_TARGET 	(total_target*sizeof(instruction))

#define NEW_SIZE_TARGET		(EXPAND_SIZE*sizeof(instruction)+CURR_SIZE_TARGET)


extern void generate_ASSIGN (quad*);
extern void generate_ADD (quad*);
extern void generate_SUB (quad*);
extern void generate_MUL (quad*);
extern void generate_DIV (quad*);
extern void generate_MOD (quad*);

extern void generate_UMINUS (quad*);

extern void generate_JUMP (quad*);


extern void generate_IF_EQ (quad*);
extern void generate_IF_NOTEQ (quad*);
extern void generate_IF_LESSEQ (quad*);
extern void generate_IF_GREATEREQ (quad*);

extern void generate_IF_LESS (quad*);
extern void generate_IF_GREATER (quad*);


extern void generate_CALL (quad*);
extern void generate_PARAM (quad*);

extern void generate_FUNCSTART (quad*);
extern void generate_FUNCEND (quad*);


extern void generate_TABLECREATE (quad*);
extern void generate_TABLEGETELEM (quad*);
extern void generate_TABLESETELEM (quad*);

extern void generate_NOP (quad*);

extern void generate_RET (quad*);
extern void generate_GETRETVAL (quad*);


typedef void (*generator_func_t)(quad*);

generator_func_t generators[] = {
	generate_ASSIGN,
	generate_ADD,
	generate_SUB,
	generate_MUL,
	generate_DIV,
	generate_MOD,
	generate_UMINUS,
	generate_JUMP,
	generate_IF_EQ,
	generate_IF_NOTEQ,
	generate_IF_LESSEQ,
	generate_IF_GREATEREQ, 
	generate_IF_LESS,
	generate_IF_GREATER,
	generate_CALL,
	generate_PARAM,
	generate_FUNCSTART,
	generate_FUNCEND,
	generate_TABLECREATE,
	generate_TABLEGETELEM,
	generate_TABLESETELEM,
	generate_NOP,
	generate_GETRETVAL,
	generate_RET

};

typedef struct symbol_stack{
	symbol * sym;
	struct symbol_stack * next;
} symbol_stack;

symbol_stack * funcstack = NULL;

unsigned magic_number = 340200501;
