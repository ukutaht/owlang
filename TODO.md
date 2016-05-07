### Big list of TODO items

* Allow equality checks
* Implement `assert` and `assert_eq` in stdlib
* `if location == 0` test in call is not enough. Functions can be defined in location 0;
* Add `!` (not) operator
* Add short-circuit && and ||
* Add `nil`
* Use static libraries for deps
* Ensure `nil` is returned when no else branch in if
* A graceful way to fatally quit the VM with a message (for example, when function is not found)
* Make `if` work with booleans (no special casing for >, <, == and friends)
* Ensure `else` branches work properly
* stdlib testing framework
* Tuple stdlib (bounds check for `nth`)
* Vector stdlib
* IO stdlib
* Public/Private distinction
* Module constants
* Generalise `if` to `cond`
* Dynamically grow the stack
* Dynamically grow the code array
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
* Benchmarks
* Check that functions exist at compile time
* Dead code analysis
* Higher-order functions
* Closures (oh boy)
* Pattern matching
* Direct threaded VM
* Register windowing
* Dynamically grow register array
* Linear-Scan register allocation in compiler
* Custom garbage collection
* FFI (at least C)
* Lightweight concurrency (CSP? Actors?)
