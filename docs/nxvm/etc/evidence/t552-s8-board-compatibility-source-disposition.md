# T552 S8 — Core Board Compatibility Source Disposition

## Compaq CECG B0000h Compatibility Window

The implementation has one CECG planar-memory owner in `core/chips/video`.
When the selected Compaq colour personality has map 3 active, the narrow
`B0000h..B7FFFh` compatibility window canonicalizes to the selected `B8000h`
planar window. It therefore cannot fall through to ordinary RAM or create a
second video store.

The established Compaq *Enhanced Color Graphics Board / Color Monitor
Technical Reference Guide* evidence in T386 S6, S11 and S12 supports the
selected CECG personality, identity registers, colour I/O base and CPU-video
gate. It does not independently establish every firmware POST write observed
by this compatibility mapping. The retained mapping is therefore an existing
bounded compatibility contract, not a new physical-board or firmware claim.
The existing CECG chip and board tests exercise its sole storage and route
owner. No source-grade basis exists in this S to delete, broaden or relabel it.

## PC/AT Bounded L1 Progress

`CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS` is explicitly a construction
policy, not a machine duration. Existing T504 S3 evidence records that each
selected PC family sets it once; Core then re-observes board state for at most
16 one-tick normal scheduler transitions. Immediate work and source-qualified
deadlines retain priority, Standard never calls the escape and physical
retirement configurations reject it.

The policy belongs to the shared PC/AT materializer because it is a host
control bound for an otherwise unsourced owner. It is not evidence of an IBM
or Compaq hardware timing fact. `core-machine-attachment-phases-smoke` and
`core-machine-time-smoke` prove the copied observation and bounded action
without a controller-specific fallback.

## Disposition

| Receiver | Result |
| --- | --- |
| CECG B0000h alias | Retain its one VADP storage owner and existing bounded compatibility behavior. Transfer any physical firmware/POST equivalence claim to future source-backed work. |
| PC/AT bounded L1 | Retain as declared Core control policy. No timer, guessed duration, mutable board state or profile-specific duplicate is admissible. |

## Verification

The six CECG chip tests, three CECG board tests, Core attachment-phase and
Core time tests pass on x64 and x86. Source and test manifests plus the Core
corpus gate also pass. This S changes no production code, public API, asset or
artifact input.
