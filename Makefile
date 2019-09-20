all: COMP AAVM
	echo "\n\nUsage :\n make run p=(path of testfile)\n"


scanner.c: scanner.l  
	flex scanner.l

parser.c: parser.y
	yacc -v -d -t --output=parser.c parser.y

out: 
	gcc -g -o comp scanner.c parser.c 


COMP: scanner.c parser.c out


AAVM: 
	gcc -g -o avm avm_machine.c -lm

run:
	./comp < $(p)
	./avm

runfunc:
	./comp < testfile_func.txt

runlib:
	./comp < testfile_lib.txt

runhash:
	./comp < testfile_hash.txt

runa:
	./avm 

clean:
	rm  parser.h parser.c scanner.c *.output comp quads.txt tcode.abc avm
