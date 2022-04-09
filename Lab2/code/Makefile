
main:syntax.y lexer.l main.c node.c util.c semantics.c
	flex -o lex.yy.c lexer.l 
	bison -o syntax.tab.c -d -v syntax.y
	cc -g util.c node.c syntax.tab.c semantics.c main.c  -lfl -o main

main-debug:syntax.y lexer.l main.c node.c util.c semantics.c
	flex -o lex.yy.c lexer.l 
	bison -o syntax.tab.c -d -v syntax.y
	cc -D DEBUGON -g util.c node.c syntax.tab.c semantics.c main.c  -lfl -o main
	
.PHONY: clean test
clean: 
	-rm $(program) main *.o syntax.output *.tab.* lex.yy.c
test:
	./main ../test/havetodo/test1
	./main ../test/havetodo/test2
	./main ../test/havetodo/test3
	./main ../test/havetodo/test4
	./main ../test/havetodo/test5
	./main ../test/havetodo/test6
	./main ../test/havetodo/test7
	./main ../test/havetodo/test8
	./main ../test/havetodo/test9
	./main ../test/havetodo/test10
	./main ../test/havetodo/test11
	./main ../test/havetodo/test12
	./main ../test/havetodo/test13
	./main ../test/havetodo/test14
	./main ../test/havetodo/test15
	./main ../test/havetodo/test16
	./main ../test/havetodo/test17