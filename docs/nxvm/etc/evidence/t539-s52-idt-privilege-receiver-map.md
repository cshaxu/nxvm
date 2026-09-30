# M5 T539 S52 IDT And Privilege Receiver Map

## Intake classification

S52 consumes exactly two direct-private sources (665 lines at intake).  Their
cases are not all CPU-only: the receiver follows the observed mechanism, not
the old file boundary.

| Original family | Receiver boundary | Required proof |
| --- | --- | --- |
| Software `INT` privilege entry, interrupt/trap-gate IF result and 16/32-bit kernel stack frame | CPU-local IDT/privilege receiver | Descriptor selection, CPL transition, frame values, accessed bits and resumed handler state. |
| Software gate-DPL rejection, stack/code rejection and rollback | CPU-local IDT/privilege receiver | Terminal fault class/code, CPU/descriptor rollback and no partial frame publication. |
| External IRQ which bypasses software-gate DPL | Independent PIC board receiver | Real PIC source assertion, interrupt acknowledgement/ISR state and resulting privilege frame. |
| 80286/80386 protected privilege transition assembled from real-mode setup | CPU-local receiver if the existing CPU fixture can preserve every delivered-fault and handler-continuation observation; otherwise a public Core board receiver | Real-to-protected transition, user-to-kernel gate/frame behavior, delivered GP/NP error codes and stack atomicity. |

## Boundary decision before implementation

No new production interface, test-only accessor or second executor is
permitted.  CPU-local work uses the existing instruction fixture's explicit
memory bus, segment caches and diagnostic callback.  The external IRQ family
uses the actual PIC chip plus its existing board adapter and an explicit CPU
bus acknowledgement; it does not borrow a `core_machine` field. The protected-privilege
family may not be copied into a CPU fixture until a focused experiment proves
that its existing fault-delivery/continuation behavior is preserved; a failed
experiment keeps that family in one public board receiver rather than creating
a partial delivery shim.

The protected-privilege source is now confirmed as the public-board branch:
its direct reset fixture is replaced by the existing public freeze/reset and
atomic debug-register patch contracts.  The former fixture also hid a required
second run after a delivered exception.  The replacement makes that one
continuation explicit in an owner-local test helper, so the guest handler still
executes without a macro alias or private CPU borrow.  Focused x64/x86 runs
pass this branch.

## Implemented receiver map

| Original source family | Final receiver | Boundary proof |
| --- | --- | --- |
| Software `INT`, interrupt/trap gate IF behavior, 16/32-bit stack frames, DPL rejection and stack/code rollback | `cpu_idt_privilege_entry_smoke.c` | Links only `x86-cpu`; its explicit fixture owns memory, descriptor caches and fault observation. |
| External IRQ bypassing a DPL-zero software gate | `machine_idt_privilege_pic_board_smoke.c` | A single-controller `x86_pic` and the PIC board adapter drive CPU-bus `peek/acknowledge`; IRR/ISR are observed at the chip boundary. |
| Full-Core privilege delivery and guest handler continuation | `machine_protected_privilege_board_smoke.c` | Uses only public create/freeze/reset/debug-patch/memory/run/diagnostic operations; the owner-local second run executes the delivered handler. |

The two intake sources are deleted. No receiver reads `executor_cpu`,
`executor_memory`, `shared_pic_master` or `shared_pic_slave`.

## Similar-issue sweep

The active CMake lists and both source files contain no direct includer.  The
later `core_machine_interrupt_entry_smoke.c`, `core_machine_software_int_s50_smoke.c`,
VM86, outer-return and task-switch sources are intentionally outside S52 and
retain their named later packets.  No receiver is duplicated merely because
these related files also use IDT state.

## Verification

- Focused x64 and x86 runs passed for all three receivers.
- Repository-only `unit` passed on x64 and x86 (419 tests on each build).
- The current specialized gates passed on both build widths, including the
  T332 fixture-lifecycle inventory (39 owners) and the T344 historical-fixture
  shape inventory (103 direct sources).
- Documentation governance and `git diff --check` pass.  This S changes no
  production API, shared component, asset, INI or executable artifact.
