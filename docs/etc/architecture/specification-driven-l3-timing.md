# Specification-Driven Instruction And Transaction Timing Simulation

`M5:Td:S108:CORE-L3-TIMING-DESIGN:OK`

## Role

This supporting design records the current Project A architecture: make every
admitted Core instruction, controller, device and integration capable of
specification-driven instruction and transaction timing when the available
contract supports it. It elaborates the Core/VM boundary in
[System Architecture](../../design/ARCHITECTURE.md); it does not define task
state, Queue order, a machine profile, or a physical-hardware claim.

## Timing Levels

| Level | Project definition |
| --- | --- |
| L0 | No Core-owned logical order is defined for the capability. |
| L1 | The functional causal order is defined, but no numerical model, proportional estimate or admitted timing input supplies the missing timing behaviour. |
| L2 | Implemented macro-scale proportional timing. The shared virtual clock preserves deterministic relative rates, event ordering, IRQ/DRQ lifecycle, acknowledgement, reset and cancellation, but does not claim instruction- or transaction-specific manual timing. |
| L3 | The current project target: documentation-driven instruction- and transaction-level timing. Each admitted rule is a documented constant, formula, or bounded range with a declared deterministic selection; a clearly labelled reference-derived contract may fill a documented gap. L3 is scheduled through L2's shared virtual clock. |
| L4 | Hardware-level real timing: electrical waveforms, propagation and measurement correlation. It is not NXVM's standard, is not an L3 prerequisite, and is prohibited from implementation scope. |

L3 is neither a guest-OS checkpoint nor a claim that every undocumented
physical detail is known. Every manual number or formula is a Manual-L3 source
fact even if its current Core-tick conversion is only L2; a component can
therefore contain both dispositions for separate parts of one capability. L4
is never a fallback, a completion condition, or a later Project A work item.

## Coverage Universe And Completion Rule

The coverage universe is every mechanism currently exposed by Core's public
machine contracts, plus every future Core mechanism before VM can select it:

1. CPU instruction execution, exception/interrupt delivery, prefetch and
   external memory/port transactions;
2. clock domains, event scheduler, retirement, reset, cancellation and stop;
3. RAM, ROM, A20, address-window, parity and board-control integrations;
4. PIC, DMA, PIT, RTC/CMOS, KBC, NMI and speaker integrations;
5. FDC, HDC, media transfer and their IRQ/DRQ/result lifecycles;
6. VADP/CGA/EGA/CECG display access, scan/status and presentation boundaries;
7. every inter-device route: port decode, memory decode, IRQ, DRQ, HOLD/HLDA,
   BUSRDY, availability wait and arbitration.

For each capability, the durable ledger records one of only these states:

- **L3 implemented and proved**: contract, rule, reset/cancellation proof and
  regression are named;
- **L2 fallback**: the functional/proportional contract is named, the missing
  L3 fact is described, and a future admission source is named;
- **not applicable**: the selected mechanism cannot express the capability,
  with a reason; or
- **not supported**: the capability is not public and cannot be selected by a
  VM profile.

An L3-capable Core mechanism is complete only when every capability in its
frozen ledger has one of those dispositions. A machine profile can claim L3
only when each selected capability is L3 or has an owner-visible, accepted L2
fallback exception. No silent generic timing is permitted.

## Core Timing Contract

Core remains machine-neutral. It owns execution, state machines, virtual time,
transaction lifecycle and state publication. VM passes a copied, immutable,
validated timing plan; it never drives controller state by a profile callback.

A future Core timing-plan entry has these conceptual fields:

| Field | Meaning |
| --- | --- |
| Capability ID | Stable neutral mechanism/capability name, never a machine name. |
| Scope | CPU profile/form/context, controller command/phase, route or address window. |
| Rule kind | `constant`, `formula`, `bounded-range`, `reference-contract`, or `l2-fallback`. |
| Inputs | Explicit architectural or transaction inputs such as prefix, branch, transfer count, bus width, address class, request source or selected mode. |
| Output | Core ticks, phase transition, wait condition, arbitration result or event deadline. |
| Lifecycle | Begin, wait, commit, acknowledge, deassert, cancel and reset semantics. |
| Qualification | Rule version, evidence tier and profile-independent contract identifier. |

A bounded range never permits host-time randomness. The selected plan must
state its deterministic rule: a documented mode, exact chip configuration,
minimum/maximum contract, or a reference-derived choice. Core records the
result through its normal trace/observation path so focused tests can observe
both timing and state effects.

### Required Core Interfaces

The target is an additive, copied `core_machine_plan` boundary that validates
all selected timing and topology entries before it publishes a machine. Existing
`core_machine_config` and `core_machine_configure_*` structures are its source
material, not a second long-term composition path.

The plan must provide:

- rational clock-domain ratios and reset phases;
- CPU instruction/form timing-program selection;
- external transaction classification, availability waits, BUSRDY and
  arbitration contracts;
- device topology, port/memory decode, IRQ/DRQ/HOLD routes and phase rules;
- reset/cancellation ownership and trace/observation declarations; and
- immutable copied data with no caller-lifetime dependency after construction.

Core validates type/range/topology consistency. VM validates machine/profile
identity, inheritance, provenance and user policy. Neither layer may validate
by executing a partial machine.

## Current Core Position

| Area | Present capability | L3 gap / L2 fallback discipline |
| --- | --- | --- |
| CPU | 8086, 80186, 80286 and 80386DX execution profiles; shared virtual time; selected instruction surcharges and retirement controls. | Replace coarse scalar/surcharge timing with a full form/context program. Every unallocated manual row remains L2 or explicitly deferred; 80486/P5 are not supported Core profiles. |
| Transaction and clocks | Rational DMA/PIT/RTC/VADP/KBC/provider clocks; CPU external-cycle observations; bounded wait windows; BUSRDY and deterministic cancellation. | Generalize selected board waits into declarative transaction/arbitration contracts; no inferred asynchronous producer. |
| Memory/ROM | Checked RAM, immutable ROM, A20, address windows, parity and selected board controls. | Memory-controller, cache, shadow/PAM and later host-bridge timing require distinct neutral mechanisms before any profile may select them. |
| PIC/DMA/PIT/RTC/KBC | Shared controller owners and common AT topologies, IRQ/DRQ and reset paths. | Each controller command/phase/routing timing row needs an explicit ledger disposition; unknown rates retain L2. |
| FDC/HDC | Core media/controller paths and selected FDC/HDC policies. | Complete command/error/recovery phase contracts by controller personality; motor/media/electrical facts without a contract remain L2. |
| Display | VADP, CGA/EGA/CECG state and selected access waits. | Scan, raster/status and monitor timing become capability rows; a port wait alone is not display L3. |
| Board integrations | Selected D4, planar parity, speaker, refresh/HOLD and reset bridges. | Each bridge must identify producer, consumer, phase and source rule; undocumented electrical details retain L2 treatment; L4 reproduction is prohibited. |

This table is a design inventory, not an assertion that the listed L3 gaps are
closed. Detailed task evidence remains indexed separately.

## VM Selection Boundary

VM profiles select Core contract IDs and topology values; they do not contain
timing algorithms. A resolved profile may state, for example, that a CPU form
uses an Intel-derived timing program, an FDC unready READ uses a
reference-derived completion contract, and an otherwise inherited controller
phase remains an explicit generic-AT L2 fallback.

The Core-visible plan contains neutral values only. The VM-only resolved
profile retains machine identity, inheritance, provenance tier, evidence links
and user-session policy. This keeps Core free of `5170`, `DeskPro`, `XT` or
firmware names while giving VM enough information to reject a profile that
requests an unavailable contract.

## Exception Ledger And Admission

The future Core L3 implementation program maintains one finite ledger per
admitted mechanism. A L2 exception must include: capability ID, selected Core
owner, current L2 behavior, missing L3 fact, evidence tier, regression,
risk, and earliest admission path. It is removed only by an L3 implementation,
not by a successful guest boot.

A new VM profile cannot turn an absent Core contract into profile-local logic.
It either selects a registered L3 contract, explicitly selects an approved L2
fallback, or is rejected during resolution. A new CPU, chipset, bus or device
requires a separate Core admission before it can be a runnable profile
capability.

## Implementation Sequence

1. Freeze the coverage ledger and timing capability IDs for existing Core
   mechanisms; record every initial L2 exception.
2. Add the copied Core machine-plan validation boundary without changing
   existing timing behavior.
3. Introduce CPU form/context timing programs and transaction/arbitration
   contracts through the single Core execution path.
4. Convert controller/device capabilities one frozen ledger batch at a time,
   retaining explicit L2 entries where specifications are unavailable.
5. Expose only validated contract IDs to the later VM profile resolver.
6. Admit additional CPU generations, chipsets or buses only with a new neutral
   Core contract, coverage ledger and regression corpus.

## Acceptance Boundary

Project A does not accept L4 work, a universal clone claim, or every possible
historical peripheral. Hardware-level real-time reproduction is outside this project
and must not be implemented. Project A accepts a bounded Core capability only when its
selected functional surface has a deterministic L2 contract, its available
specification-driven timing has been implemented as L3, and every remaining
case is explicitly recorded as L2 fallback or unsupported.
