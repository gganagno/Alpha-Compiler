%{

#include "compiler.h"
	
#include "syntax.c"
#include "intermediate.c"
#include "target.c"

%}


%union{	
		char * strVal;
		int intVal;
		double doubleVal;
		struct expr * exprVal;
		struct Stmt * stmtVal;
		struct symbol * symbolVal;
		struct List_node * nodeVal;
};


%start program

%type <stmtVal> stmts
%type <stmtVal> stmt
%type <stmtVal> loop_stmt
%type <stmtVal> return_stmt

%token <strVal> key_if
%token <strVal> key_else
%token <strVal> key_while
%token <strVal> key_for
%token <strVal> key_function
%token <strVal> key_return
%token <strVal> key_break
%token <strVal> key_continue
%token <strVal> key_and
%token <strVal> key_and_sym
%token <strVal> key_not
%token <strVal> key_not_sym
%token <strVal> key_or
%token <strVal> key_or_sym
%token <strVal> key_local
%token <strVal> key_true
%token <strVal> key_false
%token <strVal> key_nil

%token <strVal> assign_op
%token <strVal> add_op
%token <strVal> minus_op
%token <strVal> star_op
%token <strVal> slash_op
%token <strVal> mod_op
%token <strVal> equal_op
%token <strVal> nequal_op
%token <strVal> inc_op
%token <strVal> dec_op
%token <strVal> big_op
%token <strVal> less_op
%token <strVal> bigeq_op
%token <strVal> lesseq_op

%token <strVal> left_brace
%token <strVal> right_brace
%token <strVal> left_bracket
%token <strVal> right_bracket
%token <strVal> left_parenthesis
%token <strVal> right_parenthesis
%token <strVal> semicolon
%token <strVal> comma
%token <strVal> colon
%token <strVal> double_colon
%token <strVal> dot
%token <strVal> double_dot
%token <strVal> single_comm_slash

%token <intVal> int_num
%token <doubleVal> double_num
%token <strVal> id
%token <strVal> string


%type <exprVal> term 
%type <exprVal> assign_expr 
%type <exprVal> expr 
%type <exprVal> primary
%type <exprVal> lvalue 
%type <exprVal> tableitem 
%type <exprVal> tablemake
%type <exprVal> indexed
%type <exprVal> indexelem
%type <exprVal> const 
%type <exprVal> idlist
%type <exprVal> elist
%type <exprVal> normcall
%type <exprVal> methodcall 
%type <exprVal> callsuffix
%type <exprVal> call

%type <symbolVal> funcname 
%type <stmtVal> funcbody

%type <symbolVal> funcprefix 
%type <symbolVal> funcdef 

%type <exprVal> funcargs 


%type <stmtVal> block

%type <nodeVal> ifprefix
%type <stmtVal> if_stmt
%type <intVal> elseprefix
%type <intVal> whilestart
%type <nodeVal> whilecond
%type <stmtVal> while_stmt


%type <stmtVal> for_stmt
%type <exprVal> forprefix
%type <intVal> N
%type <intVal> M
%type <intVal> V


%token error_string
%token invalid
%token invalid_id

%right assign_op
%left key_or key_or_sym 
%left key_and key_and_sym 
%nonassoc equal_op nequal_op
%nonassoc big_op bigeq_op less_op lesseq_op
%left add_op minus_op
%left star_op slash_op mod_op
%nonassoc uminus_rule
%right inc_op dec_op key_not key_not_sym
%left dot double_dot
%left left_bracket right_bracket
%left left_parenthesis right_parenthesis 
%nonassoc else_dummy
%nonassoc key_else

%%

program 	: stmts 
			{	
				printf(GREEN"\nSyntax Analysis Complete \xE2\x9C\x93\n\n"RESET);
			};

stmts		: stmts stmt
			{
				reset_tmp();
			}

			| stmt 
			{
				reset_tmp();
				$$ = $1;
			}
			;	
		
stmt 		: expr semicolon 
			{

				if($1!=NULL){

					if($1->type == BOOLEXPR_E){

						$$ = newstmt_expr($1,BOOL_STMT);
						$$->expr->sym = new_tmp();
						backpatch($1->true_list,next_quad_label());

						emit(ASSIGN,newexpr_const_bool(1),NULL,$$->expr,currQuad,yylineno);

						backpatch($1->false_list,next_quad_label()+1);

						emit(JUMP,NULL,NULL,NULL,next_quad_label()+2,yylineno);
						emit(ASSIGN,newexpr_const_bool(0),NULL,$$->expr,currQuad,yylineno);
					}else{
						$$ = newstmt_expr($1,ONLY_STMT);
					}

				}else{

					$$ = NULL;
				}
			}

			| key_break semicolon
			{	
			

				if(loop_counter != 0){
					
					list_node * tmp = new_list_node(currQuad);
					loop_bc_stack->break_list = merge (loop_bc_stack->break_list,tmp);
					$$ = new_break_stmt(tmp);
					emit(JUMP,NULL,NULL,NULL,-1,yylineno);

				}else{
					printf(RED"\nerror" RESET" %d: Use of 'break' while not in a loop in line %u\n\n",error_cnt++,yylineno);
					$$ = NULL;
				}
			}

			|  key_continue semicolon
			{	
				if(loop_counter != 0) {

					list_node * tmp = new_list_node(currQuad);
					loop_bc_stack->continue_list = merge (loop_bc_stack->continue_list,tmp);
					$$ = new_continue_stmt(tmp);
					emit(JUMP,NULL,NULL,NULL,-1,yylineno);

				}else{
					printf(RED"\nerror" RESET" %d: Use of 'continue' while not in a loop in line %u \n\n",error_cnt++,yylineno);
					$$ = NULL;
				}
			}

			| semicolon
			{
				$$ = new_stmt(ONLY_STMT);
			}


			| return_stmt
			{ 
				$$ = $1;
			}


			| for_stmt
			{
				$$ = $1;
			}


			| if_stmt 		
			{ 
				$$ = $1;
			}


			| while_stmt
			{
				$$ = $1;
			}	


			| funcdef
			{
				$$ = newstmt_symbol($1,FUNCDEF_STMT);
			}


			| block
			{
				$$ = $1;

			}
	 			
			; 

	
assign_expr	: lvalue assign_op expr 
			{		
			
				if($1 != NULL){
					if($1->type==TABLEITEM_E){
						
						emit(TABLESETELEM,$1,$1->index,$3,currQuad,yylineno);
						$$ = emit_if_tableitem($1);
						$$->type = ASSIGNEXPR_E;

					}else if($1->type == PROGRAMFUNC_E || $1->type == LIBRARYFUNC_E){
						
						printf(RED"\nerror" RESET" %d: You cannot use function %s as an lvalue in line %u and scope %u\n\n",error_cnt++,$1->sym->name,yylineno,scope);
						$$ = NULL;
						

					}else if($3->type==BOOLEXPR_E){

						expr * new = newexpr(ASSIGNEXPR_E);
						new->sym = new_tmp();

						backpatch($3->true_list,next_quad_label()); //to true_list na kanei jump sto epomeno quad pou einai to assign true 

						emit(ASSIGN,newexpr_const_bool(1),NULL,new,currQuad,yylineno);
						emit(JUMP,NULL,NULL,NULL,next_quad_label()+2,yylineno);

						backpatch($3->false_list,next_quad_label()); // to false edw sto assign false

						emit(ASSIGN,newexpr_const_bool(0),NULL,new,currQuad,yylineno);

						
						emit(ASSIGN,new,NULL,$1,currQuad,yylineno);

						$$ = newexpr(ASSIGNEXPR_E);
						$$->sym = new_tmp();

						emit(ASSIGN,new,NULL,$$,currQuad,yylineno);


					}else{
						
						emit(ASSIGN,$3,NULL,$1,currQuad,yylineno);
						$$ = newexpr(ASSIGNEXPR_E);
						$$->sym = new_tmp();
						emit(ASSIGN,$1,NULL,$$,currQuad,yylineno);					
					}
				}else{

					$$ = NULL;
				}
			}


expr		: assign_expr 		{$$ = $1;}	

			| expr add_op expr
			{
				if($1->type != ARITHEXPR_E || $1->type != CONSTNUM_E ){

				}
				$$ = newexpr(ARITHEXPR_E);

				if(is_tmp_expr($1)){

					$$->sym = $1->sym;
				}else if(is_tmp_expr($3)){

					$$->sym = $3->sym;
				}else{
					$$->sym = new_tmp();
		
				}
	
				emit(ADD,$1,$3,$$,currQuad,yylineno);
				
							
			}			
			
			| expr minus_op expr
			{

				$$ = newexpr(ARITHEXPR_E);

				if(is_tmp_expr($1)){

					$$->sym = $1->sym;

				}else if(is_tmp_expr($3)){

					$$->sym = $3->sym;
				}else{
					$$->sym = new_tmp();
		
				}
				emit(SUB,$1,$3,$$,currQuad,yylineno);
			} 	


			| expr star_op expr
			{

				$$ = newexpr(ARITHEXPR_E);
				if(is_tmp_expr($1)){

					$$->sym = $1->sym;
				}else if(is_tmp_expr($3)){

					$$->sym = $3->sym;
				}else{
					$$->sym = new_tmp();
		
				}
				emit(MUL,$1,$3,$$,currQuad,yylineno);
			} 	


			| expr slash_op expr
			{

				$$ = newexpr(ARITHEXPR_E);
				if(is_tmp_expr($1)){

					$$->sym = $1->sym;
				}else if(is_tmp_expr($3)){

					$$->sym = $3->sym;
				}else{
					$$->sym = new_tmp();
		
				}
				emit(DIVISION,$1,$3,$$,currQuad,yylineno);
			} 


			| expr mod_op expr
			{
								
				$$ = newexpr(ARITHEXPR_E);
				if(is_tmp_expr($1)){


					$$->sym = $1->sym;
				}else if(is_tmp_expr($3)){

					$$->sym = $3->sym;
				}else{
					$$->sym = new_tmp();
		
				}
				emit(MOD,$1,$3,$$,currQuad,yylineno);
			}



			| expr less_op expr	{

				$$ = newexpr(BOOLEXPR_E);
				
				$$->true_list = new_list_node(next_quad_label());
				$$->false_list = new_list_node(next_quad_label() +1);
				

				emit(IF_LESS,$1,$3,NULL,-1,yylineno);
				emit(JUMP,NULL,NULL,NULL,-1,yylineno);
			}



			| expr lesseq_op expr{

				$$ = newexpr(BOOLEXPR_E);

				$$->true_list = new_list_node(next_quad_label());
				$$->false_list = new_list_node(next_quad_label() +1);

				emit(IF_LESSEQ,$1,$3,NULL,-1,yylineno);
				emit(JUMP,NULL,NULL,NULL,-1,yylineno);
			}	



			| expr big_op expr {

				$$ = newexpr(BOOLEXPR_E);

				$$->true_list = new_list_node(next_quad_label());
				$$->false_list = new_list_node(next_quad_label() +1);
				
				emit(IF_GREATER,$1,$3,NULL,-1,yylineno);
				emit(JUMP,NULL,NULL,NULL,-1,yylineno);
			}		



			| expr bigeq_op expr {

				$$ = newexpr(BOOLEXPR_E);

				$$->true_list = new_list_node(next_quad_label());
				$$->false_list = new_list_node(next_quad_label() +1);
				
				emit(IF_GREATEREQ,$1,$3,NULL,-1,yylineno);
				emit(JUMP,NULL,NULL,NULL,-1,yylineno);
			}	



			| expr equal_op expr{

				$$ = newexpr(BOOLEXPR_E);
				$$->sym = new_tmp();

				$$->true_list = new_list_node(next_quad_label());
				$$->false_list = new_list_node(next_quad_label()+1);
				

				emit(IF_EQ,$1,$3,NULL,currQuad,yylineno);
				emit(JUMP,NULL,NULL,NULL,currQuad,yylineno);

				
			} 		



			| expr nequal_op expr
			{

				$$ = newexpr(BOOLEXPR_E);
				$$->sym = new_tmp();

				$$->true_list = new_list_node(next_quad_label());
				$$->false_list = new_list_node(next_quad_label()+1);
				
				emit(IF_NOTEQ,$1,$3,NULL,currQuad,yylineno);
				emit(JUMP,NULL,NULL,NULL,currQuad,yylineno);

				
			}



			| expr key_and 	// "and"

			{ 	
				if($1->type != BOOLEXPR_E){
						
					$1->true_list = new_list_node(next_quad_label());
					$1->false_list = new_list_node(next_quad_label() +1);

					emit(IF_EQ, newexpr_const_bool(1),$1,NULL,-1,yylineno);
					emit(JUMP, NULL, NULL, NULL, -1, yylineno);
				}

			} V expr
			{ 

				$$ = newexpr(BOOLEXPR_E);

				

				if($5->type != BOOLEXPR_E){

					$5->true_list = new_list_node(next_quad_label());
					$5->false_list = new_list_node(next_quad_label() +1);

					emit(IF_EQ, newexpr_const_bool(1),$5,NULL,-1, yylineno);
					emit(JUMP, NULL, NULL, NULL, -1, yylineno);
				}

				
				backpatch($1->true_list,$4);

				$$->true_list = $5->true_list;

				$$->false_list = merge($1->false_list,$5->false_list);
			
			}



			| expr key_and_sym // "&&"

			{ 	
				if($1->type != BOOLEXPR_E){
						
					$1->true_list = new_list_node(next_quad_label());
					$1->false_list = new_list_node(next_quad_label() +1);

					emit(IF_EQ, newexpr_const_bool(1),$1,NULL,-1,yylineno);
					emit(JUMP, NULL, NULL, NULL, -1, yylineno);
				}

			} V expr
			{

				$$ = newexpr(BOOLEXPR_E);

				

				if($5->type != BOOLEXPR_E){

					$5->true_list = new_list_node(next_quad_label());
					$5->false_list = new_list_node(next_quad_label() +1);

					emit(IF_EQ, newexpr_const_bool(1),$5,NULL,-1, yylineno);
					emit(JUMP, NULL, NULL, NULL, -1, yylineno);
				}

				
				backpatch($1->true_list,$4);

				$$->true_list = $5->true_list;

				$$->false_list = merge($1->false_list,$5->false_list);
			
			}



			| expr key_or // "or"
			{

				if($1->type != BOOLEXPR_E){

					$1->true_list = new_list_node(next_quad_label());
					$1->false_list = new_list_node(next_quad_label() +1);

					emit(IF_EQ, newexpr_const_bool(1),$1,NULL,-1,yylineno);
					emit(JUMP, NULL, NULL, NULL, -1, yylineno);
				}

			} V expr
			{

				$$ = newexpr(BOOLEXPR_E);

				
				if($5->type != BOOLEXPR_E){

					$5->true_list = new_list_node(next_quad_label());
					$5->false_list = new_list_node(next_quad_label() +1);

					emit(IF_EQ, newexpr_const_bool(1),$5,NULL,-1, yylineno);
					emit(JUMP, NULL, NULL, NULL, -1, yylineno);
				}


				backpatch($1->false_list,$4);

				$$->true_list = merge($1->true_list,$5->true_list);

				$$->false_list = $5->false_list;

			}	


			| expr key_or_sym // "||"
			{

				if($1->type != BOOLEXPR_E){

					$1->true_list = new_list_node(next_quad_label());
					$1->false_list = new_list_node(next_quad_label() +1);

					emit(IF_EQ, newexpr_const_bool(1),$1,NULL,-1,yylineno);
					emit(JUMP, NULL, NULL, NULL, -1, yylineno);
				}

			} V expr
			{

				$$ = newexpr(BOOLEXPR_E);

				
				if($5->type != BOOLEXPR_E){

					$5->true_list = new_list_node(next_quad_label());
					$5->false_list = new_list_node(next_quad_label() +1);

					emit(IF_EQ, newexpr_const_bool(1),$5,NULL,-1, yylineno);
					emit(JUMP, NULL, NULL, NULL, -1, yylineno);
				}


				backpatch($1->false_list,$4);

				$$->true_list = merge($1->true_list,$5->true_list);

				$$->false_list = $5->false_list;

			}	


			
			| key_not expr 	// "not"
			{

				if($2->type != BOOLEXPR_E){

					$2->true_list = new_list_node(next_quad_label());
					$2->false_list = new_list_node(next_quad_label() +1);

					emit(IF_EQ, newexpr_const_bool(1),$2,NULL,-1, yylineno);
					emit(JUMP, NULL, NULL, NULL, -1, yylineno);
				}
				
				$$ = newexpr(BOOLEXPR_E);

				if($2->sym)
					$$->sym = $2->sym;
				else
					$$->sym = new_tmp();
					
				$$->true_list = $2->false_list;

				$$->false_list = $2->true_list;
			}

			| key_not_sym expr 	// "!"
			{
				if($2->type != BOOLEXPR_E){

					$2->true_list = new_list_node(next_quad_label());
					$2->false_list = new_list_node(next_quad_label() +1);

					emit(IF_EQ, newexpr_const_bool(1),$2,NULL,-1, yylineno);
					emit(JUMP, NULL, NULL, NULL, -1, yylineno);
				}
				
				$$ = newexpr(BOOLEXPR_E);

				if($2->sym)
					$$->sym = $2->sym;
				else
					$$->sym = new_tmp();
					

				$$->true_list = $2->false_list;

				$$->false_list = $2->true_list;

		 	}


			| term {$$=$1;};

		
V 			: {$$ = next_quad_label();}
			;





term		: left_parenthesis expr right_parenthesis 
			{ 
				$$= $2; 
			}

			| primary {$$ = $1;}


			| minus_op expr %prec uminus_rule 
			{
				if(check_uminus($2)){
					$$ = newexpr(ARITHEXPR_E);

					if(is_tmp_expr($2)){
						$$->sym = $2->sym;
					}else{
						$$->sym = new_tmp();
					}
					emit(UMINUS,$2,NULL,$$,currQuad,yylineno);
				}
			};	

			

			| inc_op lvalue 	
			{	
				if($2->type == PROGRAMFUNC_E || $2->type == LIBRARYFUNC_E){

					printf(RED"\nerror" RESET" %d: You cannot perform pre %s to function %s in line %u and scope %u\n\n",error_cnt++,$1,$2->sym->name,yylineno,scope );
					$$ = NULL;
				}else if($2->type == TABLEITEM_E){

					$$ = emit_if_tableitem($2);
					emit(ADD,$$,newexpr_const_num(1),$$,currQuad,yylineno);
					emit(TABLESETELEM,$2,$2->index,$$,currQuad,yylineno);

				}else{

					emit(ADD,$2,newexpr_const_num(1),$2,currQuad,yylineno);
					$$ = newexpr(ARITHEXPR_E);
					$$->sym = new_tmp();
					emit(ASSIGN,$2,NULL,$$,currQuad,yylineno);
				}
			};	


			| lvalue inc_op 
			{	
				if($1->type == PROGRAMFUNC_E || $1->type == LIBRARYFUNC_E){

					printf(RED"\nerror" RESET" %d: You cannot perform post %s to function %s in line %u and scope %u\n\n",error_cnt++,$2,$1->sym->name,yylineno,scope );
					$$ = NULL;
				}else{

					$$ = newexpr(ARITHEXPR_E);
					$$->sym = new_tmp();

					if($1->type == TABLEITEM_E){

						expr * value = emit_if_tableitem($1);
						emit(ASSIGN,value,NULL,$$,currQuad,yylineno);
						emit(ADD,value,newexpr_const_num(1),value,currQuad,yylineno);
						emit(TABLESETELEM,$1,$1->index,value,currQuad,yylineno);

					}else{

						emit(ASSIGN,$1,NULL,$$,currQuad,yylineno);
						emit(ADD,$1,newexpr_const_num(1),$1,currQuad,yylineno);
						
					}
				}
				
			};


			| dec_op lvalue
			{	
				if($2->type == PROGRAMFUNC_E || $2->type == LIBRARYFUNC_E){

					printf(RED"\nerror" RESET" %d: You cannot perform pre %s to function %s in line %u and scope %u\n\n",error_cnt++,$1,$2->sym->name,yylineno,scope );
				
				}else if($2->type == TABLEITEM_E){

					$$ = emit_if_tableitem($2);
					emit(SUB,$$,newexpr_const_num(1),$$,currQuad,yylineno);
					emit(TABLESETELEM,$2,$2->index,$$,currQuad,yylineno);

				}else{

					emit(SUB,$2,newexpr_const_num(1),$2,currQuad,yylineno);
					$$ = newexpr(ARITHEXPR_E);
					$$->sym = new_tmp();
					emit(ASSIGN,$2,NULL,$$,currQuad,yylineno);
				}
			};	


			| lvalue dec_op

			{	
				if($1->type == PROGRAMFUNC_E || $1->type == LIBRARYFUNC_E){

					printf(RED"\nerror" RESET" %d: You cannot perform post %s to function %s in line %u and scope %u\n\n",error_cnt++,$2,$1->sym->name,yylineno,scope );
				
				}else{

					$$ = newexpr(ARITHEXPR_E);
					$$->sym = new_tmp();

					if($1->type == TABLEITEM_E){

						expr * value = emit_if_tableitem($1);
						emit(ASSIGN,value,NULL,$$,currQuad,yylineno);
						emit(SUB,value,newexpr_const_num(1),value,currQuad,yylineno);
						emit(TABLESETELEM,$1,$1->index,value,currQuad,yylineno);

					}else{

						emit(ASSIGN,$1,NULL,$$,currQuad,yylineno);
						emit(SUB,$1,newexpr_const_num(1),$1,currQuad,yylineno);
						
					}
				}
				
			};							
			;									
			



primary		: lvalue 
			{
				$$=emit_if_tableitem($1);
			}

			| left_parenthesis funcdef right_parenthesis
			{
				$$ = lvalue_expr($2);
			} 

			| const 
			{
				$$=$1;
			}

			| tablemake
			{
				$$ = $1;
			}

			| call 
			{	
				$$ = $1;
			}											
			;



lvalue		: id 
			{		
				
				int i,j;
				symbol * tmp = NULL, *tmp2;
					
					
				for(i=scope;i>-1;i--){

					tmp = scope_link_lookup_ret($1,i);

					if(tmp != NULL){	//to vrhke
					
						
						if (funct==0){

							if(tmp->type == LIBRARYFUNC_S){
							}
							//yparxei se prohgoumena scope kai dn mesolavei synarthsh
							$$ = lvalue_expr(tmp);
						  	break;
						}else{
							
							
							if(tmp->type == PROGRAMFUNC_S || tmp->type == LIBRARYFUNC_S || tmp->scope == 0){

								if(tmp->type == LIBRARYFUNC_S){
								}
								$$ = lvalue_expr(tmp);
								break; //ok 

							}else if(tmp->type == VAR_S){
								
									if(i==scope){
										$$ = lvalue_expr(tmp);
										break;
									}
									
									for(j=i;j>-1;j--){
										tmp2 = scope_link_lookup_scope_and_space(j,PROGRAMVAR);

										if(tmp2 != NULL){
											
											if(get_line(tmp2) < yylineno && get_line(tmp) <= get_line(tmp2)){
												printf(RED"\nerror" RESET" %d:  Cannot access %s in line %u in function %s \n\n",error_cnt++,get_name(tmp),yylineno,get_name(tmp2));

												break;
											}
											
										} 
									}
									$$ = lvalue_expr(tmp);
									break;
							}
						
						}//else
					}//den to vrhskei sto link		
				}//for
				
				if(i==-1){	
			
					if(funct > 0){
						tmp = sym_table_insert(strdup($1),yylineno,scope,FUNCTIONLOCAL,VAR_S);
					}else{
						tmp = sym_table_insert(strdup($1),yylineno,scope,PROGRAMVAR,VAR_S);
						
					}
					
					$$ = lvalue_expr(tmp);
				}
				

			}

			| key_local id

			{			

				symbol * tmp = NULL;

				if(is_lib_func($2)){

					if(scope == 0){
						tmp = scope_link_lookup_ret($2,scope);
						$$ = lvalue_expr(tmp);
					}else{
						printf(RED"\nerror" RESET" %d: %s is LIBFUNC. Variable cannot shadow LIBFUNC in line %u and scope %u\n\n",error_cnt++,yylval.strVal,yylineno,scope);
						$$ = NULL;
					}

				}else{

					tmp = scope_link_lookup_ret($2,scope);
					if(tmp == NULL){

						if(scope == 0){
							tmp = sym_table_insert(strdup($2),yylineno,scope,PROGRAMVAR,VAR_S);

							$$ = lvalue_expr(tmp);
						}else{
							tmp = sym_table_insert(strdup($2),yylineno,scope,FUNCTIONLOCAL,VAR_S);
							$$ = lvalue_expr(tmp);
						}
					}else{
						$$ = lvalue_expr(tmp);
					}					
				}		
			}


			|double_colon id 
				
			{	
				symbol * tmp = NULL;
				tmp = scope_link_lookup_ret($2,0);
				if(tmp == NULL){
					printf(RED"\nerror" RESET" %d:  Referring to global %s in line %u but not declared\n\n",error_cnt++,$2,yylineno);
					$$ = NULL;
				}else{
					if(tmp->type == LIBRARYFUNC_S){
					}
					$$ = lvalue_expr(tmp);
				}
					
			}

			|tableitem 
			{
				$$=$1;
			}
			;	


tableitem 	: lvalue dot id
			{
				
				$$ = member_item($1,$3);	 
			}
						

			| lvalue left_bracket expr right_bracket 
			{
				$1 = emit_if_tableitem($1);
				$$ = newexpr(TABLEITEM_E);
				$$->sym = $1->sym;
				$$->index = $3;
			}

			|call dot id
			{
				$$ = member_item($1,$3);	 
			}


			| call left_bracket expr right_bracket 	
			{
				$1 = emit_if_tableitem($1);
				$$ = newexpr(TABLEITEM_E);
				$$->sym = $1->sym;
				$$->index = $3;
			}
			;



tablemake 	: left_bracket elist right_bracket
			{
				expr * tmp = NULL;
				int i =0;
				expr * t = newexpr(NEWTABLE_E);
				t->sym = new_tmp();
				emit(TABLECREATE,NULL,NULL,t,currQuad,yylineno);
				
				for(tmp = $2; tmp != NULL; tmp = tmp->next){
					emit(TABLESETELEM,t,newexpr_const_num(i++),tmp,currQuad,yylineno);
				}

				$$ = t;
			}


			| left_bracket indexed right_bracket
			{
				
				expr * tmp = NULL;
				expr * t = newexpr(NEWTABLE_E);
				t->sym = new_tmp();
				emit(TABLECREATE,NULL,NULL,t,currQuad,yylineno);
				
				expr *index_t = NULL;

				for(index_t = $2;index_t!=NULL;index_t=index_t->next){
					
					emit(TABLESETELEM,t,index_t,index_t->index,currQuad,yylineno);
				}

				$$ = t;
			}
			;


indexed 	: indexelem
			{
				$$=$1;
				$$->next = NULL;
			}

			|  indexed comma indexelem 

			{
				
				if($1 != NULL && $3 != NULL){
					expr * tmp = NULL;
					for(tmp = $1;tmp->next != NULL; tmp = tmp->next);
					tmp->next = $3;
					$3->next = NULL;
					$$ = $1;
				}
						
			} 		
			; 


indexelem	: left_brace expr colon expr right_brace
			{
				$$=$2;
				$$->index=$4;
				$$->next=NULL;
			} 	
			;





elist		: expr 
			{
				if($1 != NULL){

					if($1->type==BOOLEXPR_E){

						expr * new = newexpr(ASSIGNEXPR_E);
						new->sym = new_tmp();

						backpatch($1->true_list,next_quad_label()); //to true_list na kanei jump sto epomeno quad pou einai to assign true 

						emit(ASSIGN,newexpr_const_bool(1),NULL,new,currQuad,yylineno);
						emit(JUMP,NULL,NULL,NULL,next_quad_label()+2,yylineno);

						backpatch($1->false_list,next_quad_label()); // to false edw sto assign false

						emit(ASSIGN,newexpr_const_bool(0),NULL,new,currQuad,yylineno);

						$$ = new;

						
					}else{
						$$ = $1;
						}
					
					$$->next = NULL;
				}else{
					
					$$ = NULL;
				}
			}

			| elist comma expr 			
			{		
				if($1 != NULL && $3 != NULL){
					expr * tmp = NULL;
					for(tmp = $1;tmp->next != NULL; tmp = tmp->next);
					tmp->next = $3;
					$3->next = NULL;
					$$ = $1;
				}
			}

			| {$$ = NULL;}
			;





idlist		: id
			{	
				symbol * tmp = NULL;
				 if(is_lib_func($1)){
					printf(RED"\nerror" RESET" %d: Cannot use LIBFUNC %s as a formal argument in line %u \n\n",error_cnt++,$1,yylineno);
				}else{
					tmp = scope_link_lookup_ret($1,scope);
					if(tmp == NULL){

							tmp = sym_table_insert(strdup($1),yylineno,scope,FORMALARG,VAR_S);
							$$ = lvalue_expr(tmp);
							$$->next = NULL;
					}else{
						if (tmp->type == FORMALARG) {
							printf(RED"\nerror" RESET" %d: Cannot redefine %s as a formal argument in the same function in line %u \n\n",error_cnt++,$1,yylineno);
							$$ = NULL;

						}else {
							tmp = sym_table_insert(strdup($1),yylineno,scope,FORMALARG,VAR_S);
							$$ = lvalue_expr(tmp);
							$$->next = NULL;
						}	
					}
				}
			}


			| idlist comma id
				
			{
		
				symbol * tmp = NULL;

				 if(is_lib_func($3)){

					printf(RED"\nerror" RESET" %d: Cannot use LIBFUNC %s as a formal argument in line %u \n\n",error_cnt++,$3,yylineno);
				}else{

					tmp = scope_link_lookup_ret($3,scope);

					if(tmp == NULL){

							tmp = sym_table_insert(strdup($3),yylineno,scope,FORMALARG,VAR_S);

							expr * newexpr = lvalue_expr(tmp);

							expr *tmp2 = NULL;
							for(tmp2 = $1; tmp2->next != NULL; tmp2 = tmp2->next);
							tmp2->next = newexpr;
						$$= $1;

					}else{
						if (tmp->space == FORMALARG) {
							printf(RED"\nerror" RESET" %d: Cannot redefine %s as a formal argument in the same function in line %u \n\n",error_cnt++,$3,yylineno);
							$$ = NULL;

						}else {
							tmp = sym_table_insert(strdup($3),yylineno,scope,FORMALARG,VAR_S);

							expr * newexpr = lvalue_expr(tmp);
							
							expr *tmp2 = NULL;
							for(tmp2 = $1; tmp2->next != NULL; tmp2 = tmp2->next);
							tmp2->next = newexpr;
							$$= $1;
						}
					}
				}
			}

			| {$$ = NULL;}; 									
			;






funcname	: id
			{	
				symbol * tmp = NULL;
				tmp = scope_link_lookup_ret($1,scope);

				if(tmp == NULL){
					 tmp = sym_table_insert(strdup($1),yylineno,scope,PROGRAMVAR,PROGRAMFUNC_S);
					 $$ = tmp;
				}else{

					if(is_lib_func($1)) {
						printf(RED"\nerror" RESET" %d: Cannot assign LIBRARYFUNC_S %s as a PROGRAMFUNC_S in line %u \n\n",error_cnt++,$1,yylineno);
						
					}else{
						printf(RED"\nerror" RESET" %d: Cannot assign %s as a PROGRAMFUNC_S in line %u . Already defined in line %u and scope %u\n\n",error_cnt++,$1,yylineno,get_line(tmp),scope);	
					}

					char * error_name = (char *) malloc(255);
					sprintf(error_name,"_cannot_assign_name_%s",strdup($1));
					$$ = sym_table_insert(error_name,yylineno,scope,PROGRAMVAR,PROGRAMFUNC_S);
				}
				
			}


			| 
			{ 
				$$ = sym_table_insert(new_tmp_funcname(),yylineno,scope,PROGRAMVAR,PROGRAMFUNC_S);	 
			}
			;


funcprefix	: key_function {++funct;}funcname
			{
				
				$$ = $3;
				
				if(!is_lib_func($$->name))
					$$->addr.iaddress = next_quad_label();

				expr * lvalue = lvalue_expr($$);
				emit(FUNCSTART,NULL,NULL,lvalue,currQuad,yylineno);
		
				push(function_locals_stack,functionLocalOffset);
				enter_scope_space();
			
				reset_formalargs_offset();
			}
			;


funcargs	: left_parenthesis { ++scope;} idlist right_parenthesis
			{
			
				enter_scope_space();
				reset_functionlocals_offset();
				$$ = $3;
				scope--;
			}
			;


funcbody	: funcblockstart block funcblockend
			{
				$$ = $2;
			}
			;



funcblockstart: 
			{
				loop_counter_stack = push(loop_counter_stack,loop_counter);
				loop_counter = 0;
			}
			;


funcblockend: 
			{	
				loop_counter = pop(loop_counter_stack);
			}			
			;


block		: left_brace right_brace
			{
				$$ = 0;
			}



			| left_brace { ++scope;} stmts right_brace 
			{ 
				$$ = $3;
				hide_scope(scope--);
			}
			;



funcdef	 	: funcprefix funcargs funcbody
			{	
				
				exit_scope_space();
				exit_scope_space();
				$1->total_locals = functionLocalOffset;
				
				functionLocalOffset = pop(function_locals_stack);
				

				$$ = $1;
				emit(FUNCEND,NULL,NULL,lvalue_expr($1),currQuad,yylineno);
				--funct;

				expr * tmp_expr = NULL;
				for(tmp_expr = $2;tmp_expr != NULL; tmp_expr = tmp_expr->next ){
					tmp_expr->sym->isActive = 0;
				}
				

			}
			;



return_stmt : key_return semicolon 
			{
				if (funct == 0) {
					
					printf(RED"\nerror" RESET" %d: Use of 'return' while not in a function in line %u\n",error_cnt++,yylineno);
					$$ = new_stmt(INVALID_STMT);
				}else {
					emit(RET,NULL,NULL,NULL,currQuad,yylineno);
				}
			}


			| key_return expr semicolon 
			{
				if (funct == 0) {
					printf(RED"\nerror" RESET" %d: Use of 'return' while not in a function in line %u \n",error_cnt++,yylineno);
					$$ = new_stmt(INVALID_STMT);

				}else {

					if($2->type == BOOLEXPR_E){


						patch_label(currQuad-2,currQuad);
						patch_label(currQuad-1,currQuad+2);

							emit(RET,NULL,NULL,newexpr_const_bool(1),currQuad,yylineno);
							emit(JUMP,NULL,NULL,NULL,currQuad+2,yylineno);
							$$ = newstmt_expr($2,RETURN_STMT);
							emit(RET,NULL,NULL,newexpr_const_bool(0),currQuad,yylineno);
							$$ = newstmt_expr($2,RETURN_STMT);
						

					}else{
						
						emit(RET,NULL,NULL,$2,currQuad,yylineno);
						$$ = newstmt_expr($2,RETURN_STMT);
					}
				}
			}
			;





call 		: call left_parenthesis elist right_parenthesis
			{

				$$ = make_call ($1,$3);
			} 


			| lvalue callsuffix
			{	

				
				if($2->method != NULL){
					
					if($2->method->is_method == 1){
						
						expr *self = $1;
					
						expr * tmp = member_item( self, $2->method->methodname);
						
						$1 = emit_if_tableitem(tmp) ;

						expr * tmp2 = NULL;

						if($2 != NULL && $2->elist != NULL){
							for (tmp2 = $2->elist; tmp2->next != NULL ; tmp2 = tmp2->next);
							tmp2->next = NULL;
							self->next = $2->elist;
							$2->elist = self;
						}

					}
					
					$$ = make_call($1,$2->elist);
				}else{
				
					$$ =  make_call($1,NULL);
					
				}
			} 



			| left_parenthesis funcdef right_parenthesis left_parenthesis elist right_parenthesis 	
			{
				expr * func = newexpr(PROGRAMFUNC_E);
				func->sym = $2;
				$$ = make_call(func,$5);
			}	
			;
		

callsuffix	: normcall 	{$$=$1;}	
			| methodcall {$$=$1;}	
			;



normcall	: left_parenthesis elist right_parenthesis 
			{
				$$ = newexpr(VAR_E);

				if($2 != NULL)
					$$->elist = $2;
				else
					$$->elist = NULL;

				$$->method->is_method = 0;
				$$->method->methodname = NULL;
			}		
			;


methodcall	: double_dot id left_parenthesis elist right_parenthesis 
			{
				$$ = newexpr(1);
				$$->elist = $4;
				$$->method->is_method = 1;
				$$->method->methodname = strdup($2);
			}	
			;	









if_stmt		: ifprefix stmt %prec else_dummy
			{
				backpatch($1,next_quad_label()); 
												 //backpatching to (ifprefix= false_list tou f) sto current quad edw dhladh 8a kanei jump sto else
				$$ = $2;
			}

			| ifprefix stmt elseprefix stmt
			{
				printf("%d\n",($3)+1);
				backpatch($1,($3)+1);// patch to falselist sto elseprefix sto prwto quad tou else

				patch_label($3,next_quad_label()); // h teleutaia entolh tou true-if na kanei jump meta sto telos olou tou if

				$$ = new_stmt(IF_STMT);
				$$->break_list = merge($2->break_list,$4->break_list);
				$$->continue_list = merge($2->continue_list,$4->continue_list);

			}			
			;


ifprefix	: key_if left_parenthesis expr right_parenthesis
			{

				backpatch($3->true_list, next_quad_label()); // edw kanei patch to current quad giati exei teleiwsei to if kai edw kanei jump ama bei sthn if

				if($3->type != BOOLEXPR_E){

					emit(IF_EQ, newexpr_const_bool(1), $3, NULL, next_quad_label()+2, yylineno); // to +2 einai to prwto quad tou if-stmt
					$$ = new_list_node(next_quad_label());	// kouvalaw to ifprefix pou einai to epomeno quad pou einai to jump sto else
					emit(JUMP, NULL, NULL, NULL, -1, yylineno);	// to jump sto else
					
				} else {

					$$ = $3->false_list; //kouvalaw to false_list tou if
				}
			}
			;


elseprefix	: key_else
			{
				$$ = next_quad_label();
				emit(JUMP,NULL,NULL,NULL,-1,yylineno); // einai mesa sthn if kai kanei jump na prosperasei to else
			}
			;



whilestart	: key_while 
			{
				$$ = next_quad_label(); // einai edw pou 8a kanei to continue jump
			}
			;


whilecond	: left_parenthesis expr	right_parenthesis 
			{
				backpatch($2->true_list, next_quad_label()); // to true_list tou expr oso einai true tha kanei jump sthn syn8hkh

				if($2->type != BOOLEXPR_E){

					emit(IF_EQ, newexpr_const_bool(1), $2, NULL, next_quad_label()+2, yylineno);
					$$ = new_list_node(next_quad_label());
					emit(JUMP, NULL, NULL, NULL, -1, yylineno);

				} else {
					$$ = $2->false_list; // kouvalaw thn falselist
				}
			}		
			;


while_stmt	: whilestart whilecond loop_stmt
			{
				assert($3);
				emit(JUMP,NULL,NULL,NULL,$1,yylineno); 

				backpatch($2,next_quad_label());	//patch sto telos tou loop to falselist tou condition

				$3->type = LOOP_STMT;
				
				$$ = $3;
				
				backpatch(loop_bc_stack->break_list,next_quad_label());//patch sto telos tou loop to breaklist tou top ths stoivas.

				backpatch(loop_bc_stack->continue_list,$1);

				pop_bc(loop_bc_stack); 

			}
			;


loop_stmt 	: loop_start stmt loop_end
			{
				assert($2);
				$$ = $2;
			}
			;


loop_start  : 
			{	
				stack_bc_t * curr_loop = (stack_bc_t *) malloc (sizeof(stack_bc_t));

				curr_loop->break_list = NULL;
				curr_loop->continue_list = NULL;
				curr_loop->loop_num = ++loop_counter;

				push_bc(curr_loop); // push to twrino loop coonter, current loop == top ths stoivas.

			}
			;


loop_end  	: 
			{
				--loop_counter;
			}
			;



N:
{
	$$=next_quad_label();
	emit(JUMP,NULL,NULL,NULL,currQuad,yylineno);
};


M:
{
	$$=next_quad_label();
};



forprefix : key_for left_parenthesis elist M semicolon expr semicolon 
		{
			
			$$ = $6;

			if($6->type != BOOLEXPR_E){

					emit(IF_EQ, newexpr_const_bool(1), $6, NULL, next_quad_label()+2, yylineno);
					$$->false_list = new_list_node(next_quad_label());
					emit(JUMP, NULL, NULL, NULL, -1, yylineno); 

			}
			$$->for_test = $4;
			$$->for_enter = next_quad_label();		
		};



for_stmt: forprefix N elist right_parenthesis N loop_stmt N 
		{
			

				backpatch($1->false_list,next_quad_label());  

				backpatch($1->true_list,$5+1);	
			
				patch_label($1->for_enter,$5+1);	
			
				patch_label($5,$1->for_test);

				patch_label($7,$2+1);	

				backpatch(loop_bc_stack->break_list,next_quad_label());

				backpatch(loop_bc_stack->continue_list,$2+1);	

				$$ = $6;

				pop_bc(loop_bc_stack); 

			};
		



const 		: int_num 	
				{	
					$$ = newexpr_const_num((double)$1);
				}	

			| double_num 
				{
					$$ = newexpr_const_num($1);
				}	

			| string
				{
					$$ = newexpr_const_string($1);
				}

			| key_nil 	
				{	
					$$ = newexpr(NIL_E);
				}	

			| key_true 	
				{
					$$ = newexpr_const_bool(1);
				}	

			| key_false 
				{	
					$$ = newexpr_const_bool(0);
				}	
			;



%%


int main (int argc,char ** argv) {

	
	init_all();

	if(argc > 1){

		yyin = fopen(argv[1],"r");

		if(!yyin){
			printf("Cannot read file: %s\n",argv[1] );
			return -1;
		}

	}else{
			yyin =stdin;
	}
	
	yyparse();

	print_quads();

	generate_t();

	print_arrays();

	write_binary();

	// free_compiler();

	return 0;
}
