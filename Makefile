.PHONY: all compiler stdlib vm

all: compiler stdlib vm

compiler:
	cd compiler && cargo build

vm:
	cd vm && bin/build debug

stdlib:
	compiler/target/debug/owlc stdlib/tuple.owl -o stdlib/.build
