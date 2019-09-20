

int yyerror(char * yaccmsg) {

	printf(RED"%s "RESET": At line %d before token %s \n", yaccmsg, yylineno, yytext);
}




unsigned int get_scope(symbol * entry) {
	return entry->scope;
}

const char * get_name(symbol * entry) {
	return entry->name;
}

unsigned int get_line(symbol * entry) {
	return entry->line;
}

unsigned int sym_table_hash(const char *key) {

	unsigned int i;
	unsigned int uiHash = 0U;

	for (i = 0U; key[i] != '\0'; i++)
		uiHash = uiHash * HASH_MUL + key[i];

	return uiHash % SIZE;
}


void sym_table_init(void) {
	symbol_table = (symbol **) malloc (SIZE * sizeof(symbol *));
}


symbol * lookup_hashtable(char * name, unsigned int scope) {

	symbol * tmp = NULL;

	tmp = symbol_table[ sym_table_hash(name)];

	for (; tmp != NULL; tmp = tmp->next ) {
		if (get_name(tmp) != NULL) {
			if ((strcmp(name, get_name(tmp)) == 0) && scope == get_scope(tmp)) {
				if (tmp->isActive)return tmp;
			}
		}
	}

	return NULL;
}


int scope_link_lookup(const char * new_entry, unsigned int scope)  {

	symbol * tmp, * tmp2;

	for (tmp = scope_link_list; tmp != NULL; tmp = tmp->next) {
		if (get_scope(tmp) != scope)break;
	}

	if (tmp == NULL)return 0;

	for (tmp2 = tmp->scope_link_next; tmp2 != NULL; tmp2 = tmp2->scope_link_next) {

		if (strcmp(get_name(tmp2), new_entry) == 0) {
			if (tmp2->isActive) {
				return 1;
			} else {
				return 0;
			}
		}
	}
}


symbol * scope_link_lookup_ret(const char * new_entry, unsigned int scope)  {

	symbol * tmp, * tmp2;

	for (tmp = scope_link_list; tmp != NULL && get_scope(tmp) != scope ; tmp = tmp->next);


	if (tmp == NULL) {
		return NULL;
	}

	for (tmp2 = tmp->scope_link_next; tmp2 != NULL; tmp2 = tmp2->scope_link_next) {

		if (tmp2->isActive) {

			if (strcmp(get_name(tmp2), new_entry) == 0) {

				return tmp2;
			}
		}
	}

	return NULL;
}


symbol * scope_link_lookup_scope_and_space(unsigned int scope , scopespace_t space) {

	symbol * tmp, * tmp2;

	for (tmp = scope_link_list; tmp != NULL && get_scope(tmp) != scope ; tmp = tmp->next);

	if (tmp == NULL) {
		return NULL;
	}

	for (tmp2 = tmp->scope_link_next; tmp2 != NULL; tmp2 = tmp2->scope_link_next) {

		if (tmp2->space == space) {

			if (tmp2->isActive)
				return tmp2;
			else
				return NULL;
		}
	}
}


void scope_link_init() {

	scope_link_list = (symbol *) malloc (sizeof(symbol));
	scope_link_list->name = "dummy";
	scope_link_list->scope = 0;
	scope_link_list->next = NULL;
	scope_link_list->scope_link_next = NULL;

}


void add_dummy(unsigned int scope) {

	symbol * tmp;
	symbol * new_dummy = (symbol *) malloc (sizeof(symbol));
	new_dummy->name = "dummy";
	new_dummy->scope = scope;
	new_dummy->next = NULL;
	new_dummy->scope_link_next = NULL;

	for (tmp = scope_link_list; tmp->next != NULL && scope >= get_scope(tmp->next) ; tmp = tmp->next);

	if (tmp->next != NULL) {

		new_dummy->next = tmp->next;
		tmp->next = new_dummy;

	} else {
		tmp->next = new_dummy;
		new_dummy->next = NULL;

	}
}


int have_dummy(unsigned int scope) {

	symbol * tmp_dummy;
	for (tmp_dummy = scope_link_list ; tmp_dummy != NULL ; tmp_dummy = tmp_dummy->next) {

		if (scope == get_scope(tmp_dummy))
			return 1;
	}
	return 0;
}


int scope_link_insert(symbol * new_entry) {

	symbol * tmp = NULL;
	symbol * tmp_dummy = NULL;

	if (!have_dummy(get_scope(new_entry))) {
		add_dummy(get_scope(new_entry));
	}

	for (tmp_dummy = scope_link_list ; tmp_dummy != NULL ; tmp_dummy = tmp_dummy->next) {

		if (get_scope(new_entry) == get_scope(tmp_dummy)) {

			if (tmp_dummy->scope_link_next == NULL) {

				tmp_dummy->scope_link_next = new_entry;
				new_entry->scope_link_next = NULL;
				return 1;

			} else {

				for (tmp = tmp_dummy->scope_link_next ; tmp->scope_link_next != NULL; tmp = tmp->scope_link_next);
				tmp->scope_link_next = new_entry;
				new_entry->scope_link_next = NULL;
				return 1;
			}
		}
	}
}


int hide_scope(unsigned int scope) {
	symbol * tmp, * tmp2;

	for (tmp = scope_link_list; tmp != NULL && get_scope(tmp) != scope; tmp = tmp->next);

	if (tmp == NULL) return 0;
	for (tmp2 = tmp->scope_link_next; tmp2 != NULL; tmp2 = tmp2->scope_link_next) {
		tmp2->isActive = 0;
	}
	return 1;
}


int is_lib_func(const char * func_name) {

	if ( !strcmp(func_name, "print") ||
	        !strcmp(func_name, "input") ||
	        !strcmp(func_name, "objectmemberkeys") ||
	        !strcmp(func_name, "objecttotalmembers") ||
	        !strcmp(func_name, "objectcopy") ||
	        !strcmp(func_name, "totalarguments") ||
	        !strcmp(func_name, "argument") ||
	        !strcmp(func_name, "typeof") ||
	        !strcmp(func_name, "strtonum") ||
	        !strcmp(func_name, "sqrt") ||
	        !strcmp(func_name, "cos") ||
	        !strcmp(func_name, "sin") )
	{
		return 1; // it is a lib_func
	} else {
		return 0; //not
	}
}


symbol*  sym_table_insert(char * name, unsigned int line, unsigned int scope, scopespace_t space, symbol_t type)  {


	symbol * head = NULL, * tmp = NULL;


	if (scope_link_lookup(name, scope) == 1) {

		if (is_lib_func(name)) {
			printf(RED"error"RESET" %d: Cannot shadow libfunc %s in line %u and scope %u\n", error_cnt++, name, yylineno, scope);
			return NULL;
		}

	}

	symbol * new_entry = (symbol *)malloc(sizeof(symbol));
	new_entry->name = name;
	new_entry->isActive = 1;
	new_entry->type = type;
	new_entry->space = space;
	new_entry->line = line;
	new_entry->scope = scope;
	
	new_entry->offset = curr_scope_offset();

	if(type != LIBRARYFUNC_S && type != PROGRAMFUNC_S){
		inc_curr_scope_offset();
	}

	if (type == PROGRAMFUNC_S) {

		new_entry->addr.iaddress = 0;

	} else if(type == LIBRARYFUNC_S){
		new_entry->addr.lib_addr = strdup(name);
	}


	new_entry->total_locals = 0;
	new_entry->next = NULL;
	new_entry->scope_link_next = NULL;


	head = symbol_table[ sym_table_hash(name) ];

	if (head == NULL) {

		symbol_table[ sym_table_hash(name) ] = new_entry;
		symbol_table[ sym_table_hash(name) ]->next = NULL;

		scope_link_insert(new_entry);
		return new_entry;
	}

	scope_link_insert(new_entry);

	for (tmp = head; tmp->next != NULL; tmp = tmp->next);

	tmp->next = new_entry;
	new_entry->next = NULL;
	return new_entry;
}




void insert_lib_func() {
	sym_table_insert("print", 0, 0, PROGRAMVAR, LIBRARYFUNC_S);
	sym_table_insert("input", 0, 0, PROGRAMVAR, LIBRARYFUNC_S);


	sym_table_insert("objectmemberkeys", 0, 0, PROGRAMVAR, LIBRARYFUNC_S);
	sym_table_insert("objecttotalmembers", 0, 0, PROGRAMVAR, LIBRARYFUNC_S);
	sym_table_insert("objectcopy", 0, 0, PROGRAMVAR, LIBRARYFUNC_S);

	sym_table_insert("totalarguments", 0, 0, PROGRAMVAR, LIBRARYFUNC_S);
	sym_table_insert("argument", 0, 0, PROGRAMVAR, LIBRARYFUNC_S);
	sym_table_insert("typeof", 0, 0, PROGRAMVAR, LIBRARYFUNC_S);
	sym_table_insert("strtonum", 0, 0, PROGRAMVAR, LIBRARYFUNC_S);
	sym_table_insert("sqrt", 0, 0, PROGRAMVAR, LIBRARYFUNC_S);
	sym_table_insert("cos", 0, 0, PROGRAMVAR, LIBRARYFUNC_S);
	sym_table_insert("sin", 0, 0, PROGRAMVAR, LIBRARYFUNC_S);
}


void init_all() {
	sym_table_init();
	scope_link_init();
	insert_lib_func();
}

