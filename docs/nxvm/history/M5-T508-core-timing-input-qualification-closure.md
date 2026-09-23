# M5 T508: Core Timing-Input Qualification Closure

T508 closed.  It closes the finite construction-time timing-input universe
for the current four product profile families.  Core remains the sole guest
time writer and deadline composer; profiles provide one copied, immutable
construction plan and never mutate controller timing at runtime.

## Accepted Subtasks

| Subtask | Accepted result |
| --- | --- |
| S1 | Accepted: the complete four-profile copied-input ledger finds no active built-in L1 route, one source-neutral HDC naming repair, and one inseparable unqualified generic/profile axis class for S3. |
| S2 | Accepted: `service_ticks` is one provenance-neutral copied HDC input at every Core, VM and test call site; the former L2-only term and compatibility alias are removed. |
| S3 | Accepted: ATA-3 and external models provide no portable default-PC/AT service duration, and the current Model-40 elapsed axis is not a CPU-cycle axis; both retain one explicit unqualified boundary. |
| S4 | Accepted: one table-driven four-profile test proves the plan-copy route turns each supported pending DMA request into a deadline, not L1; unit 313/313, integration 20/20, governance and stripped Release 0508 pass. |
| Corrective S5 | Accepted: local 86Box and Compaq/86Box evidence qualify the default-PC/AT ATA `200`-tick controller quantum and Model-40 16-MHz macro pacing input as Other-L2. The existing INT 13h and ROM unit consumers now wait for the sole Core HDC deadline rather than assuming immediate DRQ. |

## Completion Standard

Every frozen input must have a source-qualified disposition and owner route.
Built-in profiles may not trigger the bounded L1 compatibility escape.  Any
unsourced duration remains an immediate/unsupported boundary rather than a
guessed number.  T508 closes only after complete unit, external-asset
integration, governance, and stripped Release 0508 verification.

## Closure Result

Core still has one copied timing-plan route and is the only guest-time writer.
The only production naming correction is the provenance-neutral HDC
`service_ticks` input; the final matrix test adds no product path, mutable
setter, controller mirror or VM-to-Core tick path. The test uses the same
plan-copy construction path as product composition, so direct raw creation
cannot accidentally validate a profile timing claim.

T508's original tracked source/test/CMake delta from `67cd07a6` is 158 added
lines, 11 removed lines, net +147. Corrective S5 adds 38 and removes 6 tracked
source/test lines from `1fa4c35b` through `78b14f45`; documentation is
excluded. Core contains no machine-profile selector: VM resolves its profile,
copies a frozen generic plan once, and Core owns deadline/time consumption.
Hardware-personality enums in Core name controller behavior, not a machine
profile or its timing source. No built-in profile enters L1 under the S4/S5
matrices.
