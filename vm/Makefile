CC=clang
CFLAGS+=-O2 -W -Wall -Wextra -pedantic -std=gnu99
SRC=src/main.c src/vm.c src/opcodes.c src/term.c src/alloc.c
LINKS=-lgc

all: vm

vm:
	mkdir -p target
	$(CC) $(SRC) $(CFLAGS) $(LINKS) -o target/vm

clean:
	@rm -rf target
