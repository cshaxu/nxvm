# T333 S2 Interactive Input Repair

Against the S1 inventory, the debugger now has one private
`core_product_debug_read_line` owner boundary. All 43 retained debugger input
callers use it. A failed nested read returns before normalization, scanning,
assembly, buffer indexing, or guest-state mutation; a failed main-prompt read
breaks before parse and exec.

The VM Console retains its separate product owner and uses one private
`vm_product_console_read_line` check. Its main loop breaks before `parse` and
`execute` when no input line is available.

Both main owners preflight their argument-array allocation. The debugger clears
target/input-provider bindings on allocation failure, and its shared finalizer
frees then nulls `arguments`; the Console finalizer likewise frees then nulls
its `arguments`. No public header or ABI changes.

Static sweep after the repair finds `STD_FGETS` only inside the two private
owner helpers for these modules. The existing MinGW build successfully rebuilt
`core-product-debug`, `vm-product`, and `vm-product-console-lifecycle-smoke`.
Runtime EOF and allocation-failure proof remains S3.
