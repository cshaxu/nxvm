# M5 Direct 80386 Package-Close Audit

## Scope And Method

Td S57 audits T301--T312 after accepted T312 closure and before any M6
consideration. It reviewed the retained NXVM route, `src/core/machine`, public
core interfaces, VM composition and default-profile use of core, CMake current
artifact and smoke registration, and the M5 status, Queue, TODO, history, and
evidence records. This is governance evidence only: it changes no CPU, ABI,
artifact, or product behavior.

The source-shape review used the existing `verify-executor-closure`,
`verify-public-interface-boundary`, `verify-vm-provider-composition`,
debugger-capability, and current-artifact checks as the recurring contracts.
The accepted current identity remains `vm-0-5-0311`.

## Results

| Area | Result | Evidence and disposition |
| --- | --- | --- |
| Core machine and public interfaces | Clean | T301--T311 use the retained `core-machine-executor`, checked memory/stack, selector-cache, port, and exception routes. Existing public-interface and executor-closure gates reject private-layout leakage, raw debug borrows, and a second executor. No new global machine facade or unused public wrapper was found. |
| VM composition, profile, and product | Clean | VM session composition binds typed providers to one `core_machine`; the retained composition gate rejects executor/shared-state access from VM providers. The NXVM Console/window/debugger route remains the only product route reviewed. The existing paused-boundary debug mutation debt remains separately recorded in TODO, not introduced by this package. |
| CMake, tests, and artifact identity | Clean | `current-gcc` selects `vm-0-5-0311`; `PROJECT_CURRENT_SMOKE_TARGETS` and current-gate verification remain registered. No T301--T312 CMake target creates a second runtime or product route. |
| Documentation identity and links | Corrected | The recent-closure table had retained stale T301--T307 rows while omitting accepted T308--T311. Td S57 restores the latest eight closures (T305--T312). T303--T308 evidence plus T310/T311 history now record former pending state as historical rather than current. |
| System-extension candidates | Deferred | T312 found existing handlers and focused probes but no real VM/M5 consumer. The candidate remains withdrawn; per-family future checkpoint/trace requirements are in [T312 evidence](t312-system-extension-admission.md) and TODO. |
| M6 mantle | Deferred | Queue candidates remain unadmitted. No package result creates a mantle session, second executor, public facade, or M6 implementation permission. |

## Conclusion

No confirmed, independently remediable T301--T312 runtime or boundary defect
requires T313. The only confirmed issues were documentation lifecycle and
recent-closure drift; Td S57 corrects them in place. M5 is package-complete
for this direct 80386 scope. Any later work requires a separately approved
candidate and may not infer M6 admission from this audit.
