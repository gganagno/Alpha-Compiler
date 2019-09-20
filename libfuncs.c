extern void avm_memcell_clear(avm_memcell * m);

extern avm_table * avm_tablenew();

extern void avm_tablesetelem ( avm_table * table , avm_memcell * key , avm_memcell * value);

extern unsigned avm_get_envvalue(unsigned i);

extern unsigned avm_total_actuals(void);

extern avm_memcell * avm_get_actual(unsigned i);

library_func_t avm_get_libraryfunc(char * name){

	int i;

	for(i = 0; i < libfuncs_total;i++){

		if(!strcmp(libfunc_names[i],name)){
			
			return libFuncs[i];
		}
	}
	return NULL;
}

void avm_call_libfunc(char * id){

	library_func_t f = avm_get_libraryfunc(id);

	if(!f){
		avm_error("unsupported libfunc called !\n");
	}else{
	
		topsp = top;
		total_actuals = 0;
		(*f)();
	
		if(!execution_finished){
			execute_FUNCEXIT((instruction*) 0);
		}
	}
} 



void libfunc_totalarguments(){

	unsigned p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	avm_memcell_clear(&retval);

	if(!p_topsp){
		avm_error(" 'totalarguments' called outside a function\n");
	}else{

		retval.type = NUMBER_M;
		retval.data.numVal = avm_get_envvalue(p_topsp + AVM_NUMACTUALS_OFFSET);
	}
}


void libfunc_argument(void){


	unsigned p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);


	avm_memcell_clear(&retval);

	if(!p_topsp){

		avm_error("'argument' called outside a function!");
		retval.type = NIL_M;

	}else{

		unsigned x = 0U;

		unsigned n = avm_total_actuals();

		avm_memcell * tmp =  avm_get_actual(0);

		if (tmp->type!=NUMBER_M){

			avm_warning("Wrong input for argument");

		}else {
			
			x = tmp->data.numVal;
		}
		if (n!=1) avm_error("Wrong input for total actuals.");
		
		avm_assign(&retval,&stack[p_topsp + AVM_STACKENV_SIZE + x + 1]);
	}
}



void libfunc_print(){

	unsigned n = avm_total_actuals();
	int i,j;
	for(i=0; i < n ; ++i) {
		
		avm_memcell * tmp = avm_get_actual(i);
		char * s = strdup(avm_tostring(tmp));
		printf("%s",s );
		free(s);
	}
	retval.type = NIL_M;
}


void libfunc_typeof(void){

	unsigned n = avm_total_actuals();

	if(n!=1){
		avm_error("one argument expected in 'typeof'\n");

	}else{

		avm_memcell_clear(&retval);
		retval.type = STRING_M;
		retval.data.strVal = strdup(typeStrings[avm_get_actual(0)->type]);
	}
}




#include <math.h>

void libfunc_sqrt(void){


	unsigned n = avm_total_actuals();

	avm_memcell_clear(&retval);

	if(n!=1){
		avm_error("one argument expected in 'sqrt'\n");

	}else{

		if(avm_get_actual(0)->type == NUMBER_M){
			if(avm_get_actual(0)->data.numVal >= 0){
				retval.type = NUMBER_M;
				retval.data.numVal = sqrt(avm_get_actual(0)->data.numVal);

			}else{
				retval.type = NIL_M;
			}
		}
	}
}

#define PI 3.14159265

void libfunc_cos(void){

	double val = PI / 180.0;

	unsigned n = avm_total_actuals();

	avm_memcell_clear(&retval);

	if(n!=1){
		avm_error("one argument expected in 'cos'\n");

	}else{

		if(avm_get_actual(0)->type == NUMBER_M){
			retval.type = NUMBER_M;
			retval.data.numVal = cos(avm_get_actual(0)->data.numVal * val);
		}else{
			retval.type = NIL_M;
		}
	}
}

void libfunc_sin(void){

	double val = PI / 180.0;

	unsigned n = avm_total_actuals();

	avm_memcell_clear(&retval);

	if(n!=1){
		avm_error("one argument expected in 'sin'\n");

	}else{

		if(avm_get_actual(0)->type == NUMBER_M){
			retval.type = NUMBER_M;
			retval.data.numVal = sin(avm_get_actual(0)->data.numVal * val );
		}else{
			retval.type = NIL_M;
		}
	}
}

void libfunc_strtonum(void){

	unsigned n = avm_total_actuals();

	avm_memcell_clear(&retval);

	if(n!=1){
		avm_error("one argument expected in 'strtonum'\n");

	}else{

		if(avm_get_actual(0)->type == STRING_M){

			char * s = strdup(avm_get_actual(0)->data.strVal);

			unsigned len = strlen(s);
			
			char * c = (char*)calloc(len-1, sizeof(char));
			memcpy(c,s+1,len-2);

			char * end = NULL;

			double num = strtod(c,&end);
			
			if (strlen(end) != 0) {

    			retval.type = NIL_M;
			}else{

				retval.type = NUMBER_M;
				retval.data.numVal = num;
			}
	
			free(c);
			free(s);

		}else{
			retval.type = NIL_M;
		}
	}
}

void libfunc_input(void){

	unsigned n = avm_total_actuals();

	if(n){
		avm_error("no arguments expected in 'input'\n");

	}else{

		char * input = (char * ) malloc (128);
		scanf("%s",input);

		char * end = 0;

		double num = strtod(input,&end);
	
		avm_memcell_clear(&retval);

		if (input == end) {

			if(!strcmp(input,"true") ){

				retval.type = BOOL_M;
				retval.data.boolVal = 1;
				printf("%s  // boolean\n",input );
				

			}else if (!strcmp(input,"false")){

				retval.type = BOOL_M;
				retval.data.boolVal = 0;
				printf("%s  // boolean\n",input );
			

			}else{
				retval.type = STRING_M;
				retval.data.strVal = input;
				printf("%s  // string \n",input );

			}


		}else{
			retval.type = NUMBER_M;
			retval.data.numVal = num;
			printf("%f  // number \n",num );
		}
	}

}


void libfunc_objecttotalmembers(void){

	unsigned n = avm_total_actuals();
	avm_memcell_clear(&retval);

	if(n!=1){
		avm_error("one argument expected in 'tablelength'\n");

	}else{

		if(avm_get_actual(0)->type == TABLE_M){
			retval.type = NUMBER_M;
			retval.data.numVal = avm_get_actual(0)->data.tableVal->total;
			
		}else{
			retval.type = NIL_M;
		}
	}
}

void libfunc_objectmemberkeys(void) { 

	
	unsigned n = avm_total_actuals();
	unsigned i;
	double ind = 0;

	avm_memcell_clear(&retval);

	if(n!=1){
		avm_error("one argument expected in 'tableindices'\n");

	}else{

		if(avm_get_actual(0)->type == TABLE_M){

			avm_table * new_table = avm_tablenew();
			avm_table_bucket * tmp = NULL;

			
			for(i=0;i<AVM_TABLE_HASHSIZE;++i){

				for(tmp = avm_get_actual(0)->data.tableVal->str_indexed[i]; tmp!= NULL; tmp= tmp->next ){

					avm_memcell * new_index = (avm_memcell *)malloc (sizeof(avm_memcell));
					avm_memcell * new_value = (avm_memcell *)malloc (sizeof(avm_memcell));

					new_value->type = STRING_M;
					new_value->data.strVal = strdup(tmp->key.data.strVal);	
			
					new_index->type = NUMBER_M;

					new_index->data.numVal = ind++;
			
					avm_tablesetelem(new_table,new_index,new_value);

				}
			}
			retval.type = TABLE_M;
			retval.data.tableVal = new_table;
		}else{

			retval.type = NIL_M;
			avm_warning(" argument of 'tableindices' must be table\n");
		}
	}
}



void libfunc_objectcopy (void){

	unsigned n = avm_total_actuals();
	unsigned i;

	avm_memcell_clear(&retval);

	if(n!=1){
		avm_error("one argument expected in 'tablecopy'\n");

	}else{

		if(avm_get_actual(0)->type == TABLE_M){

			avm_table * new_table = avm_tablenew();
			avm_table_bucket * tmp = NULL;

			
			for(i=0;i<AVM_TABLE_HASHSIZE;++i){

				for(tmp = avm_get_actual(0)->data.tableVal->num_indexed[i]; tmp!= NULL; tmp= tmp->next ){
					
					avm_memcell * new_index = (avm_memcell* )malloc (sizeof(avm_memcell));
					avm_memcell * new_value = (avm_memcell *)malloc (sizeof(avm_memcell));

					memcpy(new_index,&tmp->key,sizeof(avm_memcell));
					memcpy(new_value,&tmp->value,sizeof(avm_memcell));

					avm_tablesetelem(new_table,new_index,new_value);

				}

				for(tmp = avm_get_actual(0)->data.tableVal->str_indexed[i]; tmp!= NULL; tmp= tmp->next ){
		
					avm_memcell * new_index = (avm_memcell* )malloc (sizeof(avm_memcell));
					avm_memcell * new_value = (avm_memcell *)malloc (sizeof(avm_memcell));

					memcpy(new_index,&tmp->key,sizeof(avm_memcell));

					memcpy(new_value,&tmp->value,sizeof(avm_memcell));

					avm_tablesetelem(new_table,new_index,new_value);

				}

			}
			new_table->ref_counter = avm_get_actual(0)->data.tableVal->ref_counter;
			new_table->total = avm_get_actual(0)->data.tableVal->total;
			retval.type = TABLE_M;
			retval.data.tableVal = new_table;
		}else{

			retval.type = NIL_M;
			avm_warning(" argument of 'tablecopy' must be table\n");
		}
	}

} 
