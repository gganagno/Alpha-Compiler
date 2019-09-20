#include "avm.c"


int main (int argc,char ** argv) {

	read_binary();

	avm_initialize();

	print_instructions();
	
	execute_cycle();

	return 0;
}