# Contributing

## Language And Scope

- Write repository content in English.
- Implement runtime code in portable C11 plus narrowly scoped Win32 API calls.
- Keep `core/` independent of Win32 where practical. Host files belong under
  `host/win32/`; executable-format classification belongs under `formats/`.
- New behavior starts with a bounded task record, exact verification command,
  and compatibility expectation.

## Clean-Room Reference Discipline

Use public documentation and observed behavior to define requirements. When a
source contribution is intentionally derived from NXVM, record source commit,
paths, license obligations, destination paths, and tests in
`docs/provenance/`. Do not copy from NTVDMx64. Do not inspect or reproduce
Windows private symbols, leaked Windows source, or private loader internals.

WineVDM integration is a process contract: classify an NE executable and pass
it to a configured external executable. Do not link, compile in, or redistribute
WineVDM without an owner-approved GPL-compatible release plan.

## Tests And Compatibility

- Add focused unit tests for VM state, executable loading, and DOS service
  contracts.
- Add application or utility probes only when their redistribution status is
  documented. Generated guest programs are preferred.
- A completed subtask records automated commands, result, local-input identity,
  and manual result when applicable under `docs/verification/`.
- Unsupported behavior must fail predictably with a documented diagnostic, not
  silently emulate a guessed result.

## Commits And Tracking

The hierarchy is `Milestone -> Task -> Subtask -> Part`.

```text
M1 T1 S1 P1: add COM launch contract
M2 T2 S3 P2: map DOS handle writes to host console
```

Create or update one `docs/tracking/M<milestone>-T<task>.md` file when a task
starts. Each active subtask has an `## S<subtask>` section and one concise part
entry in the same commit as its change.

Use only `TODO(High)`, `TODO(Medium)`, or `TODO(Low)` for deferred work. State
the reason and the condition that makes it active. Do not hide a current
blocker in a TODO.
