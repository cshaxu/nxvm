# T333 S3 Interactive Input Proof

T333 uses two focused owner-local smoke targets because the CMake dependency
audit correctly rejects a target that mixes `core/product` and `vm/product`
production sources.

- `core-product-debug-input-failure-smoke` compiles only the core debugger
  sources with a source-target-local `STD_MALLOC=test_debug_input_malloc`
  definition. It proves main EOF and reuse, deterministic allocation failure,
  and EOF in the assembly, single-value, and register-edit nested prompt
  classes. The stub target observes zero register writes.
- `vm-product-console-input-failure-smoke` compiles only the VM Console source
  with `STD_MALLOC=test_console_input_malloc`. It proves EOF/reuse and
  deterministic allocation failure before command parsing.

The ordinary `core-product-debug` and `vm-product` libraries are not compiled
with either definition. The two local allocators exist only in their respective
test executables; no product header, public ABI, or process-global allocator
state changed.

Expected focused markers are `M5:T333:S3:DEBUG-INPUT:OK` and
`M5:T333:S3:CONSOLE-INPUT:OK`. Task closure records the current-gate result.
