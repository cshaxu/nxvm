# T559 S5 System-Instruction Boundary Review

## Baseline And Qualification Limit

Reviewed source baseline: `24d4ce370`. S5 executor delivery is complete and
pending coordinator acceptance. I4--I6 have explicit system dispositions,
not acceptance as implemented 486 hardware semantics.
No production source, Shared-six, MyNES or product artifact has changed.

## Primary Source

Intel486 Family Programmer's Reference Manual (1992), external archive file
`intel-486-family-programmers-reference-1992.pdf`, SHA-256
`d86207aaebd9b10464f2058efc61559e6141d3b072db100e89bfe17c8187e143`.
Reviewed printed pages 10-9--10-16, 26-171, 26-172 and 26-281.
This is manual-derived analysis, not imported implementation code.

## Actual Owners And Whole-Family Disposition

| Family | Required effect | Existing implementation and remaining work |
| --- | --- | --- |
| INVD, 0F08 | Invalidate internal cache and signal external cache flush; no flags change; privileged outside real mode | Metadata rejects it and dispatch has no implementation. No cache-line owner or external cache-operation contract was found. Do not admit a success-only operation. |
| WBINVD, 0F09 | Invalidate internal cache, signal external writeback, then external flush; no flags change | Metadata rejects it; the existing TODO handler raises UD. This is an implementation gap, not the real 486 behavior. External writeback/flush ordering belongs with the missing system implementation. |
| INVLPG, 0F01 /7 | Invalidate the TLB entry for the effective address, without an ordinary target-data load; register form is invalid | Metadata and lexeme form validation reject /7. Paging performs direct PDE/PTE walks in `_kma_prepare_physical_linear()`; its temporary translation record is not a TLB. Full address/segment/privilege/fetch behavior must be implemented with the actual invalidation owner. |
| MOV TR3--5, 0F24/26 | Transfer cache-test data/status/control; TR3 addresses fill/read buffers through TR5, TR5 commands perform cache access | Decoder rejects TR0--5. Adding passive fields would not implement the specified operations. TR5 read behavior requires explicit handling: section 10.7 states that TR5 cannot be read; it is not an ordinary round-trip register. |
| MOV TR6--7, 0F24/26 | TR6 write performs TLB write/lookup; TR7 conveys data and lookup outcome | Decoder selects plain TR6/TR7 fields and both handlers only copy values. 486 currently inherits this 386 baseline. It is not evidence of a functioning 486 TLB test interface. |

The 486 TLB also differs from the 386 in permitted testing with paging enabled
and replacement behavior. Cache testing involves four-word fill/read buffers,
sets, ways, tags, valid and replacement state. No such state owner exists in
the current CPU. Segment-descriptor caches are unrelated and must not be
repurposed as data caches or TLBs.

The finite receiver matrix remains: both cache opcodes; INVLPG memory/register
forms with 16/32 addressing and segment overrides; both MOV-TR directions and
all eight register indices; real/CPL0, CPL1--3 and VM86; operand/address-size
prefixes; LOCK; every required instruction-byte fetch failure; no target-data
access for INVLPG; flags/register preservation and restart behavior. Current
retained-form tests cover only the subset explicitly described below.

## Design Constraint Found During Review

The S1 ledger forbids a new unsupported-instruction fallback, invented exception
or dummy cache. Consequently, changing legal 486 TR6/TR7 forms to unconditional
guest UD would not fix this gap. Likewise, reusing an internal bus error as an
invented unsupported-instruction result would not implement the instruction.
Do not change production behavior merely to make an unavailable-effects test
pass. The existing Core machine profile gate still rejects profiles above
80386; isolated CPU tests are not production admission.

The [system receiver](../../history/M5-T560-complete-486sx-cpu-proposal.md) explicitly receives
the complete per-form implementation and proof, including the decode/privilege/
address work that cannot yet be connected to real effects. This is the S1
ledger's permitted unavailable-owner disposition, not a new scope reduction.
The S5 packet was reconciled after inspection to forbid falsely claiming the
passive 486 inheritance fixed. This record neither silently retires legal forms
nor marks their current UD/passive-copy behavior architecturally correct.

## Retained-Behavior Regression

`test/core/chips/cpu/cpu_test_register_smoke.c` directly executes CPU handlers:

- 386 TR6/TR7 reads and writes, real/CPL0, 16/32-bit code and all combinations of 66/67;
  values deliberately use the upper word to detect accidental 16-bit transfer.
- Full CPU-state equality except the intended destination and instruction
  pointer, unchanged flags and no memory writes.
- Complete lexeme length and rejection of each truncated preview.
- Invalid register indices (386 TR0--5, 486 TR0--2), memory forms for every
  index, and LOCK forms in both directions, with actual real-mode UD delivery
  and state/frame checks through the existing CPU fixture.
- Existing MOV-TR CPL1--3/VM86 rejection on both profiles, with actual GP(0)
  delivery to a valid handler and inspection of the saved fault frame. VM86
  uses the real TSS ring-0 stack switch, not an eventual shutdown assertion.
  Checking the existing nonzero-privilege rejection does not qualify TR5 read
  behavior or any other legal-mode cache test effect.
- Every byte of a prefixed 386 transfer in both directions and both test
  registers has an injected execution-fetch failure; full CPU state remains
  unchanged and no memory write occurs. This is distinct from lexeme truncation.

The test deliberately does not assert UD for legal 486 TR3--7 register forms.
It does not prove the unavailable 486 hardware effects or INVLPG effective-
address behavior; those remain in the receiver matrix.

Both original Ninja/ccache graphs built this target and its focused CTest
passed. After whole-batch review, the complete repository unit suite passed
596/596 on each width: x64 31.15 seconds, x86 32.76 seconds. Logs are
`build/t559-s5-x64-unit.log` and `build/t559-s5-x86-unit.log`.
The suite includes Lib 51, Emulator 21, shared x86 15, Core 370 and all five
Apps; the existing machine CPU profile-gate test rejects 486 construction.
Three desktop-labelled unit cases passed on each width. No manual desktop/RDP
or external guest-boot validation was performed; external boot remains a T gate.

## Size, Artifacts And Review

Production +0/-0/net0. Test C/CMake +242/-0/net+242: 241 lines in the new
CPU-owned regression plus one registration. Counts exclude manifests/docs and
use Git numstat plus the new file's line count. There is no new production
helper, API, mutable state, handler or alternative execution path. Test-local
helpers group retained transfers, rejection, privilege and failed fetch.

The eight S4 0559 PC EXEs are retained byte-for-byte: all executable inputs,
build declarations, source, INIs and external assets are unchanged. Rebuilding
them would not validate this test-only addition. No MyNES or Shared-six file
changed. Core manifest and documentation governance pass; diff whitespace is
clean. Actual review inspected the new test, registration, manifest delta,
packet reconciliation, ledger and complete system receiver, rather than
inferring implementation from a green suite.

## Similar-Path Inspection

Inspected CPU metadata, lexeme ModRM validation, MOV-TR handlers, test-register
decoder, WBINVD TODO, paging translation and exception-delivery paths. Searches
for `tr6`, `tr7`, test-register opcodes, cache/TLB state and existing tests found
retained 386 timing recipes but no direct complete TR transfer/form matrix.
The new test complements those recipes; it does not replace timing qualification.
Remaining legal 486 hardware operations are all mapped above to one receiver.
