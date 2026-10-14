# T557 S1 PC Publication And INI Identity

## Baseline And Ownership

Baseline is `ed75cf572`, after the separately approved MyNES diagnostic
publication. The owner accepted T556 S11; that task's unresolved MyNES S10
remains in MyNES Current, not silently treated as fixed. T557 changes only
PC publication/startup and its tests/documents. The three root PC preset target
version strings have a separate Shared-target commit. Shared-six implementation,
MyNES, external firmware and owner media remain unchanged. SoftPC was not read.

## Implementation And Contract

- Four App CMake declarations provide a release basename and INI filename.
  Generic artifact construction uses the basename; the same declared INI value
  is compiled into that App entry and passed explicitly to `vm_app_run()`.
- Core Product accepts the filename and uses its existing executable-adjacent
  loader. Grammar, media resolution, profile keys and display names are unchanged.
  No executable/display-name guessing, model switch, fallback or alias exists.
- Build integration consumes the selected INI variable instead of `NXVM.ini`.
  The Console process fixture loses an unused filename argument that only
  asserted the old name; it never selected the loaded file.
- The release-root gate requires exactly the current lowercase pair for each
  PC App. The INI gate checks App declaration/entry wiring and actual lowercase
  file names. Historical records are not mechanically renamed.

Windows filenames are case-insensitive: `NXVM.ini` and `nxvm.ini` cannot be
separate fallback candidates for the default App. Git records a case-only rename
through an intermediate filename, and the release gate enforces on-disk spelling.
No runtime compatibility branch was added.

## Verification Design

The new Core integration runs each real release EXE copied as `program.exe`
in a private temporary fixture. It does not copy owner INIs, ROMs or media.
With the correct adjacent INI, other product files and the CWD INI are invalid:
the real monitor must show help and exit normally. Without the correct adjacent
file, all other product INIs and the CWD file are valid: startup must return 1,
not fall back. The same fixture reads a code-owned INI through the actual Core
file loader and asserts that a relative media path resolves against its directory.
That parser assertion does not claim a guest-media boot; existing profile
integrations supply the separate real-media proof.

An initial synthetic blank-disk variant was rejected by the real startup path.
It was removed: this test does not need to emulate valid guest media to verify
configuration selection. An initial root registration classified the new target
as a unit without its arguments; 592 existing units passed, that added route
failed. Explicit registration in the existing integration partition fixes the
test wiring, without weakening classification gates or changing production.

## Protected Configuration Hashes

Renames preserve these SHA-256 hashes:

| File under assets | SHA-256 |
| --- | --- |
| my5160/my5160.ini | EC2AFB0E89421ED4BDA95D8B1797864D19EC360A3AD57237D2256DCE78950338 |
| my5170/my5170.ini | F4D85EE2E7BFF403F8C19588DA8416CC96FE5AC72FE7A1764817EDDDC9B76F19 |
| mydeskpro386/mydeskpro386.ini | C812A0C99D258C19CA8BB4BED584617DF2F4D25F17372583EB58BBD71D378599 |
| nxvm/nxvm.ini | FEEFDF82F40A8C7C1CD96C130BBEC4B5C41E099E194FA955AD42B99F35E8FC4F |

## Qualification And Release

Both original Ninja/ccache graphs pass, using the same persistent cache:

| Qualification | x64 | x86 |
| --- | ---: | ---: |
| Complete repository units | 592/592 | 592/592 |
| Default NXVM integration | 23/23 | 23/23 |
| My5160 integration | 3/3 | 3/3 |
| My5170 integration | 5/5 | 5/5 |
| DeskPro integration | 5/5 | 5/5 |

The 36 integration executions per width include common tests with distinct
App bindings. DeskPro suites complete in 129.95/149.05 seconds respectively,
without changing boot budgets or injecting F1. MyNES's 45 units are included;
its integration suite is not rerun or claimed for this PC-only migration.
Logs use `build/t557-s1-<width>-<app>-*.log`. Both graphs return to the default
profile after qualification. Profile switches reuse component libraries;
they do not create another build graph or cache.

Both specialized gate aggregates, Core source/test manifests, explicit INI
and registration gates, documentation and whitespace checks pass. Direct
compilation retains the existing three deferred strict-warning rows (510
strict of 513), not a claim of exemption-free whole-tree compilation.

Builds, full units and external DOS/Windows/profile tests are background
qualification. Registered Lib desktop tests pass in each unit suite; the new
INI test uses actual native Console child processes. No phone-client/RDP
manual gameplay, display scaling or owner interactive acceptance is claimed.

## Publication And Actual-Change Review

All eight EXEs have correct PE widths, embedded 0.5.0557 version and App INI
identities. Deployment succeeds before the old eight names are removed.
Each asset directory contains only its new pair, unchanged-content INI and
README. Superseded binaries remain recoverable from Git history.

| File under assets | SHA-256 |
| --- | --- |
| my5160/my5160_0_5_0557_x64.exe | FBAD6E581C4550126EEE51B8E8521FB2A66130D9BC5537989DC7A635C56AB676 |
| my5160/my5160_0_5_0557_x86.exe | 7C863CFAFAC63B6863F1C6812402B19911E85E1BAC8828A22FEDC87649A035C4 |
| my5170/my5170_0_5_0557_x64.exe | 7A8D214EF6C3E7A9E47D9AE343B66DCE58F0BF7BE1EB242E65D01B4F63961C6C |
| my5170/my5170_0_5_0557_x86.exe | DADF87F16183521C8151B63F632A8EB2D0374B9E6C7FADF33E618BABFEE8F73F |
| mydeskpro386/mydeskpro386_0_5_0557_x64.exe | 54A6D0C9D2B451099F017B0DE534BA2935FCC06508B1E09340F475C7F19096E1 |
| mydeskpro386/mydeskpro386_0_5_0557_x86.exe | EDEB75C4C28F46D6CA73008ADC9E7C07F022F291285401DFA4522F983150F52C |
| nxvm/nxvm_0_5_0557_x64.exe | DDDB91B973A6FFE96CB2CE6AAFF956E97465F1D0A96FBD453DB63D6439361D81 |
| nxvm/nxvm_0_5_0557_x86.exe | E5BA8A5B92EF3608D0A08F9DF25C6792E11816D41BDFB2EBA95AD3B8CD2E4384 |

Source/test/build review against admission:

- Production C/H: +11/-9, net +2. One existing startup signature gains the
  explicit INI argument; no new function, runtime state or public-six interface.
- Tests C/H: +120/-14, net +106. This includes the 107-line actual-process
  configuration test, removal of the unused Console fixture argument and
  neutral filename spelling in the parser unit. No existing assertion is reduced.
- Build/configuration: +70/-30, net +40, including the separate root preset
  +3/-3. App declarations own identity; generic build/registration consumes it.
- Core source/test manifests: +10/-9, net +1, preserving existing entry order.
  Documents and binary changes are separate from these implementation counts.

Coordinator review inspected the actual source, caller, build, test, document,
rename and binary changes. All requested wiring has one owner; no display-name
guess, fallback configuration, second parser or model switch was introduced.
Core chips/CPU/board/Machine, Shared-six and MyNES source/artifacts remain
unchanged. The obsolete shared-rule naming paragraph has an explicit
owner-approved exception and a separate Td receiver in TODO. Delivery awaits
owner verification rather than claiming all possible desktop scenarios passed.
