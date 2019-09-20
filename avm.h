#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <stdarg.h>


#define RESET "\x1B[0m"
#define BLUE  "\033[01;34m"
#define RED   "\033[1m\033[31m"

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



typedef enum avm_memcell{
	NUMBER_M	=0,
	STRING_M	=1,
	BOOL_M		=2,
	TABLE_M 	=3,
	USERFUNC_M	=4,
	LIBFUNC_M	=5,
	NIL_M		=6,
	UNDEF_M		=7,
	INVALID_M   =8
}avm_memcell_t;




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



typedef struct AVM_memcell {
	avm_memcell_t type;

	union{
		double			numVal;
		char *			strVal;
		unsigned char 	boolVal;
		struct AVM_table * tableVal;
		unsigned		funcVal;
		char*			libfuncVal;
	} data;

}avm_memcell;

typedef struct AVM_table_bucket {
	avm_memcell key;
	avm_memcell value;
	struct AVM_table_bucket *next;
}avm_table_bucket;


#define		AVM_TABLE_HASHSIZE	211
#define		HASH_MUL 65599

typedef struct AVM_table {

	unsigned ref_counter;
	avm_table_bucket * str_indexed[AVM_TABLE_HASHSIZE];
	avm_table_bucket * num_indexed[AVM_TABLE_HASHSIZE];
	avm_table_bucket * userfunc_indexed[AVM_TABLE_HASHSIZE];
	avm_table_bucket * lib_indexed[12];
	avm_table_bucket * table_indexed[AVM_TABLE_HASHSIZE];
	avm_table_bucket * bool_indexed[2];
	unsigned id;
	unsigned total;

}avm_table;

unsigned total_arr = 0;

extern instruction* instructions;

instruction * instructions = (instruction*) 0;

unsigned currInstruction = 0;

unsigned magic_number = 0;


struct avm_table;

#define	AVM_STACKSIZE	4096

#define	AVM_WIPEOUT(m)	memset(&(m), 0, sizeof(m))

avm_memcell stack[AVM_STACKSIZE];

static void avm_initstack(void);


//loaded constant arrays
double*		num_consts;

unsigned	total_num_consts;

unsigned	curr_num_consts;



char**		string_consts;

unsigned	total_string_consts;

unsigned	curr_string_consts;



char**		named_libfuncs;

unsigned	total_named_libfuncs;

unsigned	curr_named_libfuncs;


userfunc *	user_funcs;

unsigned	total_user_funcs;

unsigned	curr_user_funcs;



#define AVM_STACKENV_SIZE 4

avm_memcell ax, bx, cx;

avm_memcell retval;

unsigned top, topsp;

unsigned execution_finished;

unsigned pc;

unsigned curr_line;


extern void execute_ASSIGN (instruction *);
extern void execute_ADD (instruction *);
extern void execute_SUB (instruction *);
extern void execute_MUL (instruction *);
extern void execute_DIV (instruction *);
extern void execute_MOD (instruction *);

#define execute_ADD execute_arithmetic
#define execute_SUB execute_arithmetic
#define execute_MUL execute_arithmetic
#define execute_DIV execute_arithmetic
#define execute_MOD execute_arithmetic
#define execute_UMINUS execute_arithmetic

extern void execute_JUMP (instruction *);


#define execute_JLE execute_relational
#define execute_JGE execute_relational
#define execute_JLT execute_relational
#define execute_JGT execute_relational


extern void execute_JEQ(instruction * unused);
extern void execute_JNE(instruction * unused);
extern void execute_JLE(instruction * unused);
extern void execute_JGE(instruction * unused);
extern void execute_JLT(instruction * unused);
extern void execute_JGT(instruction * unused);


extern void execute_CALL (instruction *);
extern void execute_PUSHARG (instruction *);

extern void execute_FUNCENTER (instruction *);
extern void execute_FUNCEXIT (instruction *);


extern void execute_NEWTABLE (instruction *);
extern void execute_TABLEGETELEM (instruction *);
extern void execute_TABLESETELEM (instruction *);

extern void execute_NOP (instruction *);

extern void execute_RET (instruction *);
extern void execute_GETRETVAL (instruction *);

extern void execute_arithmetic(instruction * instr);
extern void execute_relational(instruction * instr);



typedef void (*execute_func_t)(instruction *);

execute_func_t executors[] = {
	execute_ASSIGN,
	execute_ADD,
	execute_SUB,
	execute_MUL,
	execute_DIV,
	execute_MOD,
	execute_UMINUS,
	execute_JUMP,
	execute_JEQ,
	execute_JNE,
	execute_JLE,
	execute_JGE,
	execute_JLT,
	execute_JGT,
	execute_CALL,
	execute_PUSHARG,
	execute_FUNCENTER,
	execute_FUNCEXIT,
	execute_NEWTABLE,
	execute_TABLEGETELEM,
	execute_TABLESETELEM,
	execute_NOP,
	0,
	0

};



void avmtable_inc_refcounter(avm_table * table);
#define AVM_ENDING_PC currInstruction

void execute_cycle();

typedef void(*memclear_func_t)(avm_memcell *);

extern void memclear_string(avm_memcell * m);

extern void memclear_table(avm_memcell * m);


memclear_func_t memclear_funcs[]= {
	0,	//NUMBER_M
	memclear_string,
	0, //"BOOL_M"
	memclear_table,
	0, //USERFUNC_M
	0, // LIBFUNC_M
	0, // NIL_M
	0, // UNDEF_M
	0 // INVALID_M
};


extern void avm_warning(char * format, ...);
extern void avm_error(char * format, ...);
extern void avm_assign (avm_memcell * lv, avm_memcell * rv);




#define AVM_NUMACTUALS_OFFSET 	4
#define AVM_SAVEDPC_OFFSET 		3
#define AVM_SAVEDTOP_OFFSET 	2
#define AVM_SAVEDTOPSP_OFFSET 	1

unsigned total_actuals;
unsigned total_globals;
typedef void (*library_func_t)(void);

extern void libfunc_print();
extern void libfunc_typeof();
extern void libfunc_totalarguments();
extern void libfunc_argument();
extern void libfunc_sqrt();
extern void libfunc_cos();
extern void libfunc_sin();
extern void libfunc_strtonum();

extern void libfunc_objecttotalmembers();

extern void libfunc_objectmemberkeys();

extern void libfunc_objectcopy();
extern void libfunc_input();

library_func_t libFuncs[] = {

	libfunc_print,
	libfunc_typeof,
	libfunc_totalarguments,
	libfunc_argument,
	libfunc_sqrt,
	libfunc_cos,
	libfunc_sin,
	libfunc_strtonum,
	libfunc_objecttotalmembers,
	libfunc_objectmemberkeys,
	libfunc_objectcopy,
	libfunc_input,

};


library_func_t avm_get_libfunc(char * id);


char * libfunc_names[] = {
	"print",
	"typeof",
	"totalarguments",
	"argument",
	"sqrt",
	"cos",
	"sin",
	"strtonum",
	"objecttotalmembers",	
	"objectmemberkeys",
	"objectcopy",
	"input"
	
};

unsigned libfuncs_total = 12;

















typedef char * (*tostring_func_t)(avm_memcell *);

extern char * number_tostring(avm_memcell *);

extern char * string_tostring(avm_memcell *);

extern char * bool_tostring(avm_memcell *);

extern char * table_tostring(avm_memcell *);

extern char * userfunc_tostring(avm_memcell *);

extern char * libfunc_tostring(avm_memcell *);

extern char * nil_tostring(avm_memcell *);

extern char * undef_tostring(avm_memcell *);


tostring_func_t tostringFuncs[] = {

	number_tostring,
	string_tostring,
	bool_tostring,
	table_tostring,
	userfunc_tostring,
	libfunc_tostring,
	nil_tostring,
	undef_tostring
};


typedef void  (*setelem_t)(avm_table * , avm_memcell * , avm_memcell *);

extern void setelem_num(avm_table * , avm_memcell * , avm_memcell *);

extern void setelem_string(avm_table * , avm_memcell * , avm_memcell *);

extern void setelem_bool(avm_table * , avm_memcell * , avm_memcell *);

extern void setelem_table(avm_table * , avm_memcell * , avm_memcell *);

extern void setelem_userfunc(avm_table * , avm_memcell * , avm_memcell *);

extern void setelem_lib(avm_table * , avm_memcell * , avm_memcell *);

extern void setelem_nil(avm_table * , avm_memcell * , avm_memcell *);

extern void setelem_undef(avm_table * , avm_memcell * , avm_memcell *);


setelem_t setelemFuncs[] = {

	setelem_num,
	setelem_string,
	setelem_bool,
	setelem_table,
	setelem_userfunc,
	setelem_lib,
	0,
	0
};



typedef double (*arithmetic_func_t) (double x, double y);

extern double add_impl(double x, double y);
extern double sub_impl(double x, double y);
extern double mul_impl(double x, double y);
extern double div_impl(double x, double y);
extern double mod_impl(double x, double y);
extern double uminus_impl(double x, double y);

arithmetic_func_t arithmeticFuncs[] = {
	add_impl,
	sub_impl,
	mul_impl,
	div_impl,
	mod_impl,
	uminus_impl
};



typedef unsigned char (*relational_func_t) (double x, double y);

extern unsigned char jle_impl(double x, double y);
extern unsigned char jge_impl(double x, double y);
extern unsigned char jlt_impl(double x, double y);
extern unsigned char jgt_impl(double x, double y);

relational_func_t relationalFuncs[] = {
	jle_impl,
	jge_impl,
	jlt_impl,
	jgt_impl
};




typedef unsigned char (*tobool_func_t)(avm_memcell *);

extern unsigned char number_tobool(avm_memcell *);

extern unsigned char string_tobool(avm_memcell *);

extern unsigned char bool_tobool(avm_memcell *);

extern unsigned char table_tobool(avm_memcell *);

extern unsigned char userfunc_tobool(avm_memcell *);

extern unsigned char libfunc_tobool(avm_memcell *);

extern unsigned char nil_tobool(avm_memcell *);

extern unsigned char undef_tobool(avm_memcell *);


tobool_func_t toboolFuncs[] = {

	number_tobool,
	string_tobool,
	bool_tobool,
	table_tobool,
	userfunc_tobool,
	libfunc_tobool,
	nil_tobool,
	undef_tobool
};


char * typeStrings[] = {

	"\"number\"",
	"\"string\"",
	"\"bool\"",
	"\"table\"",
	"\"userfunc\"",
	"\"libfunc\"",
	"\"nil\"",
	"\"undef\""
};


