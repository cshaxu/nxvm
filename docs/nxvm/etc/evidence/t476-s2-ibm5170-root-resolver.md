# T476 S2 IBM 5170 Root Resolver

`M5:T476:S2:IBM5170-ROOT-RESOLVER:OK`

`M5:T476:S2:IBM5170-ROOT-NEGATIVE:OK`

## Retained Owner And Data Flow

`ibm_5170_model_339_descriptor` remains the sole selected-machine source while
the existing session route remains live.  The new `pc-at-5170` declaration is
created from that descriptor on demand, then `profile_resolver` produces a
wholly copied `vm_resolved_profile`.  The root result also owns an in-place
deep copy of every PC/AT descriptor scalar plus its port leaves, routes and
firmware-service list; its descriptor points only to those copied arrays. The
copied Core configuration and controller rules use the same profile-owned
materializer as the current session route; no alternate Core-plan builder,
mutable Core pointer or profile-name branch was added.

The earlier continuous port-window model could not represent this machine:
the board has discontiguous ports and direction-specific leaves.  It is
therefore replaced, not supplemented, by copied port leaves.  An absent gap
does not become an invented device decode.

## Ledger Mapping

| Frozen group | Root result | Disposition |
| --- | --- | --- |
| CPU, RAM, Core time and controller rules | Copied `core_machine_config` and controller rules. | 80286/512 KiB; 8 MHz remains L2 macro pacing; PIT/RTC rational clocks and DMA service provenance remain as recorded in S1. |
| Ports and display memory | 63 enabled copied port leaves, each with read/write direction; CGA `B8000h-BFFFFh` window. | HDC and EGA leaves stay absent; no interval inference. |
| IRQ/DRQ | Five IRQ routes and one FDC DRQ route. | Copied board topology only; controller state remains Core-owned. |
| Firmware, CMOS and media | Copied firmware slot/services, ROM mapping, CMOS fields; built-in abstract-firmware policy and session media policy with no allowed optional request bit. | No ROM/media payload or HDD policy is introduced. |
| Dynamic resources | Not represented. | BIOS objects, registries, presentation and debugger retain their existing lifetime owners. |

## Validation

`vm-ibm-5170-root-resolver-smoke` proves root identity, exact active leaf
sequence/directions, copied descriptor/ROM/CMOS/firmware-service data,
selected timing facts, CGA mapping, routes, policy and the existing copied
Core-plan input handoff. It then rejects duplicate port, duplicate IRQ and
unavailable contract mutations before any session is created.
`vm-profile-resolver-smoke` remains green after the leaf replacement.

The current-gate owner-test inventory increases from 178 to 179 pure targets;
the mixed count remains three.  The T345 static verifier correspondingly moves
its owner-test source count from 181 to 182 and uses the same 179 target count.
This adds one focused owner-boundary smoke rather than a second runtime route.

## Transfer

S3 alone may make the IBM session consume this resolved root and remove the
old direct Model-339 selection/materialization route after the S1 parity suite
proves equivalence.  DeskPro and default-at remain outside this S.
