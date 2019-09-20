

scopespace_t curr_scope_space(void) {
	if (scopeSpaceCounter == 1)
		return PROGRAMVAR;
	else if (scopeSpaceCounter % 2 == 0)
		return FORMALARG;
	else
		return FUNCTIONLOCAL;
}

char * csspace_totext(scopespace_t space) {

	switch (space){

	case PROGRAMVAR     : return "PROGRAMVAR";
	case FUNCTIONLOCAL  : return "FUNCTIONLOCAL";
	case FORMALARG      : return "FORMALARG";
	default: assert(0);
	}
}


unsigned curr_scope_offset (void) {

	switch (curr_scope_space()) {
	case PROGRAMVAR     : return programVarOffset;
	case FUNCTIONLOCAL  : return functionLocalOffset;
	case FORMALARG      : return formalArgOffset;
	default: assert(0);
	}
}


void inc_curr_scope_offset (void) {
	switch (curr_scope_space()) {
	case PROGRAMVAR     : ++programVarOffset; break;
	case FUNCTIONLOCAL  : ++functionLocalOffset; break;
	case FORMALARG      : ++formalArgOffset; break;
	default: assert(0);
	}
}


void restore_curr_scope_offset (unsigned n) {
	switch (curr_scope_space()) {
	case PROGRAMVAR     : programVarOffset = n; break;
	case FUNCTIONLOCAL  : functionLocalOffset = n; break;
	case FORMALARG      : formalArgOffset = n; break;
	default: assert(0);
	}
}


void enter_scope_space (void) { ++scopeSpaceCounter; }


void reset_formalargs_offset(void) { formalArgOffset = 0; }


void reset_functionlocals_offset (void) { functionLocalOffset = 0;}


void exit_scope_space (void) { assert (scopeSpaceCounter > 1); --scopeSpaceCounter; }


void  push_expr(expr * new) {
	
		if (param_stack == NULL) {
			param_stack = (stack_expr_t *) malloc (sizeof(stack_expr_t));
			param_stack->expr = new;
			param_stack->next = NULL;
	
		} else {

		stack_expr_t *	new_node =( stack_expr_t *) malloc (sizeof(stack_expr_t));
		new_node->next = param_stack;
		new_node->expr = new;
		param_stack = new_node;
	}
}

expr * pop_expr() {

	expr * tmp = NULL;

	if (param_stack == NULL) {

		return NULL;

	} else if (param_stack->next == NULL) {

		tmp = param_stack->expr;
		param_stack = NULL;

	} else {

		tmp = param_stack->expr;
		param_stack = param_stack->next;

	}
	return tmp;
}

int check_uminus(expr * expr) {

	if ( expr->type == VAR_E || expr->type == ARITHEXPR_E || expr->type == CONSTNUM_E)
		return 1;
	else
		return 0;
}


unsigned next_quad_label(void) {
	return currQuad;
}


void patch_label(unsigned quadNo, unsigned label) {
	assert(quadNo < currQuad);
	quads[quadNo].label = label;
}


void backpatch(list_node * head, unsigned label) {
	list_node * tmp = NULL;

	for (tmp = head; tmp != NULL; tmp = tmp->next) {
		patch_label(tmp->label, label);
	}
}


void expand (void) {

	assert (total == currQuad);

	quad* p = (quad*) malloc(NEW_SIZE);

	if (quads != NULL) {
		memcpy(p, quads, CURR_SIZE);
		free(quads);
	}
	quads = p;
	total += EXPAND_SIZE;
}


void emit (iopcode op, expr * arg1, expr * arg2, expr  *result, unsigned label, unsigned line ) {

	if (currQuad == total) expand();

	quad* p     = quads + currQuad++;
	p->op       = op;
	p->arg1     = arg1;
	p->arg2     = arg2;
	p->result   = result;
	p->label    = label;
	p->line     = line;
}


expr* newexpr (expr_t t) {

	expr* e = (expr*) malloc(sizeof(expr));
	e->method = (method_t *) malloc(sizeof(method_t));
	
	e->true_list = NULL;
	e->false_list = NULL;
	e->type = t;

	return e;
}


expr *  emit_if_tableitem (expr * arg) {

	if(arg != NULL){
		if (arg->type != TABLEITEM_E) {

			return arg;
		} else {
			expr * result = newexpr(VAR_E);
			result->sym = new_tmp();

			emit(TABLEGETELEM, arg, arg->index, result, currQuad, yylineno);
			return result;
		}
	}else{
		return NULL;
	}
}


expr* lvalue_expr (symbol* sym) {

	assert(sym);

	expr* e = (expr*) malloc(sizeof(expr));
	e->method = (method_t *) malloc(sizeof(method_t ));


	e->next = (expr*) 0;
	e->sym  = sym;

	switch (sym->type) {
	case VAR_S          : e->type = VAR_E; break;
	case PROGRAMFUNC_S  : e->type = PROGRAMFUNC_E; break;
	case LIBRARYFUNC_S  : e->type = LIBRARYFUNC_E; break;
	default: assert(0);
	}

	return e;
}


stmt * newstmt_expr (expr * exp, stmt_type type) {

	assert(exp);

	stmt* st = (stmt *) calloc(1,sizeof(stmt));
	st->expr = (expr *) calloc(1,sizeof(expr));
	st->expr->method = (method_t *) calloc(1,sizeof(method_t ));

	st->expr  = exp;
	st->type = type;
	return st;
}


stmt * newstmt_symbol (symbol * sym, stmt_type type) {

	assert(sym);

	stmt* st = (stmt *) calloc(1,sizeof(stmt));
	st->expr = (expr *) calloc(1,sizeof(expr));
	st->expr->method = (method_t *) calloc(1,sizeof(method_t ));
	st->expr->sym = (symbol *) calloc(1,sizeof(symbol));

	st->expr->sym  = sym;
	st->type = type;
	return st;
}


stmt * new_stmt ( stmt_type type) {


	stmt* st = (stmt*) calloc(1,sizeof(stmt));
	st->expr = (expr*) calloc(1,sizeof(expr));
	st->expr->method = (method_t*) calloc(1,sizeof(method_t));

	st->type = type;
	return st;
}


stmt * new_break_stmt(list_node * break_list) {


	stmt* st = (stmt*) calloc(1,sizeof(stmt));
	st->expr = (expr*) calloc(1,sizeof(expr));
	st->expr->method = (method_t*) calloc(1,sizeof(method_t));

	st->break_list = break_list;
	st->type = BREAK_STMT;
	return st;
}


stmt * new_continue_stmt(list_node * continue_list) {


	stmt* st = (stmt*) calloc(1,sizeof(stmt));
	st->expr = (expr*) calloc(1,sizeof(expr));
	st->expr->method = (method_t*) calloc(1,sizeof(method_t));

	st->continue_list = continue_list;
	st->type = CONTINUE_STMT;
	return st;
}


expr* newexpr_const_string (char* s) {
	expr* e = newexpr(CONSTSTRING_E);
	e->strConst = strdup(s);
	return e;
}


expr* newexpr_const_num (double x) {
	expr* e = newexpr(CONSTNUM_E);
	e->numConst = x;
	return e;
}

expr* newexpr_const_bool (int x) {
	expr* e = newexpr(CONSTBOOL_E);
	if (x)
		e->boolConst = 1;
	else
		e->boolConst = 0;
	return e;
}


void reset_tmp() {

	symbol * tmp = NULL, *tmp2 = NULL;
	tmp_counter = 0;

	for (tmp = scope_link_list; tmp != NULL; tmp = tmp->next) {

		for (tmp2 = tmp->scope_link_next; tmp2 != NULL; tmp2 = tmp2->scope_link_next) {

			if (is_tmp_name(tmp2->name)) {
				tmp2->isActive = 0;
			}
		}
	}
}


unsigned is_tmp_name(char * s) { return *s == '_'; }



unsigned int is_tmp_expr( expr * e) {

	if (e->sym != NULL && e->sym->type == VAR_S && is_tmp_name(e->sym->name))
		return 1;
	else
		return 0;

}


char * new_tmp_name() {

	char * str = (char *)malloc(128);
	sprintf(str, "_t%d", tmp_counter++);
	return str;
}


char * new_tmp_funcname() {

	char * str = (char *)malloc(128);
	sprintf(str, "_f%d", func_name++);
	return str;
}


symbol * new_tmp() {

	char * name = new_tmp_name();

	symbol * a  = NULL;

	a = lookup_hashtable(name, scope);

	if (a == NULL) {

		a = sym_table_insert(name, yylineno, scope, curr_scope_space(), VAR_S);
		return a;

	} else {

		return a;
	}
}


symbol * new_symbol(char * name, symbol_t type, unsigned line) {

	symbol * new = (symbol *)malloc(sizeof(symbol));
	new->isActive = 1;
	new->name = name;
	new->type = type;
	new->space = curr_scope_space();
	new->offset = curr_scope_offset();
	if (type != LIBRARYFUNC_S){
		inc_curr_scope_offset();
	} 
	new->scope = scope;
	new->line = line;
	new->next = NULL;
	new->scope_link_next = NULL;
	return new;
}


expr * member_item(expr * lvalue, char * name) {

	lvalue = emit_if_tableitem(lvalue);
	expr * item = newexpr(TABLEITEM_E);
	item->sym = lvalue->sym;
	char * s = (char *)malloc(128);
	sprintf(s,"\"%s\"",name);
	item->index = newexpr_const_string(s);
	return item;

}



expr * make_call(expr* lvalue, expr * elist) {

	
	expr * func = emit_if_tableitem(lvalue);

	expr * tmp = NULL;

	for (tmp = elist; tmp != NULL; tmp = tmp->next) {
		push_expr(tmp);
	}


	while(param_stack != NULL){
		tmp = pop_expr();
		emit(PARAM, NULL, NULL, tmp, currQuad, yylineno);
	}


	emit(CALL, NULL, NULL, func, currQuad, yylineno);
	expr * result = newexpr(VAR_E);
	result->sym = new_tmp();

	emit(GETRETVAL, NULL, NULL, result, currQuad, yylineno);
	return result;

}



list_node * new_list_node(unsigned label) {

	list_node * new = (list_node *)malloc (sizeof (list_node));
	new->label = label;
	new->is_skip = 0;
	new->next = NULL;
	return new;
}



list_node * merge (list_node * list_a, list_node * list_b) {
	
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



stack_offset_t * push(stack_offset_t * stack, int local_offset) {

	stack_offset_t * new = (stack_offset_t *)malloc(sizeof(stack_offset_t));
	new->offset = local_offset;

	if (stack == NULL) {


		stack = new;
		stack->next = NULL;
		return stack;

	} else {
		new->next = stack;
		stack = new;
		return stack;
	}

}

void push_bc(stack_bc_t * new) {
	
		if (loop_bc_stack == NULL) {

		loop_bc_stack = new;
		loop_bc_stack->next = NULL;
	

	} else {
		
		new->next = loop_bc_stack;
		loop_bc_stack = new;
	
	}

}





int pop(stack_offset_t * stack) {

	int offset;

	if (stack == NULL) {

		return 0;

	} else if (stack->next == NULL) {

		offset = stack->offset;
		stack = NULL;

	} else {

		offset = stack->offset;
		stack = stack->next;

	}
	return offset;
}


	

stack_bc_t * pop_bc() {
	
	stack_bc_t * tmp = NULL;

	if (loop_bc_stack == NULL) {
	
		return NULL;

	} else if (loop_bc_stack->next == NULL) {

		tmp = loop_bc_stack;
		loop_bc_stack = NULL;

	} else {

		tmp = loop_bc_stack;
		loop_bc_stack = loop_bc_stack->next;
	}
	return tmp;
}
