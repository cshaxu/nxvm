# T540 S86 Whole Electrical Boundary

## Intake And Executor Confirmation

Baseline is accepted S85 P2 `04b7afa7d`. The executor confirms CURRENT's
complete electrical receiver, not a single-command repair. NXVM-only source
authorization and the existing embedded-artifact exception apply. No Shared,
MyNES, owner INI or external master change is admitted.

Read machine_board.c's complete parity/D4/speaker/absent-memory groups and
d4_memory.c. Remaining electrical callbacks still recover board state from
Core; their registrations must move together at construction and cold reset.
The actual board already borrows opaque Core for bounded routing/signals.
Absent-memory callbacks already own their individual window and retain that
distinct context; they must not be forced into a board-context callback.

## Complete Receiver And Decisions

- Nine public operations: configure planar parity, configure D4 platform,
  report planar parity fault, clear/report D4 IOCHK, observe D4, observe
  speaker, configure absent memory and observe planar parity.
- Their parity/D4 NMI, Port-B status/read/write, refresh edge, failsafe,
  speaker source/gate/refresh/output and timer programming helpers, including
  XT speaker setup/update and after-PIT-reset registrations.
- D4 memory configure/reset, setup/control memory callbacks, parity fault
  and write observer, their routes and every caller.
- The one neutral RAM-resize admission/allocation/reset operation. Existing
  board wrapper rejects configured planar parity with nonzero memory_bytes.
  Preserve this exact predicate through an optional admission callback in
  the existing copied attachment. Core calls it before mutation; no board
  state or profile name enters Core. Remove the old wrapper and private
  `_core` entry rather than maintaining two resize paths.

NULL handling, eligibility, NMI delivery, reset order, refresh polarity,
fallback routing, parity rollback and timing classifications must remain
unchanged. Constructor-only private association and unrelated white-box
fixtures are not accepted by this receiver; their removal is the next whole
boundary before actual Core/flat IBM-PC relocation.

## Implementation And Source Review

The RAM operation now has one implementation in neutral machine.c. The board
wrapper and private `_core` declaration are removed. Board supplies its exact
prior parity predicate through optional attachment memory_admission; Core
invokes it before allocation or mapping changes, preserving the existing
stopped/frozen operation checks. No mirrored board data or getter is added.

All 28 electrical operation/helper definitions now consume the board handle,
including speaker, parity and D4 callbacks at construction and cold reset.
D4 memory drops the private Core header and uses board state directly;
its bounded memory-route transaction still executes on board->core. Every
affected operation caller uses the actual constructor output. Absent-memory
routes retain their individual window context; Core-only NMI/reset/A20 signal
callbacks retain their genuine Core context.

Whole-definition comparison against baseline confirms all 28 bodies after
the explicit handle/context substitution, bounded Core-operation receiver
change and three configuration NULL guards. D4's whole file has only this
owner migration and removal of its private Core include. There is no waveform,
deadline, IRQ polarity, NMI latch or timing-grade change. Source/test numstat
currently covers 26 tracked paths: 375 additions, 353 removals, net +22.
Nine are production paths; seventeen are test/fixture paths. The small net
increase supplies explicit borrowed handles and the one optional admission
contract, not another state copy or production path. Final counts remain
subject to complete delivery review.

With the four existing gate updates included, tracked code/test/build-script
numstat covers 30 paths: 455 additions, 361 removals, net +94. The additional
72 net gate lines guard the complete migrated class, not another runtime
mechanism. Counts exclude documentation and generated/product artifacts.

The existing D4 route gate now checks all 28 complete definitions, the eleven
public/configure/reset caller forms, private-header exclusion and the sole
Core RAM operation/admission call. Its positive check and 32 injected
negative copies pass. The temporary copied-source probe is removed after
verification. The DMA authority gate checks both construction and cold-reset
refresh registrations with board context; its positive check passes.

The first full x64 unit run executed all 470 cases: 469 passed, one failed
in 249.82 seconds. The parity-publication rollback fixture still expected
the old Core callback identity after the route owner changed to board.
That assertion now checks the actual board identity; the original rollback,
parity allocation and publication assertions remain. This was a test migration
omission, not evidence of a production fault. The complete x64 rerun passes
470/470 in 272.97 seconds. Its subsequent specialized gate run rejects one
stale absent-memory receiver literal in verify_a20_fallback_routes.cmake.
The route still publishes through the same bounded Core operation, now on
board->core. The gate is updated to require that actual receiver; all other
A20/fallback checks remain. A copied-source positive and deliberate old-owner
negative both pass, bringing the boundary negative total to 33. The next
x64 specialized target run finds the same
obsolete Core receiver in the PIC/CPU refresh notification check. It now
requires core_machine_cpu_bus_refresh_pulse(board->core) and rejects direct
external-cycle invalidation regardless of argument. Its positive/negative
copy check also passes; the complete copied-source negative total is 34.
The final complete x64 specialized target run exits successfully. Its final
output is authoritative; the earlier x64 gate log retains the first failure
and is not misrepresented as the final passing run.
The x86 full suite passes 470/470 in 76.31 seconds; its complete specialized
target also passes, including the 402-row direct-compilation matrix.
No repeated boot round
is used to hide either failure.

Two sandbox Ninja invocations remained alive without children. A separate
one-command cmd/exit probe reproduces that child-launch wait inside the
sandbox and completes immediately outside it. After checking the exact process
trees, only this receiver's waiting gate, x86 build and probe were stopped;
the progressing product build was untouched. The same gate and x86 full
verification commands then ran outside the sandbox. This changes no product,
test selection or verification requirement.

Default and XT dual-width products, their real-INI boot verifiers and neutral
receivers have rebuilt successfully. Both default widths execute neutral
Core and reach the DOS prompt through the unchanged owner INI. Both XT widths
execute neutral Core and reach the running installer on their single run.
Both AT and Model-40 widths have rebuilt, execute neutral Core and reach the
running installer on their single unchanged-INI run. All eight rows pass;
none of these boot rows was repeated. The neutral marker is
`M5:T540:S69:NEUTRAL-LINK:OK`; boot markers retain the actual INI identity
and either `dos-prompt` (default) or `installer-running` (other products).

## Remaining Task Convergence

S86 consumes the complete electrical/RAM-admission class only. The current
machine_board.c still uses the private Core-to-board association in constructor
allocation, wiring and success publication. Direct private-board fixtures also
remain to be classified and migrated as one whole boundary. A current
`rg -l -g '*.c' -g '*.h' -- 'machine->board|machine\.board|core_machine->board' test/app-nxvm`
query identifies 91 candidate files; this is an intake count, not proof that
every hit is a private Core access or belongs in Shared. The driver also has
an independently named machine->board borrowed handle, so type/owner review
must distinguish it from Core's private field. Board state still includes
the private machine.h transitively: neutral construction values/seams and
the refresh-notification declaration must receive a genuine public or
test-local owner before that include is removed. Those are the next
receiver, not further one-function electrical subtasks. After their removal,
the actual neutral Core and flat IBM-PC source/test/build movement must occur.
src/x86 currently still has only chips, debug and xasm32. Independent neutral
linkage alone does not establish the requested physical component layout or
close T540.

## Delivery Verification

CURRENT's complete S86 verification set passes: both full 470-case unit
suites, both complete specialized targets, 34 injected boundary negatives,
eight fresh product/verifier builds, eight neutral executions and eight
single real-INI boots. Documentation governance and local links pass;
`git diff --check` passes. Actual added/removed source, declarations, seventeen
caller/fixture paths and four gates were read before delivery. Physical
component extraction remains the T540 objective, not this acceptance claim.

Full units use `ctest --test-dir <tree> -L unit -j 8 --output-on-failure`
after rebuilding every executable in the actual unit inventory. Gate command
is `cmake --build <tree> --target verify-current-specialized-gates -j 8`.
Product trees build `vm-0-5-0540`, `vm-profile-floppy-boot-matrix` and
`core-machine-neutral-link-smoke`. Each boot uses the existing adjacent INI
and a 180000-ms containment budget; only the semantic checkpoint is success.
The retained ignored trees/logs support the immediately next constructor
boundary. The owned root launch-probe log and copied-source negative tree
were removed after their processes terminated.

## Product Artifact Identity

All eight selected product targets and their boot/neutral verifiers rebuild
successfully. PE inspection confirms the named architecture, revision
`0.5.0540`, no compiler-debug sections, and modification time after all nine
changed production inputs. The build target's Release check also passes.
These hashes identify this S86 source delivery; they are not evidence of a
completed Shared physical move. MyNES, six Shared trees and owner INIs have
no normalized Git delta.

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x64.exe` | 1336055 | `AB30BE1B02C763663E0FC7673F48931195282EC4426CECD7E468901D5024B993` |
| `assets/nxvm/compaq-deskpro-386-model-40-1200k/nxvm_model40_0_5_0540_x86.exe` | 1505351 | `EA817BFF3A32EB3CCA3EA98430591BDCA6076A993A81A63820ACFB9DA5B8E3ED` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x64.exe` | 1352372 | `445271A07928B8D2EABCED37FF8FE49B978B0FAACBA361692740F9080558A3DC` |
| `assets/nxvm/default-pc-at-80386-1440k-hdd/nxvm_default_0_5_0540_x86.exe` | 1521666 | `2FF10821A83A019D9587F417E8A113184F0470684644FAB4F102EC8F1A96FD82` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x64.exe` | 1352340 | `CDDF434059A82219C4EA88CED3C9115ABCD05BCDC6001FF8D017BF2ED22C6028` |
| `assets/nxvm/ibm-5160-model-268-360k/nxvm_xt_0_5_0540_x86.exe` | 1521633 | `976D44B9452146B1A787F42E6591E6329456381BD5FB7829648D1F7F2CF4B096` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x64.exe` | 1352406 | `26DFA21061338E25B7EB9A82D99C4C2645940ACE30D927828FF19EFBEB0314D3` |
| `assets/nxvm/ibm-5170-model-339-1200k/nxvm_at_0_5_0540_x86.exe` | 1521701 | `CF8FCF7EBDA23411618699FAFF5B30F970B91187DFA2DC864E9467734AE439C8` |
