# T556 S9 - Quality Repair And Final Qualification

## Baseline And Disposition

Implementation baseline is `8f1f30c4d`, following S8 audit `a28cac691`.
The owner amended S9 to flatten DeskPro rather than retain S6's subdirectories.
S9 implements that amendment and A1--A5; A6 remains the explicitly recorded
Shared-rule vocabulary Td receiver. T556 remains open for owner acceptance.

- A1: 156 Core registrations now use Core-relative test identities; 120 no
  longer claim x86 ownership. Decoder/static targets and product aggregates
  follow Core ownership. The existing route gate rejects absolute identities
  and foreign ownership, with one negative-test file covering both and a valid
  control. No assertions or runtime cases were removed.
- A2: 18 DeskPro C/H files are flat under the App root; 30 unit files are flat
  under its unit root. Integration, diagnostic and genuine support remain
  separate. Role filenames, guards, symbols, includes, CMake and gates agree.
  The redundant Chips build library is removed; no substitute manager/API or
  mirrored D4 state was introduced.
- A3: D4 attachment applies the required immutable alias decode before
  publishing configuration. Failure removes the just-installed routes,
  observer and parity owner and returns the error. Tests prove absent-alias
  rejection, unchanged failed state/resources, and successful retry after
  installing the actual alias. Normal boot semantics remain unchanged.
- A4: DeskPro-owned symbols use `mydeskpro386_*`; actual logical state uses
  `lib_bool`, while register bytes, masks and counts retain integer types.
  MyNES Machine symbols/callers are a pure owner-prefix rename, separately
  delivered in `7fcd7b1d9`, with archived-link follow-up `c5ca06e85`.
- A5: current architecture/coding/source-policy/queue descriptions match the
  owners. CURRENT contains one active packet; older packets moved to clearly
  historical records with repaired relative links. PC artifact revision is
  now 0556, including the three root build-preset target references.

## Actual Change Review

Rename-aware Git counts include the CMake/manifest files within each listed
tree. Renaming does not mean deleting and reimplementing the moved algorithms.

| Scope | Added | Deleted | Net |
| --- | ---: | ---: | ---: |
| DeskPro source/build | 350 | 349 | +1 |
| Core version and source manifest | 3 | 3 | 0 |
| DeskPro test files | 298 | 283 | +15 |
| Core tests/registration/manifest | 229 | 182 | +47 |
| NXVM CMake references/gates | 145 | 151 | -6 |
| MyNES source, separate P1 | 341 | 341 | 0 |
| MyNES tests, separate P1 | 205 | 205 | 0 |
| Root presets, separate Shared P | 3 | 3 | 0 |

NXVM target total, including documentation and excluding binary byte counts:
+2,098/-1,819, net +279 (rename detection at 20%; 93 file entries including artifacts).
MyNES P1/P2 total is +818/-724, net +94, including documents/history retention.
The scope totals include mechanical movement of historical status text; they
are not estimates of newly introduced execution logic.

Retained owners: D4 owns its register state/attachment, Core owns routing and
resource rollback, MyNES Machine owns adaptation, and deterministic Core owns
hardware. No new production helper, persistent state, runtime path, thread or
public Shared interface. Valid post-construction reset retains its void API:
the required alias is now proven during construction and remains immutable.

The same-path sweep found two verifier receivers needing correction: Core
registration recognition still expected x86 names, and the firmware gate was
mistaking the App's build-time firmware generator for a runtime loader. The
first now recognizes Core; the latter checks all four flat runtime roots and
leaves the explicitly retained firmware build tools outside that runtime rule.
Both complete specialized gate targets pass after these fixes.

## Qualification

Use the original `mingw-gcc-{x64,x86}-release-ccache` build trees, Release,
Ninja `-j 8`, persistent `build/.ccache`. Both full graphs build. The same trees
were reconfigured for each fixed PC profile; no parallel replacement trees were
created. Core/Shared libraries were reused between profile builds; the three
later profile logs show no Core source recompilation. Re-linking selected
product and integration targets is still required and was performed.

| Executed selection | x64 | x86 |
| --- | ---: | ---: |
| Complete repository unit suite | 592/592 | 592/592 |
| DeskPro integrations | 4/4 | 4/4 |
| Default NXVM integrations | 22/22 | 22/22 |
| My5170 integrations | 4/4 | 4/4 |
| My5160 integrations | 2/2 | 2/2 |
| MyNES integrations | 12/12 | 12/12 |

The 44 integration executions per width include profile-specific invocations
of common routes; they are not 44 distinct source test files. MyNES's 45 units
are included in the 592, not an additional run. Units include all Lib, Emulator,
x86, Core and App selections. No timeout, retry or skipped case is counted as
a pass in these final full runs. DeskPro unattended installer checks pass in
158.67 seconds (x64) and 184.49 seconds (x86), without synthetic F1 input.

Background execution: full units, build/gates, external-media integration and
artifact inspection. Desktop coverage: the three registered native Lib unit
cases and MyNES native Window/Console integration cases ran and passed in each
architecture. This is automated native coverage, not a fresh manual gameplay,
keyboard/focus, audio-listening or all-guest-compatibility acceptance. Diagnostic
executables are built where in the graph, not claimed executed as integrations.

Both specialized gate targets, both product documentation gates, dependency
DAG, Core source/test manifests and changed-document file-link checks pass.
The strict compilation matrix retains its existing three documented deferred
rows; this is not a claim of no warning exceptions throughout the repository.
Public six and shared rules have zero diff against the admission baseline.
No INI, media, ROM original or snapshot change is present.

Logs retained under ignored build for acceptance: `t556-s9-{x64,x86}-unit.log`,
`t556-s9-{x64,x86}-gates.log`, and
`t556-s9-{deskpro,default,at,xt}-{x64,x86}-integration.log`, with matching build
logs. These preserve actual results; the documents state their finite claims.

## Artifacts

The source/artifact revision is the NXVM-target commit containing this report.
All eight PC artifacts are rebuilt stripped Release 0.5.0556; PE architecture,
embedded version and absence of debug sections are checked. MyNES's separately
committed 0044 pair is recorded in its
[receiving report](../../../mynes/etc/evidence/t556-s9-machine-owner-receiving.md).
Ten latest EXEs remain. Eight superseded 0555 PC EXEs are removed from assets
only after replacement qualification; Git history retains them.

| Artifact under assets | SHA-256 |
| --- | --- |
| `my5160/nxvm_xt_0_5_0556_x64.exe` | `9f1514154c84a2fc2ababaf7e43a8dd8759f85f16902c1ad82760a57cdd1d5ae` |
| `my5160/nxvm_xt_0_5_0556_x86.exe` | `61aa8908e84d0eb0a959640a564215e2e66f3a73aefded35f3d621de66d6b760` |
| `my5170/nxvm_at_0_5_0556_x64.exe` | `849271349b1408e967ac2de6c42de49e803ec201a14b13d3d4a4723b96fe43d7` |
| `my5170/nxvm_at_0_5_0556_x86.exe` | `601188b884490bccc98e859ab0088ce8ad90edb703e84f9a47343e1c1af648b6` |
| `mydeskpro386/nxvm_model40_0_5_0556_x64.exe` | `2eea9101a8162f7f16db5fc74803eb27d3c8219a0b959b61219ebb50fc020340` |
| `mydeskpro386/nxvm_model40_0_5_0556_x86.exe` | `ac48a6dbded590d7ccc5a5cde738ccb952e8a9263a9cec97ac4011c072d288d3` |
| `nxvm/nxvm_default_0_5_0556_x64.exe` | `4a0b945e227ca7a7530c3b2990097f18935b89c0996fbcfcf2f48bc072d65701` |
| `nxvm/nxvm_default_0_5_0556_x86.exe` | `b4d3eed242a0e8d1ccdec65a653e021ecca562405219dff82ffb0d518a218e4b` |

## Coordinator Review

Implementer and coordinator roles were performed sequentially by one agent.
Actual-diff review confirms the file map, atomic failure regression, mechanical
MyNES changes, preserved Shared boundary and current/historical documentation
separation. All required S9 automatic qualification is complete. Deliver S9
for owner review; do not close T556 or start its successor on this evidence alone.
