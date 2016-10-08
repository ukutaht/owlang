.PHONY: all compiler vm stdlib libs intern

all: libs compiler stdlib vm

compiler:
	cd compiler && cargo build

vm:
	cd vm && bin/build debug

intern:
	cd vm/lib/intern && \
		cmake -G 'Unix Makefiles' -Wno-dev -DBUILD_STATIC=1 -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX=../target && \
		make install

libs: intern

stdlib:
	compiler/target/debug/owlc stdlib -o .build/stdlib

clean:
	rm -rf compiler/target vm/target .build

check: check-compiler check-test-cases

check-compiler: compiler
	cd compiler && cargo test

check-test-cases: vm stdlib
	vm/target/debug/vm .build/stdlib/OwlUnitRunner.owlc
