#include "avm.h"
#include "avm_other.c"
#include "libfuncs.c"



avm_memcell * avm_tablegetelem(avm_table * table , avm_memcell * index);



void read_binary(){

	FILE *fd;
	int i;
	int length;

	fd = fopen("tcode.abc","rb");

	fread(&magic_number,sizeof(unsigned),1,fd);

	assert(magic_number==340200501);



	fread(&curr_string_consts,sizeof(unsigned),1,fd);

	if(curr_string_consts)
		string_consts = (char ** ) malloc ( curr_string_consts * sizeof(char *));

	for(i=0;i<curr_string_consts;++i){

		fread(&length,sizeof(unsigned),1,fd);

		string_consts[i] = (char *) malloc (length * sizeof(char));

		fread(string_consts[i], length * sizeof(char) ,sizeof(char),fd);

	}

	
	fread(&curr_num_consts,sizeof(unsigned),1,fd);

	if(curr_num_consts)
		num_consts = (double * ) malloc ( curr_num_consts * sizeof(double ));

	

	for(i=0;i<curr_num_consts;++i){

		fread(&num_consts[i],sizeof(double),1,fd);
	}


	fread(&curr_user_funcs,sizeof(unsigned),1,fd);

	if(curr_user_funcs)
		user_funcs = (userfunc *)malloc( curr_user_funcs * sizeof(userfunc));


	for(i=0;i<curr_user_funcs;++i){


		fread(&user_funcs[i].address,sizeof(unsigned),1,fd);

		fread(&user_funcs[i].localSize,sizeof(unsigned),1,fd);

		fread(&length,sizeof(unsigned),1,fd);

		user_funcs[i].id = (char *)malloc (length);

		fread(user_funcs[i].id,length * sizeof(char) ,1,fd);

	}


	fread(&curr_named_libfuncs,sizeof(unsigned),1,fd);

	if(curr_named_libfuncs)
		named_libfuncs = (char ** ) malloc ( curr_named_libfuncs * sizeof(char *));

	for(i=0;i<curr_named_libfuncs;++i){

		fread(&length,sizeof(unsigned),1,fd);

		named_libfuncs[i] = (char *)malloc (length * sizeof(char));

		fread(named_libfuncs[i],length * sizeof(char),1,fd);

	}

	fread(&currInstruction,sizeof(unsigned),sizeof(unsigned),fd);

	if(currInstruction)
		instructions = (instruction *)malloc (currInstruction * sizeof(instruction));


	for (i = 0; i < currInstruction; i++) {

		fread(&instructions[i],sizeof(instruction),1,fd);

	}
	fread(&total_globals,sizeof(unsigned),1,fd);

	fclose(fd);

}



avm_memcell * avm_translate_operand(vmarg *  arg , avm_memcell * reg){

	
	switch (arg->type) {

		case GLOBAL_A: 
		{
			return &stack [ AVM_STACKSIZE - 1 - arg->val];
		}
		case LOCAL_A:
		{
			return &stack [ topsp - arg->val];
		}
		case FORMAL_A:
		{
			return &stack [ topsp + AVM_STACKENV_SIZE + 1 + arg->val];
		}
		case RETVAL_A:
		{
			return &retval;
		}

		case NUMBER_A: 
		{
			reg->type = NUMBER_M;
			reg->data.numVal = consts_getnumber(arg->val);	
			return reg;
		}

		case STRING_A: 
		{
			reg->type = STRING_M;
			reg->data.strVal = strdup(consts_getstring(arg->val));	
			return reg;
		}

		case BOOL_A: 
		{

			reg->type = BOOL_M;
			reg->data.boolVal = arg->val;	
			return reg;	
		}

		case NIL_A: 
		{
			reg->type = NIL_M; 
			return reg;
		}

		case USERFUNC_A: 
		{
			reg->type = USERFUNC_M;
			reg->data.funcVal = user_funcs[arg->val].address;
			return reg;
		}

		case LIBFUNC_A: 
		{

			reg->type = LIBFUNC_M;
			reg->data.libfuncVal = libfuncs_getused(arg->val);
			return reg;
		}

		case INVALID_A:
		{
			reg=(avm_memcell *)0;
			return reg;
		}

		default: {
			assert(0);
		}
	}
}


void execute_cycle(){


	execution_finished = pc = curr_line = 0;

	printf("Total Instructions : %d\n\n",currInstruction);

	while(1){

		if(pc == AVM_ENDING_PC){
			printf(GREEN"\nExecution Finished.\xE2\x9C\x93\n\n"RESET);
			return;
		}else if (execution_finished){
			printf(RED"\nExecution Finished with runtime errors.\n"RESET);
			return;
		}

		assert(pc < AVM_ENDING_PC);

		instruction * instr = instructions + pc;
		assert (instr);

		if(instr->src_line){
			curr_line = instr->src_line;
		}

		unsigned oldpc = pc;

	 // printf("pc: %3d ,executing %s \n",pc,topcode_to_text(instr->opcode) );


		(*executors[instr->opcode])(instr);

		if(pc == oldpc) ++pc;
	}
}

void avm_memcell_clear(avm_memcell * m){

	
	if (m->type != UNDEF_M && m->type < 8){

		memclear_func_t f = memclear_funcs[m->type];
		if(f)
			(*f)(m);

		m->type = UNDEF_M;
	}
}


void avm_dec_top(){

	if(!top){
		avm_error("Stack overflow\n");

	}else{
		--top;
	}
}


void avm_push_envvalue(unsigned val){

	stack[top].type = NUMBER_M;
	stack[top].data.numVal = val;
	avm_dec_top();
}


unsigned avm_get_envvalue(unsigned i){

	assert(stack[i].type == NUMBER_M);
	unsigned val = (unsigned) stack[i].data.numVal;
	assert(stack[i].data.numVal == ((double) val));
	return val;
}


void avm_callsave_environment(){

	avm_push_envvalue(total_actuals);
	avm_push_envvalue(pc + 1);
	avm_push_envvalue(top + total_actuals + 2);
	avm_push_envvalue(topsp);
}


userfunc * avm_getfunc_info(unsigned address){ 

	int i;

	for(i = 0 ; i< curr_user_funcs ; i++){
		if(user_funcs[i].address == address){
			 return user_funcs+i;
		}
	}
}


unsigned avm_total_actuals(void){

	return avm_get_envvalue(topsp + AVM_NUMACTUALS_OFFSET);
}


avm_memcell * avm_get_actual(unsigned i){

	assert(i < avm_total_actuals());
	return &stack[ topsp + AVM_STACKENV_SIZE + 1 + i];
}



void execute_JUMP(instruction * instr){
	
	if(instructions[instr->result.val].opcode == ENTERFUNC_V){
		instr->result.val--;
	}
	pc = instr->result.val;
}



void execute_NOP(instruction * unused){}




void avm_assign (avm_memcell * lv, avm_memcell * rv){

	if (lv == rv) return;

	if (lv->type == TABLE_M && rv->type == TABLE_M && lv->data.tableVal == rv->data.tableVal) return;

	if(rv->type == UNDEF_M){
		avm_warning("Assignment from 'undef' content!\n");
	}

	if(rv->type == TABLE_M)

	avm_memcell_clear(lv);

	memcpy(lv,rv,sizeof(avm_memcell));

	if(lv->type == STRING_M){

		lv->data.strVal = strdup(rv->data.strVal);
	}
	else if (lv->type == TABLE_M){
		avmtable_inc_refcounter(lv->data.tableVal);
	}
}





void execute_ASSIGN(instruction * instr){

	avm_memcell * lv = avm_translate_operand(&instr->result, (avm_memcell*)0 );
	avm_memcell * rv = avm_translate_operand(&instr->arg1, &ax );

	assert(lv);
	assert(&stack[AVM_STACKSIZE-1] >= lv);
	assert(lv >= &stack[0] || lv == &retval ) ;

	assert(rv);

	avm_assign(lv,rv);
}





void execute_FUNCENTER ( instruction * instr){

		avm_memcell * func = avm_translate_operand(&instr->result, &ax);
		assert(func);
		assert(pc == func->data.funcVal);

		total_actuals = 0;
		userfunc * func_info = avm_getfunc_info(pc);
		topsp = top;
		top = top - func_info->localSize;
}





void execute_FUNCEXIT(instruction * unused){

	unsigned oldtop = top;

	top 	= avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET);
	pc 		= avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);
	topsp 	= avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);

	while(++oldtop <= top) {
		avm_memcell_clear(&stack[oldtop]);
	}
}




void execute_CALL (instruction * instr) {

	avm_memcell * func = avm_translate_operand(&instr->result, &ax);

	assert(func);

	avm_callsave_environment();	

	switch (func->type ) {

		case USERFUNC_M: 
		{

			pc = func->data.funcVal;
			
			assert(pc < AVM_ENDING_PC);
			assert(instructions[pc].opcode == ENTERFUNC_V);
			break;

		}

		case STRING_M:
		{
			avm_call_libfunc(func->data.strVal);
			
			break;
		}

		case LIBFUNC_M:
		{
		
			avm_call_libfunc(func->data.libfuncVal);
			break;
		}
		case TABLE_M:
		{
			avm_memcell * new = (avm_memcell *)malloc(sizeof(avm_memcell));
			new->type = STRING_M;
			new->data.strVal = "()";

			avm_memcell * tmp = avm_tablegetelem(func->data.tableVal,new);

			if(tmp){
				pc = tmp->data.funcVal;
				assert(pc < AVM_ENDING_PC);
				assert(instructions[pc].opcode == ENTERFUNC_V);
			}else{
				avm_error("Error in functor");
			}
			break;
		}

		default:
		{
			char * s = avm_tostring(func);
			avm_error("Call: cannot bind to function %s \n", s);
		}

	}
}



void execute_PUSHARG ( instruction * instr ) {

	
	avm_memcell * arg = avm_translate_operand(&instr->result, &ax);
	assert(arg);
	
	avm_assign(&stack[top],arg);
	++total_actuals;
	avm_dec_top();
}






//arithm and relop
unsigned char jle_impl (double x, double y) {return x<=y;}

unsigned char jge_impl (double x, double y) {return x>=y;}

unsigned char jlt_impl (double x, double y) {return x<y;}

unsigned char jgt_impl (double x, double y) {return x>y;}

double add_impl ( double x , double y )	{return x+y;}

double sub_impl ( double x , double y )	{return x-y;}

double mul_impl ( double x , double y )	{return x*y;}

double div_impl ( double x , double y ){

	if(almost_eq(y,0)){
		avm_error("Cannot div with zero\n");

		return -1;
	}else{
		return x/y;
	}
}

double uminus_impl ( double x, double y){return x*y;}

double mod_impl ( double x , double y ){

	if(almost_eq(y,0)){

		avm_error("Cannot mod with zero\n");

		return -1;
	}else{
		return ( (unsigned)x % (unsigned)y );
	}
}




void execute_arithmetic(instruction * instr){


	avm_memcell * lv = avm_translate_operand(&instr->result, (avm_memcell*)0 );

	avm_memcell * rv1 = avm_translate_operand(&instr->arg1, &ax );

	avm_memcell * rv2 = avm_translate_operand(&instr->arg2, &bx );

	assert(lv);
	assert(&stack[AVM_STACKSIZE-1] >= lv);
	assert(lv > &stack[0] || lv == &retval);

	assert(rv1 && rv2);

	if(rv1->type != NUMBER_M || rv2->type != NUMBER_M){
		avm_error("not a number in arithmetic!\n");
		

	}else{

		arithmetic_func_t op = arithmeticFuncs[instr->opcode - ADD_V];
		avm_memcell_clear(lv);
		lv->type = NUMBER_M;
		lv->data.numVal = (*op)(rv1->data.numVal,rv2->data.numVal);
	}
}


void execute_relational(instruction* instr) {

	unsigned char lv = 0;

	avm_memcell * rv1 = avm_translate_operand(&instr->arg1, &ax );

	avm_memcell * rv2 = avm_translate_operand(&instr->arg2, &bx );

	assert(rv1 && rv2);

	if (rv1->type != NUMBER_M || rv2->type != NUMBER_M) {

		avm_error("not a number in relational!\n");

	}else {

		relational_func_t op = relationalFuncs[instr->opcode - JLE_V];

		lv = (*op)(rv1->data.numVal,rv2->data.numVal);

		if(lv){
			pc = instr->result.val;
		}
	}
}



void execute_JEQ (instruction * instr){


	assert(instr->result.type == LABEL_A);

	avm_memcell * rv1 = avm_translate_operand(&instr->arg1,&ax);

	avm_memcell * rv2 = avm_translate_operand(&instr->arg2,&bx);

	unsigned char result =0;


	if(rv1->type == UNDEF_M || rv2->type == UNDEF_M){

		avm_warning("'undef' involved in equality!\n");

	}else if (rv1->type == NIL_M || rv2->type == NIL_M){

		result = rv1->type == NIL_M && rv2->type == NIL_M;

	}else if (rv1->type == BOOL_M || rv2->type == BOOL_M){

		result = avm_tobool(rv1) == avm_tobool(rv2);

	}else if (rv1->type != rv2->type ){

		avm_error(" %s == %s is illegal\n",typeStrings[rv1->type], typeStrings[rv2->type]);

	}else{

		switch(rv1->type){

			case NUMBER_M:
			{
				result = rv1->data.numVal==rv2->data.numVal;
				break;
			}
				
			case STRING_M:
			{
				if(!strcmp(rv1->data.strVal,rv2->data.strVal)){
					result = 1;
				}
				break;
			}

			case TABLE_M:
			{
				if(rv1->data.tableVal==rv2->data.tableVal){
					result=1;
				}
				break;
			}

			case USERFUNC_M:
			{
				if(!strcmp(userfunc_tostring(rv1),userfunc_tostring(rv2))){
					result = 1;
				}
				break;
			}

			case LIBFUNC_M:
			{
			
				if(!strcmp(libfunc_tostring(rv1),libfunc_tostring(rv2))){
					result = 1;
				}
				break;
			}
			default: assert(0);
		}
	}

	if(!execution_finished && result){
		pc = instr->result.val;
	}
}


void execute_JNE (instruction * instr){


	assert(instr->result.type == LABEL_A);

	avm_memcell * rv1 = avm_translate_operand(&instr->arg1,&ax);

	avm_memcell * rv2 = avm_translate_operand(&instr->arg2,&bx);

	unsigned char result =0;



	if(rv1->type == UNDEF_M || rv2->type == UNDEF_M){

		avm_warning("'undef' involved in equality!\n");

	}else if (rv1->type == NIL_M || rv2->type == NIL_M){

		result = rv1->type == NIL_M != rv2->type == NIL_M;

	}else if (rv1->type == BOOL_M || rv2->type == BOOL_M){

		result = avm_tobool(rv1) != avm_tobool(rv2);

	}else if (rv1->type != rv2->type ){

		avm_error(" %s != %s is illegal\n",typeStrings[rv1->type], typeStrings[rv2->type]);

	}else{

		switch(rv1->type){

			case NUMBER_M:
			{
				result = rv1->data.numVal!=rv2->data.numVal;
				break;
			}
				
			case STRING_M:
			{
				if(strcmp(rv1->data.strVal,rv2->data.strVal)){
			
					result = 1;
				}
				break;
			}

			case TABLE_M:
			{
				if(rv1->data.tableVal!=rv2->data.tableVal){
					result=1;
				}
				break;
			}

			case USERFUNC_M:
			{
				if(strcmp(userfunc_tostring(rv1),userfunc_tostring(rv2))){
					result = 1;
				}
				break;
			}

			case LIBFUNC_M:
			{
			
				if(strcmp(libfunc_tostring(rv1),libfunc_tostring(rv2))){
					result = 1;
				}
				break;
			}
			default: assert(0);
		}
	}

	if(!execution_finished && result){
		pc = instr->result.val;
	}
}


//TABLES
void avm_tablebuckets_init(avm_table_bucket ** p){

	int i;
	for(i=0;i<AVM_TABLE_HASHSIZE;++i){
		p[i] = (avm_table_bucket *)0;
	}
}

avm_table * avm_tablenew(){

	avm_table * t = (avm_table *)malloc(sizeof(avm_table));

	AVM_WIPEOUT(*t);

	t->ref_counter = t->total = 0;
	avm_tablebuckets_init(t->str_indexed);
	avm_tablebuckets_init(t->num_indexed);
	t->id = total_arr++;
	return t;
}


void avm_tablebuckets_destroy(avm_table_bucket ** p){

	int i;
	for(i = 0; i < AVM_TABLE_HASHSIZE; ++i, ++p){

		avm_table_bucket * tmp = *p;

		for(; tmp != NULL;){

			avm_table_bucket * del = tmp;
			tmp = tmp->next ;
			avm_memcell_clear(&del->key);
			avm_memcell_clear(&del->value);
			free(del);
		}
		p[i] = (avm_table_bucket*)0;
	}
}


void avm_tabledestroy(avm_table * t){
	avm_tablebuckets_destroy(t->str_indexed);
	avm_tablebuckets_destroy(t->num_indexed);

}



void avmtable_dec_refcounter(avm_table * table){

	assert(table->ref_counter > 0);
	if(!--table->ref_counter)
		avm_tabledestroy(table);
}


void avmtable_inc_refcounter(avm_table * table){
	++table->ref_counter;
}


void memclear_string(avm_memcell * m){
	assert(m->data.strVal);
	free(m->data.strVal);
}



void memclear_table(avm_memcell * m){
	assert(m->data.tableVal);
	avmtable_dec_refcounter(m->data.tableVal);
}



void execute_NEWTABLE(instruction * instr) {
	
	avm_memcell * lv = avm_translate_operand(&instr->result, (avm_memcell*)0 );

	assert(lv);
	assert(&stack[AVM_STACKSIZE-1] >= lv);
	assert(lv > &stack[0] || lv == &retval);
	
	
	lv->type = TABLE_M;
	lv->data.tableVal = avm_tablenew();
	avmtable_inc_refcounter(lv->data.tableVal);

}



unsigned avm_hash(avm_memcell* m){

	unsigned i;
	unsigned uiHash = 0U;

	switch(m->type){

		case STRING_M:
		{
			char * key = m->data.strVal;
		
			for (i=0U; key[i] != '\0'; i++) 
				uiHash = uiHash * HASH_MUL + key[i];
			break;
		}

		case NUMBER_M:
		{
			double key = m->data.numVal;
			uiHash = key * HASH_MUL;
			break;
		}

		case USERFUNC_M:
		{
			double key = m->data.funcVal;
			uiHash = key * HASH_MUL;
			break;
		}

		case BOOL_M:
		{
			double key = m->data.boolVal;
			uiHash = key + HASH_MUL;
			uiHash%=2;
			break;
		}

		case LIBFUNC_M:
		{
			char * key = m->data.libfuncVal;
			for (i=0U; key[i] != '\0'; i++) {

				uiHash = uiHash * HASH_MUL + key[i];
				uiHash%=11;
			}
			break;
		}

		case TABLE_M:
		{
			double key = m->data.tableVal->id;
			uiHash = key * HASH_MUL;
			break;
		}

		default:
		{
			avm_error("unsupported index type : %s\n",typeStrings[m->type]);
		}

	}

	return uiHash % AVM_TABLE_HASHSIZE;
}



avm_memcell * avm_tablegetelem ( avm_table * table , avm_memcell * index){

	unsigned i;
	i = avm_hash(index);

	avm_table_bucket *tmp = NULL;
	
	switch(index->type){

		case STRING_M:
		{
			for(tmp = table->str_indexed[i]; tmp!= NULL ; tmp = tmp->next ){
		
				if (!strcmp(tmp->key.data.strVal,index->data.strVal)) 
					return &tmp->value;	
			}
		}

		case NUMBER_M:
		{
			for(tmp = table->num_indexed[i];tmp!= NULL; tmp = tmp->next){

				if (tmp->key.data.numVal==index->data.numVal) 
					return &tmp->value;
			}
		}

		case USERFUNC_M:
		{
			for(tmp = table->userfunc_indexed[i];tmp!= NULL; tmp = tmp->next){

				if (tmp->key.data.funcVal == index->data.funcVal) 
					return &tmp->value;	
			}
		}

		case BOOL_M:
		{
			for(tmp = table->bool_indexed[i];tmp!= NULL; tmp = tmp->next){

				if (tmp->key.data.boolVal == index->data.boolVal) 
					return &tmp->value;
			
			}
		}

		case LIBFUNC_M:
		{
			for(tmp = table->lib_indexed[i];tmp!= NULL; tmp = tmp->next){

				if (!strcmp(tmp->key.data.libfuncVal,index->data.libfuncVal) ) 
					return &tmp->value;
			}
		}

		case TABLE_M:
		{
			for(tmp = table->table_indexed[i];tmp!= NULL; tmp = tmp->next){

				if (tmp->key.data.tableVal->id == index->data.tableVal->id) 
					return &tmp->value;
			
			}	
		}
		default:
		{
			avm_error("Unsupported table get elem");
			return NULL;
		}
	}
}
	


void execute_TABLEGETELEM(instruction * instr) {

	avm_memcell * lv = avm_translate_operand(&instr->result, (avm_memcell*)0 );

	avm_memcell * t = avm_translate_operand(&instr->arg1, (avm_memcell*)0 );

	avm_memcell * i = avm_translate_operand(&instr->arg2,&ax );


	assert(lv);
	assert(&stack[AVM_STACKSIZE-1] >= lv);
	assert(lv > &stack[0] || lv == &retval);

	assert(t);
	assert(&stack[AVM_STACKSIZE-1] >= t);
	assert(t > &stack[0] || t == &retval);
	
	assert(i);

	avm_memcell_clear(lv);

	lv->type = NIL_M;

	if(t->type != TABLE_M){
		avm_error("Illegal use of %s as a table\n",typeStrings[t->type]);

	}else{

		avm_memcell * content = avm_tablegetelem(t->data.tableVal,i);

		if(content){
			
			avm_assign(lv,content);

		}else{
			char * is = strdup(avm_tostring(i));
			avm_warning(" array[%s] not found!\n",is);
			free(is);
		}
	}
}



void setelem_string(avm_table * table , avm_memcell * key , avm_memcell * value){


	unsigned i ;

	avm_table_bucket *tmp = NULL;
	avm_table_bucket *prev = NULL;

	char * c = key->data.strVal;

	if(!strcmp( key->data.strVal,"\"()\"" )){
		key->data.strVal="()";
	}

	i= avm_hash(key);

	if(table->str_indexed[i] == NULL){
		
		avm_table_bucket * new = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
		
		avm_assign(&new->key, key);
		avm_assign(&new->value, value);

		table->str_indexed[i] = new;

		new->next = NULL;
		table->total++;

		return;

	}

	for(tmp = table->str_indexed[i]; tmp != NULL  ; tmp= tmp->next) {
		
		prev = tmp;
		if (!strcmp(tmp->key.data.strVal,key->data.strVal)){

			if (value->type == NIL_M){

				prev->next = tmp->next;
				avm_memcell_clear(&tmp->value);
				avm_memcell_clear(&tmp->key);
				tmp->value.type = NIL_M;

				return;

			} else {

				avm_memcell_clear(&tmp->value);
				avm_assign(&tmp->value, value);
				return;
			}
		}
		
	}

	avm_table_bucket * new = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
	avm_assign(&new->key,key);
	avm_assign(&new->value,value);
	prev->next = new;
	new->next = NULL;
	table->total++;
	return;
}

void setelem_num(avm_table * table , avm_memcell * key , avm_memcell * value){


	unsigned i ;

	avm_table_bucket *tmp = NULL;
	avm_table_bucket *prev = NULL;


	i= avm_hash(key);

	if(table->num_indexed[i] == NULL){
		
		avm_table_bucket * new = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
		
		avm_assign(&new->key, key);
		avm_assign(&new->value, value);

		table->num_indexed[i] = new;
		new->next = NULL;
		table->total++;
		return;

	}

	for(tmp = table->num_indexed[i]; tmp != NULL  ; tmp= tmp->next) {
		
		
		if (almost_eq(tmp->key.data.numVal,key->data.numVal)){

			if (value->type == NIL_M){

				prev->next = tmp->next;
				avm_memcell_clear(&tmp->value);
				avm_memcell_clear(&tmp->key);

				return;

			} else {

				avm_memcell_clear(&tmp->value);
				avm_assign(&tmp->value, value);
				return;
			}
		}
		prev = tmp;
	}

	avm_table_bucket * new = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
	avm_assign(&new->key,key);
	avm_assign(&new->value,value);
	prev->next = new;
	new->next = NULL;
	table->total++;
	return;
}

void setelem_userfunc(avm_table * table , avm_memcell * key , avm_memcell * value){


	unsigned i ;

	avm_table_bucket *tmp = NULL;
	avm_table_bucket *prev = NULL;


	i= avm_hash(key);
		if(table->userfunc_indexed[i] == NULL){
			
			avm_table_bucket * new = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
			
			avm_assign(&new->key, key);
			avm_assign(&new->value, value);

			table->userfunc_indexed[i] = new;
			new->next = NULL;
			table->total++;
			return;

		}

		for(tmp = table->userfunc_indexed[i]; tmp != NULL  ; tmp= tmp->next) {
			
			
			if (tmp->key.data.funcVal == key->data.funcVal){

				if (value->type == NIL_M){

					prev->next = tmp->next;
					avm_memcell_clear(&tmp->value);
					avm_memcell_clear(&tmp->key);

					return;

				} else {

					avm_memcell_clear(&tmp->value);
					avm_assign(&tmp->value, value);
					return;
				}
			}
			prev = tmp;
		}

		avm_table_bucket * new = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
		avm_assign(&new->key,key);
		avm_assign(&new->value,value);
		prev->next = new;
		new->next = NULL;
		table->total++;
		return;
}

void setelem_bool(avm_table * table , avm_memcell * key , avm_memcell * value){


	unsigned i ;

	avm_table_bucket *tmp = NULL;
	avm_table_bucket *prev = NULL;


	i= avm_hash(key);

	if(table->bool_indexed[i] == NULL){
		
		avm_table_bucket * new = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
		
		avm_assign(&new->key, key);
		avm_assign(&new->value, value);

		table->bool_indexed[i] = new;
		new->next = NULL;
		table->total++;
		return;

	}

	for(tmp = table->bool_indexed[i]; tmp != NULL  ; tmp= tmp->next) {
		
	
		if (tmp->key.data.boolVal == key->data.boolVal){

			if (value->type == NIL_M){

				prev->next = tmp->next;
				avm_memcell_clear(&tmp->value);
				avm_memcell_clear(&tmp->key);

				return;

			} else {

				avm_memcell_clear(&tmp->value);
				avm_assign(&tmp->value, value);
				return;
			}
		}
		prev = tmp;
	}

	avm_table_bucket * new = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
	avm_assign(&new->key,key);
	avm_assign(&new->value,value);
	prev->next = new;
	new->next = NULL;
	table->total++;
	return;
}

void setelem_lib(avm_table * table , avm_memcell * key , avm_memcell * value){


	unsigned i ;

	avm_table_bucket *tmp = NULL;
	avm_table_bucket *prev = NULL;


	i= avm_hash(key);
	if(table->lib_indexed[i] == NULL){
		
		avm_table_bucket * new = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
		
		avm_assign(&new->key, key);
		avm_assign(&new->value, value);

		table->lib_indexed[i] = new;
		new->next = NULL;
		table->total++;
		return;

	}

	for(tmp = table->lib_indexed[i]; tmp != NULL  ; tmp= tmp->next) {
		
		
		if (!strcmp(tmp->key.data.libfuncVal,key->data.libfuncVal)){

			if (value->type == NIL_M){

				prev->next = tmp->next;
				avm_memcell_clear(&tmp->value);
				avm_memcell_clear(&tmp->key);

				return;

			} else {

				avm_memcell_clear(&tmp->value);
				avm_assign(&tmp->value, value);
				return;
			}
		}
		prev = tmp;
	}

	avm_table_bucket * new = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
	avm_assign(&new->key,key);
	avm_assign(&new->value,value);
	prev->next = new;
	new->next = NULL;
	table->total++;
	return;
}

void setelem_table(avm_table * table , avm_memcell * key , avm_memcell * value){


	unsigned i ;

	avm_table_bucket *tmp = NULL;
	avm_table_bucket *prev = NULL;


	i= avm_hash(key);

	if(table->table_indexed[i] == NULL){
		
		avm_table_bucket * new = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
		
		avm_assign(&new->key, key);
		avm_assign(&new->value, value);

		table->table_indexed[i] = new;
		new->next = NULL;
		table->total++;
		return;

	}

	for(tmp = table->table_indexed[i]; tmp != NULL  ; tmp= tmp->next) {
		
		
		if (tmp->key.data.tableVal->id == key->data.tableVal->id){

			if (value->type == NIL_M){

				prev->next = tmp->next;
				avm_memcell_clear(&tmp->value);
				avm_memcell_clear(&tmp->key);

				return;

			} else {

				avm_memcell_clear(&tmp->value);
				avm_assign(&tmp->value, value);
				return;
			}
		}
		prev = tmp;
	}

	avm_table_bucket * new = (avm_table_bucket*)malloc(sizeof(avm_table_bucket));
	avm_assign(&new->key,key);
	avm_assign(&new->value,value);
	prev->next = new;
	new->next = NULL;
	table->total++;
	return;
}


void avm_tablesetelem ( avm_table * table , avm_memcell * key , avm_memcell * value){

	assert(key->type >= 0 && key->type <= 8);
	setelem_t f = setelemFuncs[key->type];
		if(f)
			(*f)(table,key,value);

}



void execute_TABLESETELEM(instruction * instr) {


	avm_memcell * t = avm_translate_operand(&instr->arg1, (avm_memcell*)0 );

	avm_memcell * i = avm_translate_operand(&instr->arg2, &ax );
	
	avm_memcell * c = avm_translate_operand(&instr->result,&bx );

	assert(t);
	assert(&stack[AVM_STACKSIZE-1] >= t);
	assert(t > &stack[0] || t == &retval);
	
	assert(i && c);

	if(t->type != TABLE_M){
		avm_error("Illegal use of %s as a table\n",typeStrings[t->type]);

	}else{
		avm_tablesetelem(t->data.tableVal,i,c);
	}
}












