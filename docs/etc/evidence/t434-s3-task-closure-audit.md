# T434 S3: Core Timing Plan Task-Closure Audit

`M5:T434:S3:CLOSURE-AUDIT:OK`

## Closure predicate

This audit independently reviews the accepted T434 deliveries rather than
treating their prior smoke summaries as proof.  Its complete predicate is the
T434 proposal and Td S117 ledger: every frozen capability has one validated
default declaration/disposition and usable seam; all current VM families use
the one copied plan publisher; invalid and required-L3 requests fail before
publication; and the default families retain their deterministic lifecycle,
device and observation behavior.

## Requirement-to-proof review

| Requirement | Current source and proof | Result |
| --- | --- | --- |
| Exact 30-ID universe and unique default disposition | `core_machine_timing_capability` has 30 contiguous IDs; `core_machine_plan_initialize()` initializes all, and `core_machine_plan_validate()` rejects a duplicate, missing declaration, wrong seam, wrong NGT disposition or every unavailable L3 request. The strengthened `core_machine_plan_smoke` independently checks every expected ID/seam/disposition and each invalid L2/NGT substitution. | Pass |
| Neutral usable consumer seams | The copied plan contains the existing Core CPU/retirement, clock, lifecycle, transaction, memory/configuration, device-topology and observation seams. `core_machine_config` remains copied input material; plan declarations classify every owner without a VM callback or profile branch. | Pass |
| Atomic construction and copied lifetime | `core_machine_create_from_plan()` validates first, creates privately, applies topology in dependency order, destroys on every apply error and sets the output null. The plan smoke covers missing, duplicate, wrong-seam, unavailable-L3 and invalid topology rejection plus post-create declaration copy. | Pass |
| One production publication route | `rg -n "core_machine_(create|configure_)\\(" src/vm` has no result. `src/vm/composition/session/session.c` and `model40_composition.c` each use only `core_machine_create_from_plan()`. Remaining direct construction is confined to tests/support and the explicitly non-runnable `src/vdm/machine/dos_minimal.c`. | Pass |
| Three materialization families | Default PC/AT and IBM 5170 Model 339 flow through the shared session plan materializer; private Model-40 BYOB builds the same plan type before its sole publication. Default atomicity, Model-339 composition and Model-40 BYOB smokes pass. | Pass |
| Reset, cancellation, IRQ/DRQ, parity and observation equivalence | The focused D4 parity, FDC/DMA, host cancellation and CPU trace smokes pass alongside the three family smokes. They cover reset/rollback, IRQ/DRQ binding, D4 memory parity/IOCHK behavior, cancellation and trace context. | Pass |
| Core/VM ownership and coding review | Core includes no VM source/header or `vm_profile`/Model-40/5170 selector. Its retained `pc_at` names identify the generic port-B mechanism and do not select a VM profile. VM supplies copied topology/provider facts; Core owns validation, publication, controller binding and cleanup. The S3 test is owner-local, uses no public test-only API and replaces an insufficient sample check with the full finite ledger check. | Pass |

## Executed verification

All executable smokes linked the full non-Linux source graph with GCC 16.1.0
and Win32 host libraries. The affected S3 test also passed C11
`-Wall -Wextra -Wpedantic -Werror -fsyntax-only`.

| Test | Result markers |
| --- | --- |
| `core_machine_plan_smoke` | `M5:T434:S1:PLAN-DECLARATIONS:OK`; `M5:T434:S1:PLAN-VALIDATION:OK`; `M5:T434:S1:PLAN-COPY:OK`; `M5:T434:S2:ROLLBACK-EQUIVALENCE:OK`; `M5:T434:S3:ALL-DECLARATIONS:OK` |
| `vm_session_initialization_atomicity_smoke` | `M5:T300:S3:SESSION-INITIALIZATION-ATOMICITY:OK`; `M5:T332:S1:SESSION-CONFIG-MATERIALIZATION:OK`; `M5:T332:S2:SESSION-CONSTRUCTION-TRANSACTION:OK` |
| `vm_ibm_5170_model_339_composition_smoke` | `M5:T366:S5:MODEL339-COMPOSITION:OK`; `M5:T380:S2:MODEL339-NO-XMS-PROBE:OK`; `M5:T421:S1:IBM5170-SHARED-SPEAKER:OK` |
| `vm_model40_byob_s20_smoke` | `M5:T386:S20:MODEL40-BYOB-MANIFEST:OK`; `M5:T386:S20:MODEL40-BYOB-VALIDATION:OK`; `M5:T386:S20:MODEL40-PUBLIC-COMPOSITION:OK`; `M5:T424:S1:MODEL40-BYOB-RESET-LIFECYCLE:OK` |
| `vm_model40_d4_parity_s22_smoke` | `M5:T386:S22:D4-PARITY-DIAGNOSTIC:OK`; `M5:T386:S22:D4-IOCHK-CLEAR:OK`; `M5:T386:S22:MEMORY-PARITY-OWNER:OK` |
| `vm_fdc_dma_boundary_smoke` | `M5:T348:S4:FDC-DMA-BOUNDARY:OK`; `M5:T380:S2:MODEL339-512K-FDC-START:OK` |
| `vm_host_cancellation_smoke` | `M5:T201:S3:HOST-CANCELLATION:OK` |
| `cpu_trace_context_smoke` | `M5:T88:S1:TRACE-CONTEXT:OK` |

The normal full-source strict link remains blocked by pre-existing warnings in
unmodified debug, firmware, session and CPU files. This audit does not relabel
that existing repository-wide diagnostic debt as T434 failure: the changed S3
test compiles strict, every affected S2 source passed the prior focused strict
syntax check, and all behavior tests above linked the full source graph.

## Change and artifact reconciliation

From pre-T434 commit `f0e145d6` through this closure, counted tracked
`src/` and `tests/` paths are `+752/-103`, net `+649` lines, calculated with
`git diff --numstat f0e145d6..HEAD -- src tests` plus the S3 working change.
The increase is material and cohesive: one copied validated publication
contract replaces the two VM publishers and post-create configuration routes;
S3 adds a finite 30-row regression. No obsolete production publisher remains.

The current developer artifact remains source-equivalent to accepted S2:
`vm-0-5-0434` / `nxvm_0_5_0434.exe`, SHA-256
`0252F8FDA17BEC2131606F19E3547B46894AC6B56DD37EC3B16BD302494FAFDC`.

No T434 seam, capability, production family, ownership conflict or
unimplemented task-local condition remains. Later Queue candidates may add
sourced L3 rules only through this closed boundary; they may not reintroduce a
publisher or consumer seam.
