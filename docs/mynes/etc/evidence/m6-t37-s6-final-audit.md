# M6 T37 S6 Final App/Core Audit

## Scope and method

The audit re-read only product-owned `src/app`, `src/core`, their direct tests
and the two completed T37 repairs. It searched lifecycle fields, state-stream
callers, snapshot serialization, allocation use, callback ownership and direct
test receivers. Shared Lib/Common source was not changed.

## Findings and disposition

| Finding | Owner | Evidence | Disposition |
| --- | --- | --- | --- |
| Raw-layout snapshot bytes and a mismatched PRG-RAM write length | Core | S1 audit; S4 codec and Core test | Fixed in S4. |
| Save/load success text preceded the runtime completion it claimed | App | committed SoftPC `6251f896`; S5 integration test | Fixed in S5. |
| Nonrunning Window creation, atomic replacement and shared-source divergence | Shared boundary | S3 committed corpus audit | Fixed in S3; not reopened here. |
| Remaining product-owned forwarding-only layer or unowned state | App/Core | S6 source and direct-test sweep | None found. |

## Architectural conclusion

`App` owns command grammar, prompt timing and its single pending snapshot
result. `Core` owns deterministic machine-state bytes, mapper state and host
output reset. `Common` remains the sole runtime transition boundary; `Lib`
remains the host facility layer. No additional callback or state carrier is
needed.

## Verification record

The S4 and S5 direct snapshot Core, transaction and App command tests pass on
x64 and x86. The fixed v2 payload length/hash assertion also passes directly
on both targets. The complete sequential suites pass without failures:

| Target | Result | Elapsed |
| --- | --- | --- |
| x64 | 118 / 118 passed | 174.85 s |
| x86 | 118 / 118 passed | 184.77 s |
