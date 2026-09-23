# xasm Capacity And Failure Contract

Replace xasm's unbounded public assembly/disassembly wrappers with one explicit
capacity and failure contract. The task covers the public `core_product_utils`
facade, both legacy xasm engines, and every retained caller.

## Scope

The public facade currently accepts only raw pointers. `aasm32` copies an
unbounded statement into a 256-byte local array; `aasm32x` counts unbounded
lines, allocates without checking the result, stores each line in a 256-byte
array, and emits into an unbounded output pointer; `dasm32` appends into an
unbounded caller string. The contract must make input length, input code bytes,
output capacity, result length, and failure publication explicit.

The intended owner is `core_product_utils`: it defines bounded public entry
points and owns validation/publication. xasm remains its implementation detail;
debugger and VM firmware callers migrate to the bounded facade. Existing raw
legacy symbols may not remain a public bypass after migration.

## Non-goals

- Expanding assembler/disassembler opcode support, changing instruction
  semantics, or replacing xasm with a new assembler.
- Generic string or allocator frameworks, platform-specific behavior, debugger
  UX redesign, firmware content changes, or CPU execution work.
- Treating a local `strcat` substitution as a contract repair.

## Proposed Subtasks

### S1 - Complete contract inventory and API decision

Record every public entry point, direct caller, fixed internal buffer, output
publication site, allocation/failure site, and compatible capacity bound.
Choose the exact bounded facade signature and legacy-symbol disposition; define
failure atomicity for every caller. Stop before source migration if the
inventory shows a new behavior family outside these three current callers.

### S2 - Bounded facade and caller migration

Implement the S1 contract at the public facade and xasm engine boundary,
migrate debugger, firmware, lifecycle, and test callers, and remove public raw
capacity bypasses. Failure must leave caller outputs unchanged and return an
explicit status; successful output lengths must be bounded and exact.

### S3 - Boundary regressions and closure

Add deterministic regressions for input at/over the bound, insufficient code
and text output capacity, malformed input, and assembler allocation failure.
Prove failure atomicity with sentinel outputs, retain representative successful
assembly/disassembly/paragraph behavior, rebuild the task artifact, and close
with the full gate.

## Acceptance And Exit

Every exported xasm operation has explicit capacities and failure result; no
caller uses an unbounded raw entry point; all known engine writes are guarded
by the named bounds; and failure publishes neither partial code nor partial
text. Every caller has a migrated capacity and regression. New unrelated opcode
or syntax gaps transfer separately.
