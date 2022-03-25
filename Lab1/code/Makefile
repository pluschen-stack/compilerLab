# # GNU make手册：http://www.gnu.org/software/make/manual/make.html
# # ************ 遇到不明白的地方请google以及阅读手册 *************

# # 编译器设定和编译选项
# CC = gcc
# FLEX = flex
# BISON = bison
# CFLAGS = -std=c99

# # 编译目标：src目录下的所有.c文件
# CFILES = $(shell find ./ -name "*.c")
# OBJS = $(CFILES:.c=.o)
# LFILE = $(shell find ./ -name "*.l")
# YFILE = $(shell find ./ -name "*.y")
# LFC = $(shell find ./ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
# YFC = $(shell find ./ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
# LFO = $(LFC:.c=.o)
# YFO = $(YFC:.c=.o)

# parser: syntax $(filter-out $(LFO),$(OBJS))
# 	$(CC) -c -o parser $(filter-out $(LFO),$(OBJS)) -lfl

# syntax: lexical syntax-c
# 	$(CC) -g -c $(YFC) -o $(YFO)

# lexical: $(LFILE)
# 	$(FLEX) -o $(LFC) $(LFILE)

# syntax-c: $(YFILE)
# 	$(BISON) -o $(YFC) -d -v $(YFILE)

# -include $(patsubst %.o, %.d, $(OBJS))

# # 定义的一些伪目标
# .PHONY: clean test
# test:
# 	./parser ../Test/test1.cmm
# clean:
# 	rm -f parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output
# 	rm -f $(OBJS) $(OBJS:.o=.d)
# 	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
# 	rm -f *~

# program = main

main:syntax.y lexer.l main.c node.c
	flex -o lex.yy.c lexer.l 
	bison -o syntax.tab.c -d -v syntax.y
	cc -g node.c syntax.tab.c main.c  -lfl -o main
# cc -g -c syntax.tab.c -o syntax.tab.o
# cc -g -c -o main.o main.c
# cc -g -c -o node.o node.c
# cc -g -o main ./node.o ./main.o ./syntax.tab.o -lfl
	
.PHONY: clean 
clean: 
	-rm $(program) main *.o syntax.output *.tab.* lex.yy.c

