# PC Publication And INI Identity

## Objective And Boundary

T557 implements the owner's product-specific lowercase publication contract.
App display names, fixed machine keys, common INI grammar and media-relative
path semantics do not change. No SoftPC repository is read. Shared-six,
MyNES, owner INI bytes, ROMs, media and snapshots are outside this task.

## Design

Each App build declaration owns its release basename and explicit INI name.
The generic artifact rule consumes these values and supplies the INI identity
to that App's entry. The entry passes it explicitly to Core Product startup.
Core loads that file next to the executable through its existing parser; it
does not infer names from a versioned executable or display name and has no
model dispatch table, fallback or alias. Build/runtime identity has one owner.

| App | Release prefix | Runtime configuration |
| --- | --- | --- |
| My5160 | my5160 | my5160.ini |
| My5170 | my5170 | my5170.ini |
| MyDeskPro386 | mydeskpro386 | mydeskpro386.ini |
| NXVM | nxvm | nxvm.ini |

Release names are `<prefix>_0_5_0557_<x64 or x86>.exe`, in the existing flat
asset directories. Core Product still owns the shared version. Internal
machine keys and target structure remain intact. The owner-approved naming
contract supersedes the older machine-key filename rule, without rewriting
shared governance or historical records in this implementation task.

## Ordered Work

S1 is one cohesive startup/publication migration: audit all four identities,
update source/build/runtime wiring and current documentation, add positive and
negative real-App configuration proof, qualify both widths and deploy all eight
PC EXEs. Estimated implementation: 25–35 source/build/test files, roughly
180–300 added and 70–130 removed lines, excluding task records and artifacts.
Actual scope is determined by the caller/reference sweep, not a line quota.

Use existing Ninja/ccache trees, reusing component builds across profiles.
Root preset PC version target updates are a separate Shared-target commit;
no Shared source or MyNES preset is changed. INI renames retain byte hashes,
including a temporary intermediate name for Windows case-only changes.
New releases must be built and verified before obsolete names are removed.

## Exit Evidence

- Explicit App INI injection, correct display names and no legacy fallback.
- INI-relative media resolution independent of the process working directory.
- Both-width Core/Product/App units, complete repository units and affected
  configuration/Console/four-PC integration routes pass without reduced gates.
- Manifest, dependency, documentation and publication checks pass.
- Eight current lowercase EXEs have verified architecture/version/deployment
  identities; four INIs retain original content; no replaced EXE remains.
- Actual diff reviewed, target-separated commits pushed and remaining manual
  desktop/RDP verification reported honestly; owner acceptance follows delivery.
