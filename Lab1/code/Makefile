
main:syntax.y lexer.l main.c node.c util.c
	flex -o lex.yy.c lexer.l 
	bison -o syntax.tab.c -d -v syntax.y
	cc -g util.c node.c syntax.tab.c main.c  -lfl -o main
	
.PHONY: clean test
clean: 
	-rm $(program) main *.o syntax.output *.tab.* lex.yy.c
test:
	./main ../test/sample1.txt
	./main ../test/sample2.txt
	./main ../test/sample3.txt
	./main ../test/sample4.txt
	./main ../test/sample5.txt
	./main ../test/sample6.txt
	./main ../test/sample7.txt
	./main ../test/sample8.txt
	./main ../test/sample9.txt
	./main ../test/sample10.txt