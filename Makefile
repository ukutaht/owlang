.PHONY: all compiler stdlib vm

all: compiler stdlib vm

compiler:
	cd compiler && cargo build

vm:
	cd vm && bin/build debug

stdlib:
	compiler/target/debug/owlc stdlib -o .build/stdlib

clean:
	rm -rf compiler/target && rm -rf vm/target && rm -rf .build

check: check-compiler check-test-cases

check-compiler:
	cd compiler && cargo test

check-test-cases:
	test_cases/run
