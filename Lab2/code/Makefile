
main:syntax.y lexer.l main.c node.c util.c
	flex -o lex.yy.c lexer.l 
	bison -o syntax.tab.c -d -v syntax.y
	cc -g util.c node.c syntax.tab.c semantics.c main.c  -lfl -o main
	
.PHONY: clean test
clean: 
	-rm $(program) main *.o syntax.output *.tab.* lex.yy.c
test:
	./main test.cmm