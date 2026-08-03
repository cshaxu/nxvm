# M5 T123 S1: Explicit Debug Target Binding

The public `core_product_debug_*` access surface now receives a
`core_product_debug_target` explicitly. The target and input provider are
owned by the caller's debug context during `core_product_debug_main`; the TLS
target scope and its non-restoring leave operation are deleted. Focused target,
VM adapter, and unified pause-boundary tests pass.

The retained parser still uses a thread-local pointer to its caller-owned
command context. It is internal to the legacy parser, does not select a VM
session or target, and remains an explicit T127 audit item rather than a
claim of parser reentrancy.

Developer artifact: `build/output/nxvm_0_5_0123.exe`, SHA-256
`E8AE2EEF5009173E0B7E9DD4CAD92F679DCC11D02BD5841D9D1BD3BF235DCF95`.
