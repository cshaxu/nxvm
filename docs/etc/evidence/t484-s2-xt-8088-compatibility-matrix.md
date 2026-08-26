# T484 S2 IBM PC/XT 5160-268 8088 Compatibility Matrix

`M5:T484:S2:XT-8088-COMPATIBILITY:OK`

`M5:T484:S2:XT-B1-SOURCE-BOUNDARY:OK`

## Source Check

Intel, *iAPX 86, 88 User's Manual* (1981), Chapter 2 was cross-read from its
OCR layer and rendered pages 2-1, 2-5 and 2-6.  The rendered text confirms
the key distinctions below: the 8088 has an 8-bit external memory/I/O path;
the EUs are identical; the BIUs are functionally alike but implemented for
their respective buses; the 8088 instruction queue is four bytes while the
8086 queue is six bytes.  The IBM 5160 technical reference independently
selects maximum mode and the 4.77 MHz board attachment.  OCR is not used as
the sole authority for a numeric or queue rule.

## Reuse And Separation Matrix

| B1 concern | Manual/IBM fact | Current source disposition | Required implementation boundary |
| --- | --- | --- | --- |
| Architectural instruction semantics | Intel says the EUs are identical and the processors are software-compatible. | The retained 8086 decode/execution and real-mode state are Core-owned; no 8088 profile is admitted. | Reuse the 8086 semantic owner; add no copied decoder, opcode table or state mirror.  Regression compares selected real-mode semantic checkpoints under 8086 and 8088 profiles. |
| General registers, flags, address formation and reset-vector semantics | Intel shows the same 16-bit EU/register architecture and the same 20-bit addressable memory model. | Existing Core CPU/memory/reset owners provide these generic mechanisms. | Reuse those owners through one 8088 configuration value.  The XT descriptor fixes 256 KiB and no FPU; it cannot expose a mutable CPU choice. |
| External data path | Intel: 8088 uses an 8-bit external data path to memory and I/O; 8086 transfers 16 bits at a time. | No `CORE_MACHINE_CPU_PROFILE_8088`; current board/profile code is not an 8088 attachment. | Add a distinct 8088 profile/configuration branch at the existing CPU/board transaction seam.  It cannot alias the 8086 profile, and B2 owns the actual 5160 board route. |
| Prefetch queue | Intel: 8088 queue holds four bytes; it fetches a byte when a queue byte is empty and no EU bus request is active.  8086 has six bytes and a different refill threshold. | `core_machine_cpu_execution_context` has a generic 15-byte prefetch buffer; its reservation comment explicitly withholds a source-backed producer contract. | Do not relabel the 15-byte buffer as 8088.  B1 must either add the source-qualified 8088 queue policy inside the sole Core CPU owner, with focused control-transfer/self-modifying-code proof, or keep the 8088 runtime profile unavailable and transfer the missing producer as a blocking B1 correction. |
| Queue flush/control transfer | Intel describes queue reset and refetch after an EU control transfer. | Core already owns prefetch invalidation, but its correctness is against the generic buffer, not the four-byte 8088 contract. | Audit and extend the same invalidation owner with the 8088 queue policy; no VM/profile queue state. |
| Instruction timing additions | Intel Table 2-21 gives an extra four clocks per 16-bit word transfer for 8088, unlike the 8086 odd-address addition. | Current 8086 timing ledger explicitly leaves prefetch and bus availability outside its owner. | Preserve as a Manual-L3 input for the later XT timing candidate.  B1 does not convert it into guest elapsed time or host pacing. |
| Maximum mode and 5160 clock | IBM selects 8088 maximum mode at 4.77 MHz; Intel describes maximum mode as an external bus-controller configuration. | No 5160 plan exists; current time/profile contracts do not qualify an XT physical axis. | B1 records a fixed construction fact only.  B2 binds the board protocol; the XT timing candidate integrates any source-qualified cycle relation. |
| FPU and user selection | The accepted 5160-268 BOM selects no FPU and fixes the machine CPU. | Current default-AT has a declared CPU/FPU choice route. | XT request/profile validation must reject CPU/FPU overrides and hold `FPU_NONE`; it must not inherit default-AT choice semantics. |

## B1 Implementation Contract

1. Extend the existing CPU profile domain and validation to represent an 8088
   as a distinct Core configuration, but keep semantic execution in the
   existing shared owner.
2. Add the 8088 queue policy only in that owner; delete no valid 8086 behavior
   and create no parallel fetch/decode path.  It must be source-tested at the
   control-transfer and self-modifying-code boundary.
3. Add the fixed, immutable XT declaration through the existing resolver and
   copied-plan route.  It fixes 8088/256 KiB/no-FPU and rejects AT-only
   selection.  It does not yet bind the 5160 board or make a runnable XT
   session available.
4. Keep all external-bus cycle cost, 4.77 MHz conversion, device timing and
   host pacing out of B1.  Those facts have exactly one receiver: the later XT
   board/device timing closure through the existing Core time axis.

This matrix resolves every B1 source question needed before implementation.
It identifies one material correction rather than allowing a false minimal
patch: an 8088 profile is not acceptable while its visible prefetch policy is
secretly the current generic 15-byte cache.
