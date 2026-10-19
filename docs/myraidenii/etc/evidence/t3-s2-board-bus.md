# T3 S2: Board Program Bus Evidence

## Boundary And Sources

This batch implements only B01--B03/B09 from the accepted T3 ledger. The
board consumes two caller-provided 512 KiB program lanes, copies and
interleaves them, creates one neutral V30 Core Machine, and installs only its
own program-window and bank-select routes. It does not load a ROM file, own a
host clock, execute an App loop, model COP/inputs/interrupts/sound/video, or
publish a frame.

The selected MAME revision is re-read as L2 evidence only at
`c1425a1af00b88aa07477b1b23a30ed5e5f856ff`; its fetched source hash remains
`fdde57f6e9024657f2a9830864523305139562229750b87c967e597324cdb26a`.
It identifies the two observed 128 KiB program-window source states and the
bank control's bit-7 selection. The temporary 184646-byte source file was
fetched under `build/myraidenii-t3-s2/`, hash-verified, then removed using
`Clear-Build.ps1` preview plus reviewed apply. No source, table or ROM byte was
copied into this repository. The exact bank control remains L2 until a stronger
board observation is available.

## Ownership

| Obligation | Owner and mechanism | Deliberate non-owner |
| --- | --- | --- |
| 16-bit lane interleave | `myraidenii_core_board_create()` copies even then odd bytes into one local immutable backing | Lib, Emulator and X86 Core do not know ROM file roles. |
| Fixed upper program image | Existing `x86_core_machine_register_immutable_rom_mapping()` at 0x40000--0xfffff | Board does not duplicate immutable-ROM registry behavior. |
| Runtime 128 KiB window | One Board-owned standard memory provider at 0x20000--0x3ffff | Immutable mapper remains construction-only. |
| Bank select | One write-only Board route at 0x006cb; bit 7 chooses the two L2-observed source ranges | No public setter or second bus path exists. |
| Remaining B02 RAM | Existing Core ordinary RAM backing | Board does not recreate RAM, memory query or reset semantics. |
| Reset | Attachment resets only `bank_selected`; Core continues to own its normal RAM/lifecycle reset | No second reset state machine. |

## Test-Minimum Review

`test/x86-core` already owns generic immutable-ROM copying, provider priority,
ordinary RAM, cross-range routing, V30 execution and Machine lifecycle. The
S2 test is therefore one Board-only smoke, not a copied Core test family. Its
single table-driven RAM loop proves that the selected Raiden II B02 regions do
not accidentally become a Board provider; its remaining assertions prove only
the selected V30 profile, new lane order, both dynamic-window endpoints,
fixed/static map, write-discard, bank selection and reset default. Invalid
Board config belongs in the same file because it is the same public Board
construction contract.

### Core Test-Economy Audit

Before adding this Board test, the current Core corpus was inspected rather
than using its line count as a reason to duplicate behavior. `src/x86-core`
contains 37,820 C/H lines; `test/x86-core` contains 79,377 C/H/CMake lines
across 513 files. Its largest files are CPU form/timing matrices and Machine
timeline qualification, not copies of the Board route. No exact duplicate C
test file was found. The existing generic owners cover immutable-ROM mapping,
memory-device registration/transactions, ordinary RAM, CPU execution and
Machine lifecycle. This batch therefore retains one 102-line Board test only:
it covers new address ownership and board state that none of those owners can
observe. Future T3 batches must extend an existing owner when it already
proves the same public contract; a new test family needs a newly-owned board
behavior and a recorded reason.

Focused x64/x86 Ninja builds and `myraidenii.core/board-bus-smoke` pass. Full
repository-only unit qualification also passes: both `gcc-x64-release` and
`gcc-x86-release` complete 687/687 `unit` tests. The repository cleanup test
needs a temporary Windows junction to prove that cleanup refuses links; it was
run in the governed build environment that permits that fixture, rather than
weakening the test for a restricted sandbox. The Lib boundary self-test was
replayed on both widths after its test-owned probe cleanup correction, proving
that a rejected negative case cannot contaminate its next run.

MyRaidenII has no source/test corpus manifest owner yet, so this batch updates
no manifest. Its CMake registration, source/test mirror, documentation gate
and focused units are the applicable local structural evidence.
