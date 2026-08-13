# T333 S1 Interactive Input Inventory

## Baseline And Method

This inventory is against `68745ea6`, before T333 production changes. It uses
the direct `STD_FGETS` caller set in `src/`, then classifies the retained VM
Console and core debugger readers by their enclosing prompt owner and the work
performed immediately after a successful read. `src/type.c` is only the C
runtime wrapper and is not an interaction owner.

The complete retained product-reader set has 44 direct calls: 43 in
`src/core/product/debug/debug.c` and one in `src/vm/product/console.c`. No
other product source calls `STD_FGETS`.

## Owner And Variant Inventory

| Owner | Direct reader variants | Current post-read boundary | Required failure outcome |
| --- | --- | --- | --- |
| VM Console | `vm_product_console_main` (one main prompt) | Immediately enters `parse`, then `execute`; its `arguments` array is allocated by `vm_product_console_initialize`. | End the Console loop before parse/execute; finalize storage and clear the context pointer. |
| Core debugger main loop | `core_product_debug_main` (one main prompt) | Immediately enters `parse`, then `exec`; its `arguments` array is allocated in the main entry point. | End the debugger loop before parse/exec; free and clear owned storage before return. |
| Core debugger nested assembly prompts | `aconsole`, `xaconsole` (two loops) | Lowercase/assemble or strip newline/assemble; `xaconsole` indexes `strlen(buffer)-1`. | Leave the nested prompt without using its buffer; return to the debugger main loop. |
| Core debugger single-value prompts | `e`, `xe`, `v` (three readers) | Lowercase/scan and optional memory write; `v` indexes `strlen(buffer)-1`. | Return from the command without scanning, writing, or indexing the buffer. |
| Core debugger register-edit prompts | `rscanregs` (14 direct readers) and `xrscanreg` (21 direct readers) | Scan the local buffer then conditionally mutate a register/flag. | Return from the command without scan or mutation. |
| C runtime facade | `STD_FGETS` in `src/type.c` | Direct `fgets` wrapper; no interaction state or policy. | Retain its ordinary C return contract; do not turn it into a product-policy owner. |

The debugger is entered only through the VM session Console adapter at
`vm_session_machine_debug`, but the debug product owns its own nested prompts
and context lifetime. The VM Console owns only its command-loop state. These
are distinct product owners, so S2 must not create a cross-module generic input
framework or a reverse `core -> vm` dependency.

## Shared Mechanism Contract

Within each owner, a failed read has one result: no line is available and no
consumer may inspect, normalize, parse, scan, execute, or mutate based on the
destination buffer. A main-prompt failure terminates that owner's loop; a
nested-prompt failure returns from that command to the retained debugger main
loop. Allocation is preflight: a null `arguments` array prevents entry into a
parser. Finalization is the publication boundary: after every exit the context
does not retain an owned `arguments` pointer.

S2 must route all 43 debugger direct reads through one private debugger
read-result boundary, with each caller taking its specified local outcome. The
VM Console has one reader and a distinct context/lifetime owner; it may use a
narrow private check rather than a shared abstraction. This distinction is
semantic (nested debugger commands return while their parent interaction
continues), not an excuse to leave an equivalent unchecked reader.

## Existing Coverage And S3 Plan

`tests/products/nxvm_console_lifecycle_smoke.c` and
`tests/platform/vm_session_startup_failure_smoke.c` redirect stdin but always
provide an `exit` command; they do not exercise EOF. No focused caller of
`core_product_debug_main` exists. No existing controlled allocation-failure
seam covers the direct Console/debugger `STD_MALLOC` calls.

S3 therefore needs deterministic stdin-redirection probes for VM Console EOF,
debugger main-prompt EOF, and one representative of each debugger nested
semantic group (assembly loop, single-value command, register edit). It must
prove no parser, command, scan, or guest-state mutation occurs after failure,
and that the public context is reusable with `arguments == STD_NULL` after
return. The executor must select a product-local controlled allocation-failure
seam only after showing that it neither becomes process-global nor widens the
production public ABI; otherwise stop for coordinator direction.

## S2 Boundary

S2 includes only `src/core/product/debug/debug.c`, `src/vm/product/console.c`,
their directly affected tests, and minimal CMake wiring needed for those tests.
It does not include xasm capacity/failure semantics, which remains separately
tracked in `docs/states/TODO.md`.
