# T329 S6 Task LDT Image Evidence

## Scope

S6 closes only the protected task-transition use of a non-null incoming LDTR
and `TI=1` incoming task-image selectors. It does not close ordinary `LLDT` or
`LTR`, task paging/debug state, arbitrary task chains, or non-task LDT use.

## Transition Contract

Both 16-bit and 32-bit task transition paths now use the same ordered
boundary:

1. read the complete incoming TSS image;
2. prepare its LDTR from a GDT-resident, present LDT descriptor, or retain a
   null invalid cache;
3. resolve incoming `TI=1` code/data/stack selectors against that prepared
   cache, rather than the outgoing task's LDTR;
4. preflight the target stack, outgoing TSS, and descriptor writes; and
5. commit outgoing state, busy descriptors, incoming caches, LDTR, TR, and
   `CR0.TS` together.

`_s_task_prepare_ldtr` and `_s_task_read_selector` are task-preflight helpers,
not a second ordinary selector-loader API. The existing task cache builder
records system type for both TR and LDTR. The task-specific validation helpers
therefore retain their existing `#TS`/`#NP` classification while accepting a
proposed LDT only within the task plan.

## Owner Smoke Matrix

`core-machine-task-switch-smoke` executes:

- 80286 and 80386 16-bit TSS images with a valid non-null GDT LDT descriptor
  and `TI=1` CS/SS/DS/ES images; and
- an 80386 32-bit TSS image with the same LDT-backed CS/SS/DS/ES/FS/GS image,
  proving the cached LDTR selector, base, limit, and system type after the
  transition.

The 32-bit matrix also creates each controlled preflight failure: an LDT
descriptor of the wrong type, a non-present LDT, an LDT whose limit excludes a
required selector, a non-code LDT CS selector, and a non-writable LDT data
selector. With no IDT installed these producer faults reach the established
terminal `#DF` observable boundary. Each vector proves that the outgoing TSS
image remains zero, old and new busy descriptor bytes remain `0x8b`/`0x89`,
TR remains the outgoing task, and LDTR remains invalid; no partial transition
is claimed.

The retained task-switch smoke executes all S1--S5 vectors as regression
coverage. The target is already a current-gate executable with target-local
strict GCC options.

## Transfer

S7 retains task switching with paging and debug state. Ordinary LDT
instructions and non-task LDT selector breadth remain in their explicit
processor-control and protection/privilege ownership boundaries.
