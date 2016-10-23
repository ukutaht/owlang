### Big list of TODO items

* Reference registers and upvalues zero-indexed
* Ensure that captured functions are called with correct arity (at runtime)
* Function attributes (testing for example)
* A graceful way to fatally quit the VM with a message (for example, when function is not found)
* Ensure that infix operations work for everything in the first argument
* Report parsing errors (depends on next chomp release)
* Basic build tool
* Tuple stdlib
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
* Pattern matching
* Benchmarks
* Custom garbage collection
* FFI (at least C)
* Lightweight concurrency (CSP? Actors?)
* Macros
* dare I say tracing JIT?

### Optimisation ideas

Reducing bytecode footprint:
  * Don't need to include module name in every function name (put it in a header section)

Interpreter:
  * Direct threaded VM
  * Register windowing

General:
  * Remove unnecessary `mov` instructions.
  * Linear-scan register allocation in the compiler
  * Load all code statically at boot-time
  * Remove closure overhead when can be avoided
