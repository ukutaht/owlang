### Big list of TODO items

* Use static libraries for deps
* A graceful way to fatally quit the VM with a message (for example, when function is not found)
* Ensure that infix operations work for everything in the first argument
* Report parsing errors (depends on next chomp release)
* Stdlib testing framework (does it need compiler support?)
* Vim syntax file
* Basic build tool
* Tuple stdlib (bounds check for `nth`)
* List stdlib
* IO stdlib
* Public/Private distinction
* Deal with parsing edge cases(keywords)
* Module constants
* Generalise `if` to `cond`
* Dynamically grow the stack
* Dynamically grow the code array
* Dynamically grow register array
* Dynamically grow function table
* Provide allocator for c-rrb
* Signed ints
* Floats
* Strings
* Maps
* Arbitrary-percision floats
* BigInts
* Module imports
* Alias module
* Exceptions (Is it possible to avoid them?)
* Check that functions exist at compile time
* Dead code analysis
* Higher-order functions
* Closures (oh boy)
* Pattern matching
* Benchmarks
* Direct threaded VM
* Register windowing
* Linear-Scan register allocation in compiler
* Custom garbage collection
* FFI (at least C)
* Lightweight concurrency (CSP? Actors?)
* dare I say tracing JIT?
