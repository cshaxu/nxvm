# T539 S16: Independent FPU Boundary

Baseline c0722284b. This is the admission review and implementation contract,
not completed verification. The active packet is in Current; the finite batch
is the three-file FPU row in the T539 inventory.

## Source Findings

The existing FPU owns integer-based partial 8087 arithmetic, stack/control/status,
BUSY/ERROR and completion intervals. Its only CPU dependency is compatibility
selection and a constant minimum_cpu metadata field. The machine embeds its
mutable layout; CPU reads profile directly; tests mutate BUSY, remaining ticks
and pending error. These are the coupling to remove, not reasons to rewrite
arithmetic or expand public diagnostic state.

Production consumers are machine construction/reset/destruction and copied
state, CPU ESC and WAIT, retirement timing and machine scheduler. Five existing
test files inspect internals: 8086 timing ledger, 80386 timing manifest,
retirement observation, FPU 8087, FPU interface S65 and their machine fixtures.
CPU/FPU profile closure and escape tests also remain regression owners.

## Ownership And Contract

- src/x86/devices/fpu owns one opaque instance and all existing FPU data.
  Its target depends only on Types. Public declarations use x86_fpu names;
  no compatibility typedef/header or second implementation remains in NXVM.
- Construction validates the existing NONE/8087/80287/80387 variants, allocates
  and resets once. Failure leaves no published handle. Destroy follows stopped
  execution; all API use is serialized by the existing machine execution owner.
  No FPU thread, lock, global registry or board pointer is introduced.
- CPU owns CPU/FPU pairing and instruction legality; the existing compatibility
  predicate moves there without changing its accepted combinations. The FPU
  decoder consumes only ESC bytes and its own variant. The unused minimum_cpu
  field is removed; shared FPU does not include CPU declarations.
- CPU still owns operand fetch/store and architectural faults. Preserve the
  original semantic update versus memory-failure ordering. No new transactional
  rollback of already committed instruction effects is implied.
- Scheduler supplies elapsed source-axis units through advance; the FPU owns
  the remaining completion interval. WAIT consumes that remainder once; CPU
  timing observes its wait contribution. Existing L2 model values and diagnostic
  ranges remain unchanged; relocation proves no full 8087/287/387 implementation.
- Debug uses the existing copied control/status/top/tags/error observation.
  Public profile observation serves real board/CPU consumers. Do not publish
  raw registers, test setters or last-command/range getters just for old tests.

## Test Migration And Failure Review

Chip-only tests cover independent instances, invalid construction, reset,
opcode metadata, arithmetic subset, unsupported values, stack/error signals,
all variant completion choices, remaining wait and exact diagnostic ranges.
Private range assertions belong to same-owner Shared tests. Board tests retain
CPU faults, legal/incompatible pairings, transaction opcode identity, memory
effects, deadlines and retirement time. Set a three-tick remainder by issuing
and advancing a real command; generate pending error by an unmasked stack fault,
not private field writes. Record every removed assertion's receiving proof.

The 80386 timing runner's MCP record now observes command identity and remaining
completion ticks through the public deadline, not private stored range fields.
Its result verifier must check that observed 80387 FADD interval against the
existing 19-tick L2 model. The exact 12..26 range assertion moves to the Shared
FPU test along with all other range cases; CPU ticks and grade are unchanged.

Machine construction must destroy a newly allocated FPU on every later failure;
reset reuses its immutable variant and only one instance. Inspect all early
returns after allocation and null-safe partial machine teardown. Existing
failure-injection tests and independent lifetime tests cover these contracts.

## Verification And Review

Current specifies exact receiving suites, artifact targets and bounded build
trees. Before closure: inspect normalized old/new arithmetic and timing code;
map original test cases; verify private-include/old-path absence and explicit
Types-only build; update exact manifests and negative boundary coverage; review
all four products and MyNES dependency inputs. Record source/test/build added,
removed and net lines separately from docs/manifests/artifacts. Positive growth
must be attributable to lifecycle or independent proof, not wrappers.

CPU private bus/PIC/transaction/diagnostic dependencies remain in the CPU ledger
row for its own complete extraction. S16 neither accepts nor transfers them.

The receiving x64 integration exposed a test observation-time mismatch: the
DOS keyboard test compares frozen VRAM with a cadence-cached frame. Coordinator
admits using the existing force-publication operation only after acknowledged
pause, preserving every cell assertion and production cadence. This narrow
receiving-test correction belongs to S16; it does not expand FPU behavior.

Model40 x64 receiving verification reached DOS setup detection but exceeded
the existing 90-second probe bound. The board still selects NONE for FPU;
the 24 original functions and its mechanical profile change were reviewed.
Coordinator admits one controlled diagnostic contrast using the probe's
existing --no-retirement-observation option after the initial width matrix:
same executable, firmware, INI, media, 90-second bound and installer-ready
predicate. This removes only the per-instruction diagnostic observer, not
guest work or CPU/device time. Retain the original failed run and do not
attribute it to performance until contrasting evidence supports that claim.
