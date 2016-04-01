CC=clang
CFLAGS+=-O2 -W -Wall -Wextra -pedantic -std=gnu99
SRC=src/main.c src/vm.c src/opcodes.c

all: vm

vm:
	mkdir -p target
	$(CC) $(SRC) $(CFLAGS) -o target/vm

clean:
	@rm -rf target

compile:
	for i in examples/*.in; do ./compiler $$i >/dev/null  ; done

