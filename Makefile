
CC=clang
CFLAGS=-g3 -Wall
CFLAGS+= -Wl,-no_deduplicate
LDFLAGS=

# Use below to turn on sanitizers
#CC+= -fsanitize=undefined
#CC+= -fsanitize=address

# Use below for external clang from Homebrew
#LDFLAGS="-L/opt/homebrew/opt/llvm/lib/c++ -Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++ "-L/opt/homebrew/opt/llvm/li"
#CC=/opt/homebrew/opt/llvm/bin/clang
#CFLAGS+= -I/opt/homebrew/opt/llvm/include

all: program_opt program_dbg

program_opt: test.c Makefile
	$(CC) -o program_opt $(CFLAGS) -O2 test.c

program_dbg: test.c Makefile
	$(CC) -o program_dbg $(CFLAGS) -O1 test.c

.PHONY :clean test
clean:
	rm -rf program_opt.dSYM program_opt program_dbg.dSYM program_dbg

test: program_opt program_dbg
	./program_dbg
	./program_opt
