.PHONY: all compiler stdlib vm libs intern c-rrb

all: libs compiler stdlib vm

compiler:
	cd compiler && cargo build

vm:
	cd vm && bin/build debug

c-rrb:
	cd vm/lib/c-rrb && \
		cmake -H. -Btarget/release -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX=../target && \
		cd target/release && \
		make install

intern:
	cd vm/lib/intern && \
		cmake -G 'Unix Makefiles' -Wno-dev -DBUILD_STATIC=1 -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX=../target && \
		make install

libs: c-rrb intern

stdlib:
	compiler/target/debug/owlc stdlib -o .build/stdlib

clean:
	rm -rf compiler/target vm/target .build lib/target

check: check-compiler check-test-cases

check-compiler:
	cd compiler && cargo test

check-test-cases:
	test_cases/run
