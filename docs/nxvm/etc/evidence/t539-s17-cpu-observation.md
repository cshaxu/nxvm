# T539 S17: CPU Observation Evidence

Baseline a3bb1e3f8. Implementation and receiving verification are complete;
delivery and coordinator acceptance are recorded below. Current owns status.

## Mechanism And Sweep

The single memory resolver still owns A20, selected RAM, replacement/ordinary/
fallback providers, ROM alias precedence and range failures. Its read operation
now carries observation intent to the selected provider; recursive per-byte
crossings preserve that intent. RAM observation omits only parity notification.
Reset inspection retains the CPU's existing high-alias-first behavior and
ordinary fallback on UNSUPPORTED, never an operational-read fallback.

All six production memory read callbacks were inspected and migrated:

- immutable ROM: pure byte copy, no effect to suppress;
- D4 setup and control (two callbacks): pure selected-value reads;
- absent-memory window: pure configured open-bus value;
- CGA and planar video (two callbacks): select the video owner's read or inspect
  operation. Both use the same decode and value calculation; planar inspection
  uses local four-byte latch values without publishing them to the device.

Searches cover memory_device_read, register_*device_provider, provider->read,
read_physical, read_reset_physical, read_real_from and preview_mode throughout
src/app-nxvm, test/app-nxvm and the receiving Shared video owner. No other
production read provider exists at this baseline. Test provider signatures and
operational-read counters follow the same contract; no incompatible cast is used.

CPU preview's physical-read boundary and all three timing descriptor rereads
now inspect. Preview already suppresses page-table writes, transaction events
and diagnostic publication; regression now also covers provider/parity effects.
The board display snapshot backing reader is observational and now inspects.
DMA transfers, firmware memory services and normal instruction operands remain
actual bus operations. Existing paused public memory and Debug real/linear/port
read commands retain their operational read semantics (including trace and
device effects); changing those command contracts is not this CPU-preview fix.
They are not claimed to be nonintrusive observations. No command is removed.

The static Jcc boundary gate now requires CPU inspection and rejects ordinary
physical read calls in the timing model. This is source-shape prevention,
not proof of runtime correctness. CPU private pointers, opaque ownership,
board timing separation, firmware interception removal and generated catalogs
remain the pending CPU extraction batch; S17 does not accept the CPU ledger row.

## Initial Verification

Both new observation regressions and the existing CPU preview suite pass on
x64 after correcting the new video fixture to configure the EGA controller and
VGA capability before exercising its windows. Initial fixture failure is not
counted as a production failure or pass. The first build also caught a misleading
indentation in a migrated test callback; splitting its return fixed the warning.

Memory proof includes equal bytes across RAM/device boundaries, operational
provider/parity effects versus none during inspection, A20 wrapping, high reset
preview, provider errors without fallback, zero-length rejection and paged
preview without PDE/PTE accessed/dirty publication. Shared video covers CGA,
planar read maps, color compare and enabled VGA chain-4, verifying equal read
values and untouched latches before an ordinary read changes them.

Both complete receiving builds pass. Full units pass 370/370 on x64 (211.12 s)
and 370/370 on x86 (36.93 s), including both new observation tests. All six
manifest verifiers and git diff whitespace checks pass at this checkpoint.
The elapsed times are verification observations, not a cross-width performance
comparison: host load was not controlled.

Default integrations pass 20/20 on x64 (53.34 s) and x86 (61.99 s).
The tools-off independent build and all 45 tests pass (4.12 s). Default
0539 executables are rebuilt/deployed in both widths, with INI content unchanged.

The first specialized aggregate exposed the missing direct-constructor fixture
classification for the new memory test. It legitimately configures MMIO/reset
providers and parity before freezing execution, so it is retained as an explicit
constructor, not forced into an instruction-only fixture. The named inventory
now includes it (97 fixtures plus four timing producers); duplicate and unknown
constructor checks remain intact. This is a test-registration omission, not a
production pass or a weakened gate.

The rerun specialized aggregate passes, including the 369-row direct strict
compilation matrix (336 retained strict, 33 explicitly deferred). Documentation
governance and the x86 component boundary verifier also pass.

MyNES does not link x86 video and is unchanged. All receiving cases below run
once; no successful group is repeated.

XT x86 is rebuilt and its single existing BYOB probe passes with
`--no-retirement-observation`: exit 0, installer-ready, 12,848,510 executed
instructions and 127,347,775 guest elapsed ticks. The original adjacent INI
and embedded firmware are used; no injected F1 or turbo override is applied.

AT x86 likewise passes once: installer-ready, exit 0, 26,298,225 executed
instructions and 127,191,063 guest elapsed ticks. XT x64 passes in 24.16 s
(12,860,522 instructions, 127,479,899 ticks); AT x64 passes in 31.12 s
(26,317,777 instructions, 127,280,890 ticks). Diagnostic wall polling can select
different terminal instruction counts; none is used as a timing-grade claim.

Model40 x64 passes in 65.15 s (26,658,111 instructions, 167,925,699 ticks);
x86 passes in 81.11 s (26,663,879 instructions, 167,959,082 ticks). Both return
exit 0 and installer-ready within the unchanged 90-second bound. All six vendor
cases use the existing no-retirement-observation switch without F1 or turbo.
Both reusable build trees are restored to default; the three bounded S16/S17
trees remain needed for the next CPU extraction batch.

## Actual Source Review

Review covered the complete production/test/CMake diff, not only test output.
The ordinary route retains address/provider selection and error precedence;
inspection reuses that resolver, carries intent across byte splits and never
falls back to an effectful call. Immutable/open-bus providers explicitly ignore
intent because they have no mutable read effect. Video calculates local latch
bytes once and publishes them only for operational reads. CPU handler tables,
timing formula bodies and normal operand/transaction paths are unchanged.
No second decoder, live state clone or new owner is introduced.

All eight migrated test callbacks were inspected; read counters remain effects
of operational reads only, and their original assertions remain. The two new
test files cover the actual CPU/MMIO and chip-latch boundaries. The CMake target
declarations were kept contiguous during review. MyNES product linkage was
rechecked in its product CMake: it uses Common, Lib and its NES driver, not the
changed x86 video target; no MyNES rebuild or change is required.

Against a3bb1e3f8, excluding manifests, docs-tree files and binaries and including
both new tests: source +109/-37 (net +72, including Shared README), tests
+210/-14 (net +196, including test CMake), CMake +24/-9 (net +15). Total net
+283. Growth is the explicit observation boundary and its regression coverage,
not a parallel production implementation. This review does not accept the
still-pending CPU extraction or claim nonintrusive Debug bus operations.

## Artifact Identity And Delivery

All eight 0539 Release executables were rebuilt and deployed. PE machine values
are 8664 (x64) and 014C (x86); each contains 0.5.0539 and objdump reports no
compiler debug sections. Runtime debugger linkage is retained. INI content,
external asset masters, Lib/Common and MyNES are unchanged. Commercial originals
stay external; embedded-ROM executables use the recorded owner authorization.

| Artifact | SHA-256 |
| --- | --- |
| nxvm_model40_0_5_0539_x64.exe | 2A59B5B97BABB1A1B8ABC6B3B55684E69F72E02FCA0589CC45BEA7B8A4C62597 |
| nxvm_model40_0_5_0539_x86.exe | 4904C51C131502BA3789314BA8F34FFB157A57158BAC4ADA714DAB30831C537A |
| nxvm_default_0_5_0539_x64.exe | 9411B25743BF042DBD64746F5D54876ABEA760B9A7441278DC200F375021B55D |
| nxvm_default_0_5_0539_x86.exe | 4CB590157A29ADA385C8ED667E76769912BFC7E97AB15FDC8D7D9A5BBAE150AD |
| nxvm_xt_0_5_0539_x64.exe | 024485A52AE03C1B5357E1AC585CF7ABDA3AECF9E58E7D76337010894E6084C7 |
| nxvm_xt_0_5_0539_x86.exe | 0DFCCD4708FB08CA96D0A3762CCB9EB6996C8D73A4D164D1D690B3C3969FE3F5 |
| nxvm_at_0_5_0539_x64.exe | 9CE9CCDA30A7F40ECEB84FFA0D14C9C40CB0C71049043AA31CF5B0D65DF068D8 |
| nxvm_at_0_5_0539_x86.exe | 22DC4E6D9415CB9C5FAB4EE0CA69D18D83027F8AB7A9163A33108C9EFEA23C91 |

Shared P1 cadaf0990 and NXVM P2 f85888d3e are pushed. P2 owns the matching
CPU/memory/provider callers, receiving tests, gates and eight artifact blobs.
Coordinator actual-commit review accepts their complete S17 scope: all changed
provider/test signatures, operational-versus-observational routing, CPU call
sites, video latch behavior, CMake registration, fixture inventory, source gates,
manifest coverage and documentation were checked. The deployed executable blobs
equal the committed blobs; artifact architecture/version/hash checks match the
table. Targets are separated and no INI, external master or MyNES change entered.

The original owner objective remains chip extraction; S17 satisfies only its
explicit prerequisite packet. CPU private peers, timing/board separation,
firmware interception removal, generated catalogs and the other finite ledger
dispositions still need their own proof. No completion claim is transferred to
the later board task. Prevention is the CPU observational-call gate plus the two
owner-local regressions; it does not substitute for runtime side-effect proof.
