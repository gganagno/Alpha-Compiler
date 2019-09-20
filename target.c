


void expand_target (void) {
	assert(total_target==currInstruction);

	instruction* p = (instruction*)malloc(NEW_SIZE_TARGET);

	if (instructions != NULL) {
		memcpy(p, instructions, CURR_SIZE_TARGET);
		free(instructions);
	}
	instructions = p;
	total_target += EXPAND_SIZE;
}

void emit_target (instruction* p){
	
	if (currInstruction==total_target) expand_target();

	instruction* i = instructions+currInstruction++;
	i->opcode 	= p->opcode;
	i->arg1 	= p->arg1;
	i->arg2 	= p->arg2;
	i->result 	= p->result;
	i->src_line	= p->src_line;
}

unsigned next_instruction_label(void) {
	return currInstruction;
}

unsigned curr_processed_quad(void) {
	return currProcessedQuad;
}



void make_numberoperand (vmarg* arg, double val) {
	arg->val = consts_newnumber(val);
	arg->type = NUMBER_A;
}

void make_booloperand (vmarg* arg, unsigned val) {
	arg->val = val;
	arg->type = BOOL_A;
}

void make_retvaloperand (vmarg* arg) {
	arg->type = RETVAL_A;
}



void make_operand (expr* e, vmarg* arg) {
	

	if(e == NULL){

		arg->val = -1;
		arg->type = INVALID_A;
		return;
	}

	switch (e->type) {

		case VAR_E:  
		case TABLEITEM_E:
		case ARITHEXPR_E:
		case ASSIGNEXPR_E:
		case BOOLEXPR_E:
		case NEWTABLE_E: {

			arg->val = e->sym->offset;
			switch (e->sym->space) {
			
				case PROGRAMVAR: 	arg->type = GLOBAL_A; 	break;
				case FUNCTIONLOCAL: arg->type = LOCAL_A; 	break;
				case FORMALARG: 	arg->type = FORMAL_A; 	break;
				default: assert(0);
			}
			break;
		} 

		case CONSTBOOL_E: {
			make_booloperand(arg,e->boolConst);	
			break;
		}

		case CONSTSTRING_E: {
			arg->val = consts_newstring(e->strConst);
			arg->type = STRING_A;	
			break;
		}

		case CONSTNUM_E: {

			make_numberoperand(arg,e->numConst);	
			break;		
		}

		case NIL_E: {
			arg->type = NIL_A; 
			break;
		}

		case PROGRAMFUNC_E: {

			arg->type = USERFUNC_A;
			arg->val = userfuncs_new(e->sym->taddress,e->sym->total_locals,e->sym->name);
			break;
		}

		case LIBRARYFUNC_E: {
			arg->type = LIBFUNC_A;
			arg->val = libfuncs_newused(e->sym->name);
			break;
		}

		default: assert(0);
	}
	
}

 


void push_funcstack(symbol * new) {
	
	symbol_stack * new_node = (symbol_stack *)malloc(sizeof(symbol_stack));
	new_node->sym = new;

	if (funcstack == NULL) {

		funcstack = new_node;
		funcstack->next = NULL;


	} else {
		
		new_node->next = funcstack;
		funcstack = new_node;
	
	}

}


symbol * pop_funcstack() {
	
	symbol_stack * tmp = NULL;

	if (funcstack == NULL) {
	
		return NULL;

	} else if (funcstack->next == NULL) {

		tmp = funcstack;
		funcstack = NULL;

	} else {

		tmp = funcstack;
		funcstack = funcstack->next;
	}
	return tmp->sym;
}


symbol * top_funcstack() {
	return funcstack->sym;
}





list_node * new_returnlist(unsigned label) {

	list_node * new = (list_node *)malloc (sizeof (list_node));
	new->label = label;
	new->is_skip = 0;
	new->next = NULL;
	return new;
}

list_node * merge_ret (list_node * list_a, list_node * list_b) {
	
	list_node * tmp = NULL;

	if (list_a == NULL) {

		if (list_b == NULL) {
		
			return NULL;
		} else {
		
			return list_b;
		}
	}

	for (tmp = list_a; tmp->next != NULL; tmp = tmp->next);

	tmp->next = list_b;
	return list_a;
}


void generate_arithm(vmopcode op, quad* p) {

	instruction* t = (instruction*)malloc(sizeof(instruction));

	t->opcode = op;

	make_operand(p->arg1,&(t->arg1));

	make_operand(p->arg2,&(t->arg2));

	make_operand(p->result,&(t->result));

	p->taddress = next_instruction_label();

	emit_target(t);
}



void add_incomplete_jump (unsigned instrNo, unsigned iaddress) {

	incomplete_jump* t = (incomplete_jump*)malloc(sizeof(incomplete_jump));

	t->instrNo = instrNo;
	t->iaddress = iaddress;

	if (ij_head==NULL) {

		ij_head = t;
		ij_head->next = NULL;
	

	}else {

		t->next = ij_head;
		ij_head = t;
	}

}

void patch_incomplete_jumps(){

	incomplete_jump* tmp = ij_head;

	while (tmp!=NULL) {

		if (tmp->iaddress == currProcessedQuad) {

			instructions[tmp->instrNo].result.val = currInstruction;

		}else{

			instructions[tmp->instrNo].result.val = quads[tmp->iaddress].taddress;
		}
		tmp=tmp->next;
	}
}


void generate_relational(vmopcode op, quad* p) {
	
	instruction* t = (instruction*)malloc(sizeof(instruction));
	t->opcode = op;

	make_operand(p->arg1,&(t->arg1));
	
	make_operand(p->arg2,&(t->arg2));

	t->result.type = LABEL_A;

	if (p->label < curr_processed_quad()) {

		t->result.val = quads[p->label].taddress;

	}else{

		add_incomplete_jump(next_instruction_label(),p->label);
	}

	p->taddress = next_instruction_label();
	emit_target(t);
}



void generate_t (void) {

	unsigned i;

	for (i = 0; i<currQuad; ++i, ++currProcessedQuad){
		(*generators[quads[i].op])(quads+i);
	}
	patch_incomplete_jumps();
}




void generate_ASSIGN(quad* p){  generate_arithm(ASSIGN_V,p); }

void generate_ADD(quad* p){  generate_arithm(ADD_V,p); }

void generate_SUB(quad* p){  generate_arithm(SUB_V,p); }

void generate_MUL(quad* p){  generate_arithm(MUL_V,p); }

void generate_DIV(quad* p){  generate_arithm(DIV_V,p); }

void generate_MOD(quad* p){  generate_arithm(MOD_V,p); }


void generate_UMINUS(quad* p){
	instruction* t = (instruction*)malloc(sizeof(instruction));
	t->opcode = UMINUS_V;
	make_operand(p->arg1,&(t->arg1));
	make_numberoperand(&(t->arg2),-1);
	make_operand(p->result,&(t->result));
	p->taddress = next_instruction_label();
	emit_target(t);
}

void generate_JUMP (quad* p)				{  generate_relational(JUMP_V,p); }

void generate_IF_EQ (quad* p)				{  generate_relational(JEQ_V,p); }
void generate_IF_NOTEQ (quad* p)			{  generate_relational(JNE_V,p); }
void generate_IF_LESSEQ (quad* p)			{  generate_relational(JLE_V,p); }
void generate_IF_GREATEREQ (quad* p)		{  generate_relational(JGE_V,p); }
void generate_IF_LESS (quad* p)				{  generate_relational(JLT_V,p); }
void generate_IF_GREATER (quad* p)			{  generate_relational(JGT_V,p); }

void generate_CALL (quad* p){  

	p->taddress = next_instruction_label();
	instruction* t = (instruction*)malloc(sizeof(instruction));
	t->opcode = CALL_V;
	t->src_line = p->line;
	make_operand(p->arg1,&(t->arg1));
	make_operand(p->arg2,&(t->arg2));
	make_operand(p->result,&(t->result));
	emit_target(t);
}



void generate_PARAM (quad* p){  
	
	p->taddress = next_instruction_label();
	instruction* t = (instruction*)malloc(sizeof(instruction));
	t->opcode = PUSHARG_V;
	t->src_line = p->line;

	make_operand(p->arg1,&(t->arg1));
	make_operand(p->arg2,&(t->arg2));
	make_operand(p->result,&(t->result));
	emit_target(t);

}


void generate_FUNCSTART (quad* p) { 

	symbol * f = p->result->sym;
	f->taddress = next_instruction_label()+1;
	p->taddress = next_instruction_label()+1;

	push_funcstack(f);

	symbol * func = top_funcstack();

	instruction* t = (instruction*)malloc(sizeof(instruction));

	list_node * skip = new_returnlist(next_instruction_label());
	skip->is_skip = 1;

	func->return_list = merge_ret(func->return_list, skip);

	t->opcode = JUMP_V;
	t->result.type = LABEL_A;

	make_operand(NULL,&(t->arg1));
	make_operand(NULL,&(t->arg2));

	emit_target(t);


	instruction* t2 = (instruction*)malloc(sizeof(instruction));
	t2->opcode = ENTERFUNC_V;
	t2->src_line = p->line;

	make_operand(p->arg1,&(t2->arg1));
	make_operand(p->arg2,&(t2->arg2));
	make_operand(p->result,&(t2->result));
	emit_target(t2);

}


void backpatch_instr(list_node * return_list, unsigned label){

	list_node * tmp = NULL;

	for (tmp = return_list; tmp != NULL; tmp = tmp->next) {

		if(!tmp->is_skip){
			instructions[tmp->label].result.val = label;
			instructions[tmp->label].result.type = LABEL_A;

		}else{

			instructions[tmp->label].result.val = label+1;
			instructions[tmp->label].result.type = LABEL_A;
		}
	}
	
}

void generate_FUNCEND (quad* p)	{  

	symbol * f;
	f = pop_funcstack();
	backpatch_instr(f->return_list, next_instruction_label());

	p->taddress = next_instruction_label();

	instruction* t = (instruction*)malloc(sizeof(instruction));
	t->opcode = EXITFUNC_V;
	t->src_line = p->line;

	make_operand(p->arg1,&(t->arg1));
	make_operand(p->arg2,&(t->arg2));
	make_operand(p->result,&(t->result));
	emit_target(t);

}

void generate_TABLECREATE (quad* p) 	 	{generate_arithm(NEWTABLE_V,p);}
void generate_TABLEGETELEM (quad* p)		{generate_arithm(TABLEGETELEM_V,p);}
void generate_TABLESETELEM (quad* p)	 	{generate_arithm(TABLESETELEM_V,p);}


void generate_NOP (quad* p) {

	instruction* t = (instruction*)malloc(sizeof(instruction));
	t->opcode = NOP_V;
	t->src_line = p->line;
	make_operand(NULL,&(t->arg1));
	make_operand(NULL,&(t->arg2));
	make_operand(NULL,&(t->result));

	emit_target(t);
}



void generate_GETRETVAL (quad* p){ 

	p->taddress = next_instruction_label();
	instruction* t = (instruction*)malloc(sizeof(instruction));
	t->opcode = ASSIGN_V;
	t->src_line = p->line;
	make_operand(p->result,&(t->result));
	make_retvaloperand(&(t->arg1));

	make_operand(p->arg2,&(t->arg2));

	emit_target(t);
}


void reset_operand (vmarg *  vm ){
	memset(vm, 0, sizeof(vmarg) );
}



void generate_RET(quad * p){

	p->taddress = next_instruction_label();

	instruction* t = (instruction*)malloc(sizeof(instruction));

	t->opcode = ASSIGN_V;
	t->src_line = p->line;
	make_operand(p->result,&(t->arg1));

	make_retvaloperand(&(t->result));

	
	make_operand(p->arg2,&(t->arg2));

	emit_target(t);

	symbol * f = top_funcstack();

	list_node * this_return = new_returnlist(next_instruction_label());


	f->return_list = merge_ret(f->return_list, this_return);


	t->opcode = JUMP_V;
	reset_operand(&(t->arg1));
	reset_operand(&(t->arg2));

	t->result.type = LABEL_A;

	make_operand(NULL,&(t->arg1));
	make_operand(NULL,&(t->arg2));
	
	emit_target(t);

}





void write_binary(){

	FILE *fd;
	int i;
	int length;

	fd = fopen("tcode.abc","wb");

	fwrite(&magic_number,sizeof(unsigned),1,fd);


	
	fwrite(&curr_string_consts,sizeof(unsigned),1,fd);


	for(i=0;i<curr_string_consts;++i){
	
		length = strlen(string_consts[i]);
		fwrite(&length,sizeof(unsigned),1,fd);
		fwrite(string_consts[i],sizeof(char),length,fd);
	}


	fwrite(&curr_num_consts,sizeof(unsigned),1,fd);


	for(i=0;i<curr_num_consts;++i){
		fwrite(&num_consts[i],sizeof(double),1,fd);
	}


	fwrite(&curr_user_funcs,sizeof(unsigned),1,fd);

	
	for(i=0;i<curr_user_funcs;++i){
		
		fwrite(&user_funcs[i].address,sizeof(unsigned),1,fd);

		fwrite(&user_funcs[i].localSize,sizeof(unsigned),1,fd);

		length = strlen(user_funcs[i].id);

		fwrite(&length,sizeof(unsigned),1,fd);

		fwrite(user_funcs[i].id,sizeof(char),length,fd);

	}

	fwrite(&curr_named_libfuncs,sizeof(unsigned),1,fd);


	for(i=0;i<curr_named_libfuncs;++i){

		length = strlen(named_libfuncs[i]);
		fwrite(&length,sizeof(unsigned),1,fd);
		fwrite(named_libfuncs[i],sizeof(char),length,fd);

	}

	fwrite(&currInstruction,sizeof(unsigned),sizeof(unsigned),fd);

	for (i = 0; i < currInstruction; i++) {

		fwrite(&instructions[i],sizeof(instruction),1,fd);

	}
	fwrite(&programVarOffset,sizeof(unsigned),1,fd);


	fclose(fd);
}

#include <math.h>
#include <float.h>

void expand_userfuncs (void) {

	assert (curr_user_funcs == total_user_funcs);

	userfunc* p = (userfunc*) malloc(NEW_SIZE);

	if (user_funcs != NULL) {
		memcpy(p, user_funcs, CURR_SIZE);
		free(user_funcs);
	}
	user_funcs = p;
	total_user_funcs += EXPAND_SIZE;
}


unsigned userfuncs_new(unsigned iadress , unsigned locals, char * name ){

	int i;
	if(curr_user_funcs == total_user_funcs) expand_userfuncs();

	for(i = 0 ; i< curr_user_funcs ; i++){
		if(!strcmp(name,user_funcs[i].id) && user_funcs[i].address == iadress && user_funcs[i].localSize == locals){
			 return i;
		}
	}

	userfunc * new     = user_funcs + curr_user_funcs++;
	new->address       = iadress;
	new->localSize     = locals;
	new->id     = name;
	return curr_user_funcs-1;
}

void print_userfuncs(void){

	int i ;
	printf("\n*USER FUNCTIONS TABLE*\n\n");
	for (i =0;i< curr_user_funcs;i++ ){
		printf("%3d: "BLUE"iAdress"RESET": %3u ",i,user_funcs[i].address );
		printf(BLUE"Locals"RESET": %3u ",user_funcs[i].localSize );
		printf(BLUE"Name"RESET": %5s\n\n",user_funcs[i].id );
	}
}


void expand_libfuncs(void) {
	
	assert (curr_named_libfuncs == total_named_libfuncs);

	char ** p = (char**) malloc(NEW_SIZE);

	if (named_libfuncs != NULL) {
		memcpy(p,named_libfuncs, CURR_SIZE);
		free(named_libfuncs);
	}
	named_libfuncs = p;
	total_named_libfuncs += EXPAND_SIZE;
}


unsigned libfuncs_newused(char * name ){

	if(curr_named_libfuncs == total_named_libfuncs) expand_libfuncs();

	int i ;

	for(i = 0 ; i< curr_named_libfuncs ; i++){
		if(!strcmp(name,named_libfuncs[i])){
			 return i;
		}
	}

	char ** new = named_libfuncs + curr_named_libfuncs++;
	*new=name;

	return curr_named_libfuncs-1;
}

void print_libfuncs(void){

	int i ;
	printf("\n*LIBRARY FUNCTIONS TABLE*\n\n");
	for (i =0;i< curr_named_libfuncs;i++ ){
		printf("%3d: "BLUE"Name"RESET": %s \n",i,named_libfuncs[i]);
	}
	printf("\n");
}




void expand_string(void) {
	
	assert (curr_string_consts == total_string_consts);

	char ** p = (char**) malloc(NEW_SIZE);

	if (string_consts != NULL) {
		memcpy(p,string_consts, CURR_SIZE);
		free(string_consts);
	}
	string_consts = p;
	total_string_consts += EXPAND_SIZE;
}




unsigned consts_newstring (char* s){
	
	int i;

	if(curr_string_consts == total_string_consts) expand_string();


	for(i = 0 ; i< curr_string_consts ; i++){
		if(!strcmp(s,string_consts[i])){
			 return i;
		}
	}

	char ** new = string_consts + curr_string_consts++;

	*new=s;
	return curr_string_consts-1;
	

}

void print_strings(void){

	int i;
	printf("*CONSTANT STRINGS TABLE*\n\n");
	for(i=0;i<curr_string_consts;i++){
		printf("%3d: "BLUE"String"RESET": %3s \n",i,string_consts[i]);
	}
	printf("\n");
}


int	almost_eq(double a, double b) {

    return (fabs(a - b) <= (DBL_EPSILON * fabs(a + b)));
}


void expand_num(void){
	
	assert (curr_num_consts == total_num_consts);

	double* p = (double*) malloc(NEW_SIZE);

	if (num_consts != NULL) {
		memcpy(p,num_consts, CURR_SIZE);
		free(num_consts);
	}
	num_consts = p;
	total_num_consts += EXPAND_SIZE;
}


double consts_newnumber(double n){

	int i;
	if(curr_num_consts == total_num_consts) expand_num();

	for(i = 0 ; i < curr_num_consts ; i++){

		if (almost_eq(n,num_consts[i])){
			 return i;
		}
	}
	
	double * new = num_consts + curr_num_consts++;

	*new=n;
	
	return curr_num_consts-1;
}


void print_num(void){
	int i;
	printf("*CONSTANT NUMBERS TABLE*\n\n");
	for(i=0;i<curr_num_consts;i++){
		printf("%3d: "BLUE"Number"RESET": %.5f \n",i,num_consts[i]);
	}
	printf("\n");
}

void print_arrays(){

	print_userfuncs();

	print_libfuncs();

	print_strings();

	print_num();
}


#define WHITE "\x1B[37m"

char * op_to_text(iopcode op) {

	switch (op) {

	case ASSIGN     : return "ASSIGN";;
	case ADD      : return "ADD";
	case SUB      : return "SUB";
	case MUL      : return "MUL";
	case DIVISION   : return "DIVISION";
	case MOD      : return "MOD";
	case UMINUS     : return "UMINUS";
	case IF_EQ      : return "IF_EQ";
	case IF_NOTEQ   : return "IF_NOTEQ";
	case IF_LESSEQ    : return "IF_LESSEQ";
	case IF_GREATEREQ : return "IF_GREATEREQ";
	case IF_LESS    : return "IF_LESS";
	case IF_GREATER   : return "IF_GREATER";
	case JUMP     : return "JUMP";
	case CALL     : return "CALL";
	case PARAM      : return "PARAM";
	case RET      : return "RET";
	case GETRETVAL    : return "GETRETVAL";
	case FUNCSTART    : return "FUNCSTART";
	case FUNCEND    : return "FUNCEND";
	case TABLECREATE  : return "TABLECREATE";
	case TABLEGETELEM : return "TABLEGETELEM";
	case TABLESETELEM : return "TABLESETELEM";
	default: return NULL;
	}
}

char* expr_to_text (expr_t expr) {
	switch (expr) {
	case VAR_E:         return "VAR_E";
	case TABLEITEM_E:   return "TABLEITEM_E";
	case PROGRAMFUNC_E: return "PROGRAMFUNC_E";
	case LIBRARYFUNC_E: return "LIBRARYFUNC_E";
	case ARITHEXPR_E:   return "ARITHEXPR_E";
	case BOOLEXPR_E:    return "BOOLEXPR_E";
	case ASSIGNEXPR_E:  return "ASSIGNEXPR_E";
	case NEWTABLE_E:    return "NEWTABLE_E";
	case CONSTNUM_E:    return "CONSTNUM_E";
	case CONSTBOOL_E:   return "CONSTBOOL_E";
	case CONSTSTRING_E: return "CONSTSTRING_E";
	case NIL_E:         return "NIL_E";
	}
}

char* scopespace_to_text (scopespace_t space) {
	switch (space) {
	case PROGRAMVAR:    return "PROGRAMVAR";
	case FUNCTIONLOCAL: return "FUNCTIONLOCAL";
	case FORMALARG:     return "FORMALARG";
	}
}

char* symbol_to_text (symbol_t space) {
	switch (space) {
	case VAR_S:         return "VAR_S";
	case PROGRAMFUNC_S: return "PROGRAMFUNC_S";
	case LIBRARYFUNC_S: return "LIBRARYFUNC_S";
	}
}

void print_symbol_table() {
	symbol * tmp;
	int i;
	printf("\n*SYMBOL TABLE*\n\n");
	for (i = 0; i < SIZE; i++) {
		for (tmp = symbol_table[i]; tmp != NULL; tmp = tmp->next) {
			if (get_name(tmp) != NULL) {
				if (tmp->type == LIBRARYFUNC_S)continue;

				if (tmp->type == PROGRAMFUNC_S) {
					printf("Name: %s Line: %u Scope: %u Type: %s Locals: %u\n", get_name(tmp), get_line(tmp), get_scope(tmp), symbol_to_text(tmp->type), tmp->total_locals);

				} else {
					printf("Name: %s Line: %u Scope: %u Type: %s\n", get_name(tmp), get_line(tmp), get_scope(tmp), symbol_to_text(tmp->type));
				}
			}
		}
	}
}

void print_scope_link() {

	symbol * tmp = NULL;
	symbol * tmp_dummy = NULL;
	int i = 0;
	printf("\n*SCOPE LINK*\n\n");
	for (tmp_dummy = scope_link_list ; tmp_dummy != NULL; tmp_dummy = tmp_dummy->next) {

		for (tmp = tmp_dummy->scope_link_next ; tmp != NULL; tmp = tmp->scope_link_next, i++) {

			if (tmp->type == LIBRARYFUNC_S)continue;

			if (tmp->type == PROGRAMFUNC_S) {
				printf("Name: %s Line: %u Scope: %u Type: %s Locals: %u\n", get_name(tmp), get_line(tmp), get_scope(tmp), symbol_to_text(tmp->type), tmp->total_locals);

			} else {
				printf("Name: %s Line: %u Scope: %u Type: %s Space: %s\n", get_name(tmp), get_line(tmp), get_scope(tmp), symbol_to_text(tmp->type), scopespace_to_text(tmp->space));
			}
		}

	}
	printf("\n");
}

void print_list(char * type, list_node * head) {
	list_node * tmp = NULL;
	printf("%s\n",type);
	for (tmp = head; tmp != NULL; tmp = tmp->next) {
		printf("%d, ", tmp->label );
	}
	printf("\n");
}

void print_stack(stack_offset_t * top) {

	stack_offset_t * tmp = NULL;
	printf("Stack:\n");
	for (tmp = top; tmp != NULL; tmp = tmp->next) {
		printf("Value : %d\n", tmp->offset );
	}
}

void print_loop_stack() {

	stack_bc_t * tmp = NULL;
	printf("Stack LIST:\n");
	for (tmp = loop_bc_stack; tmp != NULL; tmp = tmp->next) {
		printf("Value : %d\n", tmp->loop_num );
	}
}

void print_param ( ) {

	stack_expr_t * tmp = NULL;
	printf("Param Stack:\n");
	for (tmp = param_stack; tmp != NULL; tmp = tmp->next) {
		printf("Name : %s\n", tmp->expr->sym->name );
	}
}



void print_arg(expr * arg, FILE * fd) {

	if (arg != NULL) {

		if (arg->type == CONSTNUM_E) {

			printf("%11.2f ", arg->numConst );
			fprintf(fd,"%11.2f ", arg->numConst );

		} else if (arg->type == CONSTBOOL_E) {
			if (arg->boolConst) {
				printf("%11s ", "TRUE");
				fprintf(fd,"%11s ", "TRUE");
			} else {
				printf("%11s ", "FALSE");
				fprintf(fd,"%11s ", "FALSE");
			}
		} else if (arg->type == CONSTSTRING_E) {
			
			printf("%11s ", arg->strConst );
			fprintf(fd,"%11s ", arg->strConst );

		} else if (arg->type == NIL_E) {
			printf("%11s ", "NIL");
			fprintf(fd,"%11s ", "NIL");
		} else if (arg->sym != NULL) {
			printf("%11s ", arg->sym->name );
			fprintf(fd,"%11s ", arg->sym->name );
		}
	}
}




void print_arg_example(expr * arg) {

	if (arg != NULL) {

		if (arg->type == CONSTNUM_E) {

			printf("%.2f  ", arg->numConst );

		} else if (arg->type == CONSTBOOL_E) {
			if (arg->boolConst) {
				printf("TRUE  ");
			} else {
				printf("FALSE  ");
			}
		} else if (arg->type == CONSTSTRING_E) {
			printf("%s  ", arg->strConst );

		} else if (arg->type == NIL_E) {
			printf("NIL  ");
		} else if (arg->sym != NULL) {
			printf("%s  ", arg->sym->name );
		}
	}
}


void print_quads(void) {

	int i;
	FILE * fd = fopen("quads.txt","w");

	printf("\n*QUADS TABLE*\n\n");
	fprintf(fd,"\n*QUADS TABLE*\n\n");

	for (i = 0; i < currQuad; i++) {

		printf("%3d: ", i);
		fprintf(fd,"%3d: ", i);

		printf(WHITE"OP"RESET": %13s ", op_to_text(quads[i].op) );
		fprintf(fd,"OP: %13s ", op_to_text(quads[i].op));

		printf(WHITE"Arg1"RESET": ");
		fprintf(fd,"Arg1: ");

		if (quads[i].arg1 == NULL){
			printf("%12s","- ");
			fprintf(fd,"%12s","- ");

		}else{
			print_arg(quads[i].arg1,fd);
		}

		printf(WHITE"Arg2"RESET": ");
		fprintf(fd, "Arg2: ");

		if (quads[i].arg2 == NULL){
			printf("%12s","- ");
			fprintf(fd,"%12s","- ");

		}else{
			print_arg(quads[i].arg2,fd);
		}

		printf(WHITE"Result"RESET": ");
		fprintf(fd, "Result: ");

		if (quads[i].result == NULL){
			printf("%12s","- ");
			fprintf(fd,"%12s","- ");

		}else{
			print_arg(quads[i].result,fd);
		}

		printf(WHITE"Label"RESET": %u", quads[i].label);
		fprintf(fd, "Label: %u", quads[i].label);

		printf("\n\n");
		fprintf(fd, "\n\n");
	}
	printf("%3d:\n\n", i);
	fprintf(fd,"%3d:\n\n", i);
	fclose(fd);
}





void print_example(void) {

	int i;

	for (i = 0; i < currQuad; i++) {

		printf("%d: ", i + 1 );

		printf("%s  ", op_to_text(quads[i].op));

		print_arg_example(quads[i].arg1);

		print_arg_example(quads[i].arg2);

		print_arg_example(quads[i].result);

		printf(" %u", quads[i].label + 1);

		printf("\n\n");
	}
	printf("%d:\n\n", i + 1);
}


void free_compiler(){

	symbol * tmp = NULL;
	symbol * prev_dummy = NULL;
	symbol * tmp2 = NULL;
	symbol * prev = NULL;
	list_node * tmp_node = NULL;
	list_node * prev_node = NULL;
	quad * tmp_quad = NULL;
	int i;
	expr * tmp_expr = NULL;

	for(tmp = scope_link_list; tmp != NULL;){

		for(tmp2 = tmp->scope_link_next; tmp2 != NULL;){

			prev = tmp2;
			tmp2 = tmp2->scope_link_next;

			for(tmp_node = tmp2->return_list; tmp_node != NULL;){

				prev_node = tmp_node;
				tmp_node = tmp_node->next;
				free(prev_node);
			}

			free(prev);
		}
		prev_dummy = tmp;
		tmp = tmp->next;
		free(prev_dummy);
	}

	for(i=0; i < currQuad; ++i){

		tmp_expr = quads[i].arg1;

		for(tmp_node = tmp_expr->true_list; tmp_node != NULL;){

			prev_node = tmp_node;
			tmp_node = tmp_node->next;
			free(prev_node);
		}

		for(tmp_node = tmp_expr->false_list; tmp_node != NULL;){

			prev_node = tmp_node;
			tmp_node = tmp_node->next;
			free(prev_node);
		}

		tmp_expr = quads[i].arg2;

		for(tmp_node = tmp_expr->true_list; tmp_node != NULL;){

			prev_node = tmp_node;
			tmp_node = tmp_node->next;
			free(prev_node);
		}

		for(tmp_node = tmp_expr->false_list; tmp_node != NULL;){

			prev_node = tmp_node;
			tmp_node = tmp_node->next;
			free(prev_node);
		}

		tmp_expr = quads[i].result;

		for(tmp_node = tmp_expr->true_list; tmp_node != NULL;){

			prev_node = tmp_node;
			tmp_node = tmp_node->next;
			free(prev_node);
		}

		for(tmp_node = tmp_expr->false_list; tmp_node != NULL;){

			prev_node = tmp_node;
			tmp_node = tmp_node->next;
			free(prev_node);
		}


	}
}