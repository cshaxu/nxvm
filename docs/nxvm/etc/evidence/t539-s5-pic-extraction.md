# T539 S5: PIC Extraction Evidence

Baseline be86dee2e. This batch covers pic.c, pic.h, pic_interface.h and all
their production/test consumers. It does not qualify new hardware behavior or
close the remaining chip inventory. Shared delivery is d6dc6ca3a; NXVM
delivery is 6cf3cee40. Both are pushed and accepted after actual-change review.

## Ownership And Actual Changes

`src/x86/devices/pic8259` owns one opaque controller: ICW/OCW, priority,
IRR/IMR/ISR, resolved input levels, acknowledge and unmask deadlines. The
existing cohesive handlers are retained. Types is its only dependency.
Cascade identity, available signals and selected interrupt values are copied;
no controller owns a peer pointer, board port or source registry.

NXVM's renamed pic_bus owns port decoding, independent source counts, pair
wiring and vector routing. Machine creation handles both chip allocation and
port-registration failures. The CPU and other controllers still connect to
that board endpoint until their own extraction batches; this is not a claim
that those remaining chips are already independent.

CPU fixtures initialize the vector with real ICWs instead of private writes.
Single-chip initialization, EOI/rotation, AEOI and OCW3 internal assertions move
to the chip-owned test suite. Board tests retain cascade, SFNM, poll, shared
IRQ sources, CPU acceptance and reset/deadline coverage. Failure diagnostics
do not mutate OCW3 to inspect live hardware. Model 339 timing is checked by
mask/assert/unmask and deadline observation, not internal array inspection.

The first full x64 run found a new transaction interaction: PIC's registration
begin cleared an earlier device's allocation failure. The assembler now checks
the preceding registration result before starting PIC. The existing injected
failure test passes; all eight PIC registration failures now have rollback and
successful retry coverage. This fixes the cause, not the expected test result.

## Similar-Issue Sweep

Searched all src/app-nxvm, test/app-nxvm and cmake for old pic headers, t_pic,
private PIC data, source counters and cascade links. Production hits are now
board endpoints; single-chip private assertions are inside test/x86. The
existing CPU/PIC authority gate rejects restored old files and private chip
includes/accesses. Shared negative probes reject App, Common and private peer
dependencies. No Lib/Common/MyNES source, INI, firmware or media is changed.

## Verification

- Full NXVM units: x64 and x86 each 341/341.
- Default-profile external integration: each width 20/20.
- Independent chip-only suites: each width 12/12, including manifests and
  negative boundary tests; compiled with warnings as errors.
- Specialized static aggregate passes; extended PIC authority check passes.
- All six shared manifests verify; x86 source/test revision is
  shared-m5-t539-s5-p1.
- XT boot passes once per width (x64 19.16s; x86 20.47s); 5170 passes once
  (32.41s/37.32s); Model 40 passes once (56.53s/68.93s). Together with default
  integration, all eight profile/width combinations reach their existing DOS
  or installer terminal. This is bounded regression, not exhaustive qualification.

Commands use the retained build/t539-s3 dual-width trees for full unit and
integration targets, and chip-only test/x86 builds with X86_BUILD_TOOLS=OFF.
The initial x64 failure and its correction are recorded above; the complete
x64 rerun passes. No failed or pending check is counted as closure evidence.

## Size And Receiving Artifacts

Against baseline, production C/H changes are +1078/-1040 (net +38); test C/H
changes are +754/-482 (net +272), counting added files and relocations without
rename inference. Build files, manifests, docs and binaries are excluded.
The production increase supplies the opaque instance and transactional board
boundary, not another priority algorithm. Tests add independent contracts and
failure rollback while retaining original CPU/PIC scenarios.

All eight Release 0.5.0539 files below have the expected 8664/014C PE machine,
embedded version and no .debug/.zdebug/.stab sections. Runtime Debug remains.
All four owner INIs have no content change; MyNES has no x86 dependency and
its receivers are unchanged.

| File | SHA-256 |
| --- | --- |
| nxvm_model40_0_5_0539_x64.exe | 1763DB94EAB6F83A44AB2DA1DF08D0484E697B62F79BE538FF5E1EE3B243FD3D |
| nxvm_model40_0_5_0539_x86.exe | EFDB21C2684E35AB96B66C8B8B826984F2DE36152FF4527A76AC36F6A1565425 |
| nxvm_default_0_5_0539_x64.exe | 38F4D05F8A9671D3440293352D772AB3BA1465B34945A51B4B4E2C09B433E48E |
| nxvm_default_0_5_0539_x86.exe | 607300AC083BF3FC1B9B8A8AAAA79F6AD7C004C284505DABB0A4603F0383211F |
| nxvm_xt_0_5_0539_x64.exe | 7BE57391DBA5F6C63DD378BACA183F88F9E3DA6ABBBA1D508611AC858447E80A |
| nxvm_xt_0_5_0539_x86.exe | 86E4F03D3D797111623E0925E0035BB480052765BB3212AD4BCF957C7A5B6C9C |
| nxvm_at_0_5_0539_x64.exe | BD247B9B027EA24DAF2CD67F1535D62D508B83206969745F247E9D93D73F863B |
| nxvm_at_0_5_0539_x86.exe | 6709302CE25BC056780457D73953E312E4596FC2E2FD588E409763D64D5E36E4 |

## Actual-Change Acceptance

Coordinator reviewed d6dc6ca3a and 6cf3cee40 against the finite PIC brief:
single-chip priority/ICW/OCW ownership, strict slave/poll selection versus
master SFNM, resolved level versus source counts, cascade acknowledge,
configured delay/reset retention, source lifetime and transactional failure
cleanup. CPU/device callers no longer inspect private PIC state. All required
receiver proofs above pass; six manifests and committed diff checks pass.
No outstanding item remains in S5. T539 and the unextracted inventory remain
open. Temporary S5 chip/probe build trees are removed; reusable dual-width
NXVM trees remain for subsequent automatic admission.
