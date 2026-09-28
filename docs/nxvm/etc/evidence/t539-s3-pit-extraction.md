# T539 S3: PIT Extraction

Baseline 9c1eadf0d. Owner approved the PIT Shared/NXVM boundary on 2026-09-28.
This is an extraction, not a new claim of complete hardware timing accuracy.

## Ownership And Actual Change

- `src/x86/devices/pit825x` owns the sole 8253/8254 state, counter/register
  mechanism, GATE/OUT behavior and input-clock deadline calculation. Public
  state is opaque; dependencies are Types only, no App or other chip internals.
- NXVM's `pit_bus` decodes board port addresses into chip register selectors;
  it preserves the existing byte-latch and wider bus behavior. It rolls back
  partial port registration and destroys the chip on any construction failure.
  This attachment has one construction/destruction lifetime, not a second timer.
- Machine owns the handles. Scheduler clock conversion and signal order remain
  unchanged. PIC IRQ0, DMA/DRAM refresh, speaker and Model-40 auxiliary PIT/D4
  consumers remain in NXVM. Destruction drops timer outputs while sinks live.
- The sole D4 configuration owner rejects repeat configuration; no board code
  inspects PIT callback slots. Old PIT source/header and three product-owned
  pure timer tests move, rather than remain as a parallel implementation.
- Chip tests use the public register/signal/deadline interface. Board tests
  retain ports, clock ratios and receivers; former count-array checks use latch
  reads. Integration diagnostics use OUT observations, not private PIT arrays.

## Preservation Review

After normalizing only function/type prefixes and lib_bool/lib_u8 (the same
underlying width), 28 retained functions compare identically to the baseline,
including all waveform handlers, count conversion, latch helpers, GATE, OUT,
deadline, reset and advancement. Manual review covers the remaining register
read/write adaptation, control-word handler, create/destroy and board teardown.
Unprogrammed reads still preserve the input bus byte; 8253 still ignores 8254
read-back; sink rebinding does not change GATE or emit synthetic edges.
No mode algorithm, frequency, timing grade, firmware or BIOS special case changed.

Shared tests retain mode 0--5, aliases 6/7, odd mode-3/count-one, binary/BCD zero,
rewrites, GATE and read-back cases. Additional tests cover invalid arguments,
instance isolation and reset/destroy output release. Board tests cover every
one of seven PIT route-allocation failure positions and duplicate D4 setup.

Two test construction calls were initially placed inside assertions, which
NXVM Release removes; the first full runs exposed that error. They now execute
unconditionally with explicit status checks. No production workaround was made.
An x64 runner-error test also failed once during concurrent build/test load;
its isolated rerun passed without source changes. Final full-run results follow.

Static audit repaired two existing governance omissions: current NXVM presets
still named 0535; the integration boundary gate did not recognize the real
deployed-EXE probe. The latter still forbids direct machine/config construction
and verifies the probe launches the product with its supplied working directory.
The lifecycle guard now rejects direct product-owned x86 PIT lifecycle calls.

## Verification

- Independent PIT build with `X86_BUILD_TOOLS=OFF`: x64 and x86 each 8/8
  (four chip tests plus four manifest/corpus/negative checks), without App,
  Common or external assets. Full x86's ten runtime cases also pass in the
  product unit suites, with all four x86 metadata/negative checks per width.
- Complete NXVM unit suites: x64 337/337 and x86 337/337. Final full x64 run
  includes the previously transient runner-error case and passes unchanged.
- Default-profile external integration suites: x64 20/20 and x86 20/20.
  Non-default INI boot matrices pass once per combination: XT x64/x86
  17.37/19.27 seconds, AT 31.59/35.71 seconds, Model 40 53.99/70.35 seconds.
  Existing DOS/installer terminal acceptance is unchanged; this is bounded
  regression proof, not a new whole-hardware accuracy claim.
- All 66 specialized static build steps pass, including the repaired preset
  and integration-route checks. All six manifests and diff whitespace pass.
  Documentation and actual-change review accompany acceptance below/history.
- Source/test delta against 9c1eadf0d, `git diff --numstat` with C/H pathspecs:
  +967/-838, net +129 (production +55; tests +74), 36 rows including two
  detected source renames. Moved tests whose mechanical edits defeat Git's
  rename heuristic are counted as delete/add, not duplicate retained tests.
  The increase buys the opaque interface, transactional board attachment and
  new contract/failure tests; there is no device framework or second waveform.
- Similar-issue sweep covers old PIT includes/types/functions, private fields,
  both instances, all port routes, reset/finalize and board consumers through
  `rg` over src/test/cmake. Only the new chip owner contains PIT internals;
  the lifecycle and x86 boundary gates reject regressions. Other-chip coupling
  remains explicitly assigned in the original finite migration ledger.
- MyNES product/core CMake link closure contains no x86 target. Lib/Common,
  their tests and all MyNES paths have no diff; its two 0043 EXEs are unchanged
  and do not need a rebuild. No sibling repository was modified.

## Receiving Artifacts

Eight developer artifacts in `assets/nxvm/<profile>/` are Release 0.5.0539,
compiled from the S3 Shared and NXVM source delivered as P1/P2. PE Machine is
8664 for x64 and 014c for x86; the embedded version matches. Release uses
`--strip-debug`, and all eight have no compiler debug sections; ordinary COFF
symbols are not mistaken for compiler debug information. Runtime Debug remains.
Superseded 0538 EXEs are removed from assets and remain recoverable in Git.

| File | SHA-256 |
| --- | --- |
| nxvm_xt_0_5_0539_x64.exe | 826537820C5F1CB6606ECEC61AD5C1124AB35DB88AA37226D7429FE836CD7152 |
| nxvm_xt_0_5_0539_x86.exe | 01DE48393F26A34BDEA9D81350041CE3FC37AF854BFC7561687AC49898BA2638 |
| nxvm_at_0_5_0539_x64.exe | CD796E2A9BC69D4346AF5F70997B51E24873DF7AAEA9B8C3E0408182EA5EE044 |
| nxvm_at_0_5_0539_x86.exe | 06B865CD3F9F76B13EF87DB25AE2CECB3A73C917A8A5B80902A0A0EA85295ECE |
| nxvm_model40_0_5_0539_x64.exe | C5AB53F72D17B8F5E1E9B10C7A621A7AEB58E7B1A1F8363E2C4101BD7DC3DE82 |
| nxvm_model40_0_5_0539_x86.exe | DEABB8C9A49551634BD348908A4C078DCBAFB7291AF7D8D14A7DB60AA5BB6275 |
| nxvm_default_0_5_0539_x64.exe | 82872D7B927BB98157AF56EE6CB170E14DDB92DBE5C85514A8482BD1E10AFE05 |
| nxvm_default_0_5_0539_x86.exe | C9FA8A0F14C335C0121F28E4602CC2CA81D4DC7DBC7B9CC409FDDD4BC5985911 |

Owner INIs are byte-identical before/after builds:

| Profile | NXVM.ini SHA-256 |
| --- | --- |
| XT | 48900ED858BE7B6A5AE8CD452B4F4432FC9446365FC366A49470DA331C443B29 |
| AT | 282478D7DFB479070FDB138867DD08C99C5283770C5872DDEA07529D32C8A15C |
| Model 40 | 4C363DB4FD3FA8864C89B2557FA2A2BF93994E67E295B11AEE07053EC1FDB0E3 |
| Default | A858BEE0D2145E539D69861A675FDDB10433B25EAD658A9DFAF0D69632989F13 |

Only the two NXVM build trees are retained for the immediate next chip step's
incremental regression baseline; standalone PIT and profile-switch build trees
are disposable after acceptance. No external assets, INIs, media or fonts change.
