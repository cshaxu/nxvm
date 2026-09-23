# T527 S5 - Common Debug Migration Ledger

## Frozen Legacy Universe

The S5 receiver is the whole retained DOS-style Debug grammar, not merely the
`DEBUG` product command. Its original command implementation now resides at
`src/common/debug/command.c`; it is the source-preserving migration baseline,
not a replacement or a reduced reimplementation. `common/debug/debug.c` is
now only the public lifecycle facade; it delegates every line to this sole
command owner. Raw line ingress, formatted output and direct Debug file
handles are removed.

| Family | Commands or continuation | Required receiver |
| --- | --- | --- |
| Main command dispatch | `? A C D E F G H I L M N O Q R S T U V W X` | `common/debug` provider |
| Nested line continuation | `A`, `E`, `R`, `V`, `XA`, `XE`, `XR`, main/`Q` | `common/debug` state machine |
| Paused target state | registers, real/linear memory, ports, code defaults and watchpoints | bounded `common/machine` operations |
| Break and trace policy | real/linear breakpoint, trace count and target-completed pause fact | `common/debug` submits; target enforces |
| Lifecycle escape | `G`, `T`, `XG`, `XT`, `Q` | copied ordinary request selected by session |
| File grammar | `N`, `L`, `W` | injected product file service; no Debug-owned handle |
| NXVM recorder | raw instruction recording start/stop | explicit `vm/product` optional capability |

## Old-To-New Route Accounting

| Legacy route | Disposition |
| --- | --- |
| `core_debugger_run()` raw read loop | Replace only its line ingress/output loop; session provides every line to the migrated command provider. |
| `STD_FGETS`, `STD_PRINTF`, `STD_FOPEN` in Debug | Delete; bounded copied result text/prompt and injected product file service. |
| `core_debug_target` and `core_debug_access` wrappers | Delete; no callback-table facade survives. |
| `vm_machine_debug_target()` | Delete; VM remains only common-machine driver mapping. |
| Debug execution policy | `common/debug` submits a copied bounded plan; the VM common-machine driver owns its active plan, Core run budget and actual retirement result. |
| `t_debug` raw recorder fields | Move to `vm/product` optional recorder ownership. |
| `common/xasm32` | Retain as the sole assembly/disassembly dependency. |

## Required Result Invariants

- Session owns ordered line ingress; Debug never asks the native Console for a
  line and owns no second FIFO.
- Common Debug knows only public `lib/types`, `common/xasm32`, and
  `common/machine` values. It has no Core, VM, UI, host or storage include.
- A Debug operation is copied and bounded; target access is rejected unless
  the common-machine paused lease is valid.
- Product-owned file service decides path policy and uses storage; Debug only
  issues bounded copied file requests.
- The final source/test sweep contains no legacy parser, target-table or raw
  Debug Console route.

## Current Boundary Conversion

The preserved `command.c` now sends register, real/linear-memory, port,
code-width/base and watchpoint operations through a freshly acquired
`common/machine` paused lease. Its `G`, `T`, `XG` and `XT` handlers install a
bounded execution plan while paused and return an ordinary session resume
request instead of blocking in a target callback. The target owns retirement
accounting and requests the resulting pause; Debug neither decrements an
instruction counter from an observation callback nor injects a pause request.
The paused machine fact returns through the session-owned CLI observer to the
same result-buffer writer, so `T`, `XT`, and `XG` retain their result text
without a direct Console write or a Debug queue. `XT` receives copied memory
access values; it does not ask Core to print them. `XW` queries its one Core
owner through the same lease and prints the original active address text,
rather than retaining a second watchpoint cache. `A/E/R/V/XA/XE/XR` record
their typed continuation and consume the next session line rather than
performing a nested read. `N/L/W` use only the injected file service.

Focused `common-debug`, `common-session`, and VM adapter smoke tests cover
the trace/fact return, `XT` memory-access output, `XW` state query, and
session plan delivery. The full 296-test unit label completed with exit status
zero after the final change; all four S5 static boundary gates passed.

## xasm32 Extraction Audit

The source-preservation baseline is the parent of S2 extraction commit
`b3684432`: `src/core/debug/xasm32/{aasm32,dasm32}.c`. S2 was not a
textually-neutral move: Git reports 64% similarity for `aasm32.c`
(`+2573/-2569` lines) and 50% for `dasm32.c` (`+2184/-2181` lines).

Its observed change categories are: include relocation, legacy root types to
`lib/types`, `TYPE_*` trace/operand vocabulary to `XASM32_*`, and replacement
of the former Core-local text/trace helper boundary. The instruction tables,
parser and encoder/decoder bodies therefore require source review against that
baseline; the line counts alone do not prove semantic equivalence. This ledger
does not claim otherwise.

The current S5 working-tree delta does not alter assembler or disassembler
logic: each implementation changes only its default-private header include
from `xasm32_internal.h` to `xasm32.h`. The new header has the same Git object
hash as the former header at `HEAD` (`7213913a7241c7b9b3cafdcef56702ced2218404`).
`common-xasm32-smoke` and `common-xasm32-contract-smoke` verify the retained
implementation and public contract.
