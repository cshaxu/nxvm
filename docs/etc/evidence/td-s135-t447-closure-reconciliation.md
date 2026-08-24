# M5 Td S135 T447 Closure Reconciliation

## Scope

This standalone governance correction closes the two findings from the
post-T447 code-quality re-audit. It does not change a runtime contract,
artifact, or product behavior.

| Finding | Disposition |
| --- | --- |
| VM session-private header included Core `fdc.h` and `hdc.h` implementation headers solely for interface types. | Removed both includes. `machine_interface.h`, already included by the header, supplies `controller_interface.h` and `fdc_observation_interface.h`; no replacement dependency or wrapper was added. |
| T447 remained represented as an active packet/proposal and its history stopped at S6. | Retained the proposal beside its T447 history record, appended accepted S7--S11 facts from committed evidence, removed the active queue claim, and retired the completed packet at Td closure. |

## Similar-Issue Sweep

- No VM production source retains a direct `core/machine/fdc.h` or
  `core/machine/hdc.h` include.
- The retained history record contains accepted S7, S8, S9, S10 and S11
  sections. Historical prose saying that an earlier accepted S left T447 open
  is retained as chronology, not current state.
- No current status, queue or evidence link retains the former proposal path
  or calls the closed T447 proposal active.

## Verification And Review

- All eight direct T447/session-layout static CMake scripts passed.
- Documentation governance passed after the packet's non-ASCII owner quote was
  replaced with an English traceable rendering required by repository policy.
- `git diff --check` passed; actual-diff review confirmed one code deletion
  mechanism and one documentation-topology migration, with no ABI or artifact
  change.
- Direct MinGW `-fsyntax-only` checks of three session composition units could
  not complete: the Windows execution environment started GCC/`cc1.exe` then
  returned status 1 without a compiler diagnostic, matching the concurrent
  stalled Ninja/CMake executor condition. This is recorded as environment
  limitation, not a source failure. The changed header's required types are
  directly declared by headers already included through `machine_interface.h`.

## Code Accounting

Tracked code paths: `src/vm/composition/session/session_private.h`.
Added 0 lines, removed 2 lines, net -2. Documentation and this evidence are
excluded. The deletion removes an implementation-layer dependency; it adds no
new abstraction, state, path or compatibility behavior.
