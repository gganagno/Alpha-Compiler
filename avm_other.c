

void avm_initialize(){

	avm_initstack();
	top = AVM_STACKSIZE- 1 -total_globals;

	topsp = top;

}

void avm_warning(char * text, ...){
	
	va_list arg;
	int done;

	va_start (arg, text);
	done = vfprintf (stdout, text, arg);
	va_end (arg);
}



void avm_error(char * text, ...){
	
	va_list arg;
	int done;

	va_start (arg, text);
	done = vfprintf (stdout, text, arg);
	va_end (arg);
	execution_finished = 1;

}



int	almost_eq(double a, double b) {

    return (fabs(a - b) < (DBL_EPSILON * fabs(a + b)));
}



static void avm_initstack(){
	int i;
	for (i=0;i<AVM_STACKSIZE;++i ){
		(AVM_WIPEOUT(stack[i]));
		stack[i].type = UNDEF_M;
	}
}


double consts_getnumber(unsigned n){
	return (double)num_consts[n];
}


char * consts_getstring(int n){
	return string_consts[n];
}


char * libfuncs_getused(int n){
	return named_libfuncs[n];
}



unsigned char avm_tobool(avm_memcell * m) {

	assert (m->type >= 0 && m->type <= INVALID_M);
	return (*toboolFuncs[m->type])(m);
}

unsigned char  number_tobool(avm_memcell * m){
	return m->data.numVal != 0;
}

unsigned char  string_tobool(avm_memcell * m){
	return m->data.strVal[0] != 0;
}

unsigned char  bool_tobool(avm_memcell * m){
	return m->data.boolVal;
}

unsigned char  table_tobool(avm_memcell * m){
	return 1;
}

unsigned char  userfunc_tobool(avm_memcell * m){
	return 1;
}

unsigned char  libfunc_tobool(avm_memcell * m){
	return 1;
}

unsigned char  nil_tobool(avm_memcell * m){
	return 0;
}

unsigned char  undef_tobool(avm_memcell * m){
	assert(0);
	return 0;
}



char * avm_tostring(avm_memcell * m){
	assert(m);
	assert(m->type >=0 && m->type <= INVALID_M);
	return (*tostringFuncs[m->type])(m);
} 



char * number_tostring(avm_memcell * m){

	char * str = (char *)malloc(8);
	
	sprintf(str, "%.3f", m->data.numVal);
	return str;
}

char * string_tostring(avm_memcell * m){
	char * c = m->data.strVal;
	if(c[0] == '"'){
		int len = strlen(c);
		char * s = (char*)calloc(len-1, sizeof(char));
		memcpy(s,c+1,len-2);
		return s;
	}
	return c;
}

char * bool_tostring(avm_memcell * m){

	if(m->data.boolVal){
		return "true";
	}else{
		return "false";
	}
}

char * table_tostring(avm_memcell * m){
	
	avm_table_bucket * tmp = NULL;
	unsigned i,j;
	unsigned buck;
	printf("[");

	for(i = 0; i <  AVM_TABLE_HASHSIZE; ++i){

		for(tmp = m->data.tableVal->num_indexed[i]; tmp != NULL ; tmp = tmp->next){
			printf("{ %s : %s }, ",avm_tostring( &tmp->key) , avm_tostring( &tmp->value) );
		}
		for(tmp = m->data.tableVal->str_indexed[i]; tmp != NULL ; tmp = tmp->next){
			printf("{ \"%s\" : %s }, ",avm_tostring( &tmp->key) , avm_tostring( &tmp->value) );
		}
		
		for( tmp = m->data.tableVal->userfunc_indexed[i]; tmp != NULL; tmp = tmp->next){
			printf("{ \"%s\" : %s }, ",avm_tostring( &tmp->key) , avm_tostring( &tmp->value) );
		}

		for( tmp = m->data.tableVal->table_indexed[i]; tmp != NULL; tmp = tmp->next){
			printf("{ \"%s\" : %s }, ",avm_tostring( &tmp->key) , avm_tostring( &tmp->value) );
		}	
	}

	for(i = 0; i< 2 ; ++i){
		for(tmp = m->data.tableVal->bool_indexed[i]; tmp != NULL; tmp = tmp->next){
			printf("{ \"%s\" : %s }, ",avm_tostring( &tmp->key) , avm_tostring( &tmp->value) );
		}
	}

	for(i = 0; i< 22 ; ++i){
		for(tmp = m->data.tableVal->lib_indexed[i]; tmp != NULL; tmp = tmp->next){
			printf("{ \"%s\" : %s }, ",avm_tostring( &tmp->key) , avm_tostring( &tmp->value) );
		}
	}

	printf("]");
	return "\n";
}



char * userfunc_tostring(avm_memcell * m){

	unsigned x = instructions[m->data.funcVal].result.val;
	char * func = (char *)malloc(64);
	sprintf( func,"UserFunc : %s address: %u ",user_funcs[x].id,user_funcs[x].address );
 	return func;
}

char * libfunc_tostring(avm_memcell * m){
	
	return m->data.libfuncVal;
}

char * nil_tostring(avm_memcell * m){
	return "nil";
}

char * undef_tostring(avm_memcell * m){
	return "undef";
}



#define GREEN   "\033[1m\033[32m"

char * vmargt_to_text(vmarg_t type){

	switch(type){
		case LABEL_A	 	: return "LABEL_A";
		case GLOBAL_A 		: return "GLOBAL_A";
		case FORMAL_A 		: return "FORMAL_A";
		case LOCAL_A	 	: return "LOCAL_A";
		case NUMBER_A 		: return "NUMBER_A";
		case STRING_A 		: return "STRING_A";
		case BOOL_A	 		: return "BOOL_A";
		case NIL_A	 		: return "NIL_A";
		case USERFUNC_A 	: return "USERFUNC_A";
		case LIBFUNC_A 		: return "LIBFUNC_A";
		case RETVAL_A 		: return "RETVAL_A";
		default				: return NULL;
	}
}


char * topcode_to_text(vmopcode opcode) {

	switch (opcode) {

	case ASSIGN_V     	: return "ASSIGN_V";
	case ADD_V     		: return "ADD_V";
	case SUB_V      	: return "SUB_V";
	case MUL_V     		: return "MUL_V";
	case DIV_V   		: return "DIVISION_V";
	case MOD_V      	: return "MOD_V";
	case UMINUS_V    	: return "UMINUS_V";
	case JEQ_V      	: return "JEQ_V";
	case JNE_V   		: return "JNE_V";
	case JLE_V    		: return "JLE_V";
	case JGE_V 			: return "JGE_V";
	case JLT_V    		: return "JLT_V";
	case JGT_V   		: return "JGT_V";
	case JUMP_V    		: return "JUMP_V";
	case CALL_V     	: return "CALL_V";
	case PUSHARG_V      : return "PUSHARG_V";
	case ENTERFUNC_V    : return "ENTERFUNC_V";
	case EXITFUNC_V    	: return "EXITFUNC_V";
	case NEWTABLE_V  	: return "NEWTABLE_V";
	case TABLEGETELEM_V : return "TABLEGETELEM_V";
	case TABLESETELEM_V : return "TABLESETELEM_V";
	case NOP_V 			: return "NOP_V";
	default				: return NULL;
	
	}
}

void print_userfuncs(void){

	int i ;
	printf("\n*USER FUNCTIONS TABLE*\n\n");
	printf("curr_user_funcs: %u\n",curr_user_funcs );
	for (i =0;i< curr_user_funcs;i++ ){
		printf("%3d: iAdress: %3u ",i,user_funcs[i].address );
		printf("Locals: %3u ",user_funcs[i].localSize );
		printf("Name: %5s\n\n",user_funcs[i].id );
	}
}

void print_libfuncs(void){

	int i ;
	printf("\n*LIBRARY FUNCTIONS TABLE*\n\n");
	printf("curr_named_libfuncs: %u\n",curr_named_libfuncs );
	for (i =0;i< curr_named_libfuncs;i++ ){
		printf("%3d: Name: %s \n",i,named_libfuncs[i]);
	}
	printf("\n");
}

void print_num(void){
	int i;
	printf("*CONSTANT NUMBERS TABLE*\n\n");
	printf("curr_num_consts: %u\n",curr_num_consts );
	for(i=0;i<curr_num_consts;i++){
		printf("%3d: Number: %.5f \n",i,num_consts[i]);
	}
	printf("\n");
}

void print_strings(void){

	int i;
	printf("*CONSTANT STRINGS TABLE*\n\n");
	printf("curr_string_consts: %u\n",curr_string_consts );
	for(i=0;i<curr_string_consts;i++){

		printf("%3d : %3s \n",i,string_consts[i]);
	}
	printf("\n");
}

void avm_print_arrays(){

	print_strings();
	print_num();
	print_userfuncs();
	print_libfuncs();
}


void print_arg(vmarg arg) {


	switch(arg.type){


			case LABEL_A :
			{
				printf("%d (label), jumpto:%d ",LABEL_A,arg.val);
				break;
			}
			case GLOBAL_A:
			{
				printf("%3d (global), %2d (offset) ",GLOBAL_A,arg.val);
				break;
			}
			case LOCAL_A:
			{
				printf("%4d (local), %d (offset) ",LOCAL_A,arg.val);
				break;
			}
			case FORMAL_A:
			{
				printf("%3d (formal), %2d (offset) ",FORMAL_A,arg.val);
				break;
			}

			case NUMBER_A: {
				printf("%6d (num), %2d:%8.3f ",NUMBER_A,arg.val,num_consts[arg.val]);
				break;		
			}

			case STRING_A: {
				printf("%3d (string), %2d:%8s ",STRING_A,arg.val,string_consts[arg.val]);
				break;
			}

			case BOOL_A: {

				if (arg.val) {
					printf("%5d (bool), %4d (true) ",BOOL_A,arg.val);
				} else {
					printf("%5d (bool), %3d (false) ",BOOL_A,arg.val);
				}
				break;
			}

			case NIL_A:  {

				printf("%2d (nil), %d ",NIL_A,arg.val);
				break;
			}

			case USERFUNC_A: {
				printf("%d (userfunc), %2d:%8s ",USERFUNC_A,arg.val,user_funcs[arg.val].id);
				break;
			}

			case LIBFUNC_A: {
				printf("%d (libfunc), %d:%s ",LIBFUNC_A,arg.val,named_libfuncs[arg.val]);
				break;
			}

			case RETVAL_A: {
				printf("%17d(retval) ",RETVAL_A);
				break;
			}

			case INVALID_A: {

				printf("%26s","- ");
				break;
			}

			default: assert(0);
	}	
}


void print_instructions(void) {

	int i;

	printf("\n*INSTRUCTIONS TABLE*\n\n");

	for (i = 0; i < currInstruction; i++) {

		printf("%3d: ", i);

		printf(GREEN"OP"RESET": %14s ", topcode_to_text(instructions[i].opcode) );

		printf(GREEN"Arg1"RESET": ");

		
		print_arg(instructions[i].arg1);
		

		printf(GREEN"Arg2"RESET": ");

		print_arg(instructions[i].arg2);
		

		printf(GREEN"Result"RESET": ");

		print_arg(instructions[i].result);
		

		printf("\n\n");
	}
	printf("%3d:\n\n", i);

}


void print_memarg(avm_memcell*  cell) {


	switch(cell->type){


			case NUMBER_M: {
				printf("%4d (num), %8.3f ",NUMBER_M,cell->data.numVal);
				break;		
			}

			case STRING_M: {
				printf("%d (string), %s ",STRING_M,cell->data.strVal);
				break;
			}

			case BOOL_M: {

				if (cell->data.boolVal) {
					printf("%d (bool), %d (true) ",BOOL_M,cell->data.boolVal);
				} else {
					printf("%d (bool), %d (false) ",BOOL_M,cell->data.boolVal);
				}
				break;
			}

			case NIL_M:  {

				printf("%d (nil)",NIL_M);
				break;
			}

			case USERFUNC_M: {
				printf("%d (userfunc), %u ",USERFUNC_M,cell->data.funcVal);
				break;
			}

			case LIBFUNC_M: {
				printf("%d (libfunc), %s ",LIBFUNC_M,cell->data.libfuncVal);
				break;
			}

			case UNDEF_M: {
				printf("%14d(undef) ",UNDEF_M);
				break;
			}

			case INVALID_M: {

				printf("%14d(inv) ",INVALID_M);
				break;
			}

			default: assert(0);
	}
	
}