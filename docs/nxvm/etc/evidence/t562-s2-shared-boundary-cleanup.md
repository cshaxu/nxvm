# T562 S2 Shared Boundary And Redundant-Path Cleanup

## Authority And Baseline

The owner approved all five findings of the follow-up shared-package audit.
S2 starts at clean `7309e3cfd`; the active packet and T562 proposal define
Shared and NXVM delivery targets. Siblings are read-only. This is not another
upstream import, a CPU timing qualification, or a Lib/Emulator runtime change.

## Finite Disposition

| Finding | Implemented disposition | Direct proof |
| --- | --- | --- |
| Port compatibility | Remove unused legacy handler dispatch and five private registration conveniences; one typed route registry remains | Port context, registration rollback, multi-byte/wired-OR, guest I/O and board ownership regressions |
| Test ownership | Move AT composition and mixed PIC/KBC tests to Board-Base; retain serial cadence in Board-AT without a Board-Base link | Owned registrations, bounded AT fixtures, PIC/Core fixtures and unchanged guest protocol assertions |
| Empty public wrapper | Delete `app_composed_machine`; use `emulator_product_machine` directly; rename actual UI settings to `x86_ibmpc_product_ui_options` | IBM PC entry and NXVM startup/factory tests, all four PC receivers |
| Retired names | Rename nine Core test files and stale Core/IBMPC test identifiers/guards; update direct consumers | Name gate, manifests, fixture-shape checks and full unit build |
| Checker blind spots | Reject unknown source owners; inspect included test CMake, uppercase links/registrations and inner-to-outer fixture edges; enforce Core private headers and link directions | Positive controls and negative injection suites for each changed checker |

The stronger fixture check also exposed an AT parity fixture that imported
Board-Base composition. The AT fixture now owns only route-failure injection;
Board-Base owns the composition call and verifies that injection was reached.
No production parity behavior changed.

KBC private layout, construction rollback and keyboard/chip observations stay
in the separately compiled Board-AT fixture. Board-Base holds PIC/CPU wiring
and accesses an opaque adapter through public operations plus bounded fixture
observations. Guest status masks are explicit test protocol constants. This
does not add a production API or a second emulator path. Allocation tests
remain at the adapter owner; protocol tests exercise public create/destroy.

## Retained Responsibilities And Similar-Issue Sweep

- Port ranges, byte-lane dispatch, wired-OR reads, registration status and
  rollback are live semantics and remain. The sole test caller of the removed
  write convenience now installs the same typed route as production.
- NXVM's own composed value retains its real information/speed context; it is
  not the removed empty IBM PC wrapper. Emulator continues to own lifetime.
- CPU/FPU/Machine and observable builds remain distinct existing responsibilities.
  No hardware variants, timing algorithms, queues, threads or persistent state
  were added, collapsed or replaced.
- Lib and Emulator changes are checker/negative-test changes only. No runtime
  source, public API or behavior in those two packages changed.
- Direct PC App test callers only follow renamed fixture identifiers. No App
  production source, MyNES input, INI, media, snapshot or external ROM changed.
- All twelve shared source/test manifests are regenerated with the existing
  deterministic header, sorted paths, SHA256 and LF format.

## Actual-Change Review

Production C/H changes are **+22/-92, net -70 lines across eight paths**,
including the removed wrapper header. Most test churn is identifier and file
relocation, not new test algorithms. The review checks constructors/destructors,
retained guest protocol assertions, failure injection reachability and included
build edges rather than interpreting moved lines as new functionality.

During fixture separation, compilation correctly rejected dependencies on
private KBC status constants and the private test-input setter. They were
replaced with test protocol masks and an owner-local fixture call; no public
header was widened to make the tests compile.

Shared tree counts below include manifests and use Git's default rename
detection. A heavily edited KBC move is counted as delete/add; that is not
an increase in executable coverage or a second implementation.

| Tree | Added | Deleted | Net |
| --- | ---: | ---: | ---: |
| src/lib | 2 | 2 | 0 |
| src/emulator | 5 | 2 | +3 |
| src/x86-core | 40 | 74 | -34 |
| src/x86-debug | 4 | 1 | +3 |
| src/x86-ibmpc | 14 | 23 | -9 |
| src/x86-nxvm | 11 | 11 | 0 |
| test/lib | 13 | 9 | +4 |
| test/emulator | 7 | 1 | +6 |
| test/x86-core | 175 | 162 | +13 |
| test/x86-debug | 14 | 1 | +13 |
| test/x86-ibmpc | 17 | 4 | +13 |
| test/x86-nxvm | 2050 | 1968 | +82 |

Common test boundary infrastructure is +59/-2. NXVM build/fixture/registration
checks are +19/-19; fourteen direct App test consumers are +42/-42.
No new production helper, persistent state or public capability was added.

## Qualification

Final frozen-code full repository units pass **630/630 on x64 and 630/630 on
x86** (`t562-s2-<width>-units.log/xml`). The earlier x64 pass preceded the final
fixture tightening and is not used as proof of that later change. The final
runs took 261.18 seconds and 87.99 seconds respectively. Both unit builds
completed with the existing strict target options and retained pre-existing
documented warning exceptions; this is not an exception-free compilation claim.

Both widths also pass:

- Four profile BIOS boot cases, without skip or budget change (8 total).
- Ten selected profile INI/Console/lifecycle integration executions, plus one
  restored-default INI identity test per width. With boot cases this is 15
  CTest integration executions per width.
- Four direct CLI/INI checks against the final published EXEs (8 total), after
  the last build. PE machine values, 0562 version strings, exactly two EXEs per
  App and final SHA256 values are verified in the [inventory](t562-s2-artifacts.json).
- Complete specialized gates, including registration uniqueness, fixture
  shapes, source boundaries, all manifests, vocabulary and direct-compilation
  checks. Documentation governance and Git whitespace checks also pass.

The full unit runs include three `desktop`-labelled tests per width; these are
automated desktop tests, not manual RDP acceptance. The remaining 627 units
per width are not desktop-labelled. All unit XML records report zero failures,
disabled tests and skips. Logs are `t562-s2-<width>-units`, per-profile
`integration`/`boot`, final `published-cli`, and `specialized` under `build/`.
Original Ninja graphs and persistent ccache are retained and both graphs are
restored to the default profile. Eight rebuilt PC artifacts are published;
MyNES runtime and artifacts remain unchanged because their executable inputs
did not change.

The specialized registration gate initially rejected the moved KBC test:
its component-relative name was valid, but the root checker only recognized
Core component registrations. It now recognizes both current X86 Core and
X86 NXVM namespaces, preserving target identity and duplicate-route checks;
the gate passes on both widths. A temporary release-script filter also omitted
the actual `vm-profile-floppy-boot-matrix` test name. The missing boot cases
are run explicitly with `--no-tests=error`; a partial selection is not treated
as boot coverage.

No fresh mobile RDP, physical audio/MIDI, Linux execution, manual Windows GUI
or full external diagnostic-ROM qualification is claimed by this S.

## Delivery

Shared P1 `e5b346309` contains the reviewed package source/test changes.
The NXVM receiving P contains the direct test/build references, this evidence
and eight verified PC artifacts. The inventory pins the combined source and
receiver delivery rather than relying on version text alone. Implementation
qualification is complete; coordinator review must inspect both actual commits
before accepting S2. T562 remains open.
