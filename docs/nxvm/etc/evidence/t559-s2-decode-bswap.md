# T559 S2 Decode And BSWAP Evidence

## Boundary And Source

This batch consumes I1 and the identity/decode part of I10 in the
[S1 ledger](t559-s1-instruction-inventory.md). The baseline is `406907f3f`.
The Intel 486 Family Programmer's Reference Manual (1992), BSWAP entry
26-35 (PDF page 533), defines `0F C8--CF` as a 32-bit register byte reversal
without flag changes. Source identity and hashes remain in S1/T558 evidence.
No external source code, firmware or media is imported.

The manual leaves the 16-bit result undefined. Rejecting that form is this
selected model's deterministic policy, not a claim that real hardware raises
#UD. This batch establishes instruction semantics, not a 486 timing class or
reset/system qualification.

## Implementation And Similar-Issue Review

- CPU appends `80486SX` after the retained profile values. Default remains
  80386. The CPU name and reset-code-base switch recognize the new identity;
  other reset signature/system semantics remain with their planned receiver.
- The existing metadata, lexeme scanner and dispatch table admit BSWAP only
  at the new minimum profile. One handler reads the opcode, chooses its GPR
  and reverses bytes; there is no second interpreter, copied state or API.
- Operand-size selection must be 32 bits. Address-size override does not
  change the operation. The existing LOCK and old-profile admission paths
  reject inappropriate execution.
- Core machine creation still rejects 80486SX. The exhaustive Board reset
  alias switch records the unadmitted profile with no alias; it does not
  enable a board. The FPU pairing predicate is unchanged and does not grant
  an external FPU to the selected SX identity.
- The broad `80386` comparison inventory is not mechanically replaced.
  Flags/CPUID, cache/TLB, complete reset and timing retain the S1 receivers.
  Existing profile values and all older machine choices remain intact.
- Two existing metadata matrix tests required their expected domain to
  include the new BSWAP profile. Their old execution/exception assertions
  are retained, not relaxed to accept BSWAP on 386.
- The owner approved exactly three Shared root preset target updates for
  0559. Shared-six source/test trees and MyNES inputs are unchanged.

## Direct Proof And Failure Investigation

The CPU-owned test executes all eight GPR encodings in 16/32-bit code,
with address-size override both absent and present. It compares the full
CPU result, including unchanged flags and other registers. It checks
truncated lexemes, invalid word/LOCK forms, old-profile metadata rejection,
real 386 #UD and instruction-byte bus failure before register publication.
The failure injection includes prefetch because required bytes may arrive
through the CPU's existing prefetch path; bypassing that path would not
exercise the real fetch contract.

Core's machine-level test separately proves rejected 486 construction and
retains the old-family execution cases. The updated metadata matrices cover
the complete primary/0F domains and the existing FPU domain.

Focused tests passed on both host widths: six registrations each, including
the new CPU test, machine gate and both registrations of each metadata matrix.
The first x64 complete-unit run had four failures from those two stale
matrices and one Lib boundary self-test timeout while an x86 build overlapped.
The unchanged Lib test subsequently passed alone in 3.73 seconds. No Lib
code, timeout or assertion was changed. Final complete suites run sequentially
without concurrent compilation; their terminal results are required before
delivery.

## Delivery Gate

Actual source/test accounting against the baseline uses Git numstat plus the
162-line new CPU test: production C/H (including the version header) adds 55,
removes 12, net +43; test C/CMake adds 177, removes 1, net +176. Manifests,
documents, root build configuration and EXEs are excluded. The new instruction
and its form/fault proof explain the increase. The retained sole metadata gate,
dispatch table, CPU state and bus path remain the production owners; no helper
framework, persistent state or public function was added. The appended profile
enum is the admitted existing-interface extension.

The final repository-only suites passed 593/593 on each host width: x64 in
482.26 seconds and x86 in 68.57 seconds, with Lib, Emulator, shared x86, Core
and all five Apps represented. Each includes three registered desktop-unit
cases, not an external guest/desktop UX claim. Both original Ninja/ccache
directories were reused, with no compiler/cache directory replacement.
All eight real EXEs also passed the existing product/INI identity integration:
the correct adjacent INI is selected, another product's INI cannot replace it,
and relative media paths retain their existing base. These are eight process
identity tests, not a claim that the external-ROM/disk boot suite was rerun.
That suite remains a T-level exit gate.

The affected four App pairs are current stripped Release products at 0559.
PE architecture and the embedded version string were checked for each; the
existing build/deploy rule checks architecture before copying. Other profile
builds reuse the common libraries and compile only their selected entry,
binding and embedded firmware inputs. Both graphs are restored to default
NXVM. The eight superseded 0557 EXEs are removed; INI/media/ROM/snapshot inputs
and both MyNES EXEs are unchanged.

| Product artifact under `assets/` | SHA-256 |
| --- | --- |
| `my5160/my5160_0_5_0559_x64.exe` | `046E314C43C20C9A59F8A828DF6E4C8EA221D30160F73EE2430A366942FD8D41` |
| `my5160/my5160_0_5_0559_x86.exe` | `3079309A6112BD42687B7A7F982DEA9318E8BFB33104799282A8EC59579BED4C` |
| `my5170/my5170_0_5_0559_x64.exe` | `FE479C0B83AB225A3ACE00E963F1FE7C759AC76C755A676DAEA26D29FA46B0E6` |
| `my5170/my5170_0_5_0559_x86.exe` | `64C76894C8A139498BE85ED44A35EF251369F2C2A44F5A715F82BEF9FB130E13` |
| `mydeskpro386/mydeskpro386_0_5_0559_x64.exe` | `AF6FCA07E5D2967247EFF860FA93DA12988FF16D8A06120AC95AB18548EC880F` |
| `mydeskpro386/mydeskpro386_0_5_0559_x86.exe` | `A7DFB91C1D94BF217E78A6AAF1B115FB206493A46766037F02C25A5A4E83C9F6` |
| `nxvm/nxvm_0_5_0559_x64.exe` | `FBE5A9AC1EB028F23954FE620CA4859365590AD322716728C57A0170C768E419` |
| `nxvm/nxvm_0_5_0559_x86.exe` | `CDD819FFAF9BCA0915BB7DE7961F2651027A7B5EAE0DEC119849D1C060664FCE` |

Executor actual-change review covers all added/changed CPU and test lines,
profile callers, both manifests, version/artifact/preset declarations and the
documentation changes. The new handler uses the retained CPU owner; test bus
fault injection does not add a production seam. Existing instruction tables
and all old enum values stay in place. Core manifests, documentation governance
and whitespace checks pass. The delivery source is this S2 implementation
commit over `406907f3f`; its hash is recorded by the subsequent coordinator
acceptance, without pretending the binaries embed a future commit hash.

Neither this record nor direct CPU proof admits a runnable PC110/486 machine.
