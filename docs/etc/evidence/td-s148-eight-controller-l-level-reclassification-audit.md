# Td S148: Eight-Controller L-Level Reclassification Audit

## Decision

This audit supersedes only the **L-level vocabulary** in the listed frozen
controller checklists and their later closure summaries.  It does not rewrite
closed history, change a manual fact, promote an implementation by resemblance,
or add a timing path.

The owner-approved hierarchy is applied to all 145 checklist rows below:

| Level | Meaning in this audit |
| --- | --- |
| L3 | A direct manual fact (including every manual number, period, frequency or formula), a separately labelled external-model fact, or an existing external/board input copied to the sole Core owner.  An input receiver is L3 even when a present profile selects an estimated value. |
| L2 | Only an internal proportional or fixed numerical estimate remains.  An unsourced conversion of a Manual-L3 number/formula into Core ticks is L2 data; the manual fact and its existing input receiver are not. |
| L1 | The owner has a correct causal/logical order, but no numerical model, proportional estimate, or existing input receiver supplies the missing timing/behaviour. |
| L0 | No Core-owned logical order is defined. |
| L4 | A source-backed electrical, waveform, propagation, mechanical or monitor-level timing fact.  It is more precise than L3, remains explicitly out of the current implementation scope, and is never relabelled as L0 merely because Core has no discrete consumer. |

`Manual L3`, `Other L3`, and `Input L3` are provenance labels within L3.
`Input L3` requires an existing one-way route from composition to the sole
owner; a merely possible future setter does not qualify.  A profile's selected
value has a second, independent provenance: for example, a source-derived
Model-339 ratio is L3 while an unsourced default placed in the same receiver is
L2.  L4 facts remain explicit physical exclusions rather than L0.

In particular, a missing Core tick conversion never demotes an Intel, IBM,
Motorola or ATA numeric value/formula itself: the source fact remains Manual
L3.  Only the separate, unsourced Core-tick conversion is assessed as L2.

## Method And Existing Receiver Sweep

The frozen universes are the original Checklist 1 rows and their code-gap
audits for PIC, DMA, PIT, RTC, KBC, FDC, VADP/EGA, VADP/CGA and ATA/HDC.  VADP
is one controller domain, split here into its two required personalities.  The
row count is therefore 145: PIC 18, DMA 16, PIT 17, RTC 16, KBC 16, FDC 15,
VADP/EGA 15, VADP/CGA 17 and ATA/HDC 15.

The code sweep establishes these actual, one-owner external inputs:

| Receiver | Existing one-way path | Classification consequence |
| --- | --- | --- |
| DMA/PIT/RTC controller rule and rational-clock plan | profile descriptor -> session copy -> validated `core_machine_plan` -> Core clock domain | Input L3.  A selected fallback ratio remains L2 data. |
| RTC update scale | composition -> copied `core_machine_rtc_timing_plan` -> RTC | Input L3. |
| KBC response, serial and typematic values | profile contract -> copied `core_machine_config` -> KBC at Core construction | Input L3. |
| VADP display topology and text/raster declaration | profile/composition -> copied display config -> VADP | Input L3. |
| FDC port/IRQ/DMA/media topology | composition -> copied plan topology -> FDC | Input L3 for topology and direct DMA/IRQ wiring, not for mechanics or seek duration. |
| ATA/HDC port, IRQ, protocol and media topology | composition -> copied plan topology -> HDC/media owner | Input L3 for those selected facts, not for an absent ATA service-time term. |

`core_machine_controller_timing_rules.pic_visibility` is deliberately
validated as fallback-only in `machine_plan.c`; it is not an L3 receiver.
Likewise, FDC has a fixed `CORE_MACHINE_FDC_SEEK_TRACK_TICKS` constant and no
construction-time seek/rotation plan, while HDC has no service-time plan.

## Identifier Completeness Key

The grouped disposition rows below are exact-row groups, not new aggregate
requirements.  This key makes their complete membership mechanically visible:

| Domain | Every audited frozen identifier |
| --- | --- |
| PIC | PIC-R1, PIC-R2, PIC-R3, PIC-R4, PIC-R5, PIC-F1, PIC-F2, PIC-F3, PIC-F4, PIC-F5, PIC-F6, PIC-F7, PIC-F8, PIC-T1, PIC-T2, PIC-T3, PIC-T4, PIC-T5 |
| DMA | DMA-R1, DMA-R2, DMA-R3, DMA-R4, DMA-F1, DMA-F2, DMA-F3, DMA-F4, DMA-F5, DMA-F6, DMA-F7, DMA-T1, DMA-T2, DMA-T3, DMA-T4, DMA-T5 |
| PIT | PIT-R1, PIT-R2, PIT-R3, PIT-R4, PIT-F1, PIT-F2, PIT-F3, PIT-F4, PIT-F5, PIT-F6, PIT-F7, PIT-F8, PIT-T1, PIT-T2, PIT-T3, PIT-T4, PIT-T5 |
| RTC | RTC-R1, RTC-R2, RTC-R3, RTC-R4, RTC-F1, RTC-F2, RTC-F3, RTC-F4, RTC-F5, RTC-F6, RTC-F7, RTC-T1, RTC-T2, RTC-T3, RTC-T4, RTC-T5 |
| KBC | KBC-R1, KBC-R2, KBC-R3, KBC-R4, KBC-F1, KBC-F2, KBC-F3, KBC-F4, KBC-F5, KBC-F6, KBC-F7, KBC-T1, KBC-T2, KBC-T3, KBC-T4, KBC-T5 |
| FDC | FDC-R1, FDC-R2, FDC-R3, FDC-R4, FDC-R5, FDC-F1, FDC-F2, FDC-F3, FDC-F4, FDC-F5, FDC-T1, FDC-T2, FDC-T3, FDC-T4, FDC-T5 |
| VADP/EGA | VADP-R1, VADP-R2, VADP-R3, VADP-R4, VADP-R5, VADP-F1, VADP-F2, VADP-F3, VADP-F4, VADP-F5, VADP-T1, VADP-T2, VADP-T3, VADP-T4, VADP-T5 |
| VADP/CGA | CGA-R1, CGA-R2, CGA-R3, CGA-R4, CGA-R5, CGA-F1, CGA-F2, CGA-F3, CGA-F4, CGA-F5, CGA-F6, CGA-F7, CGA-T1, CGA-T2, CGA-T3, CGA-T4, CGA-T5 |
| ATA/HDC | ATA-R1, ATA-R2, ATA-R3, ATA-R4, ATA-R5, ATA-F1, ATA-F2, ATA-F3, ATA-F4, ATA-F5, ATA-T1, ATA-T2, ATA-T3, ATA-T4, ATA-T5 |

## Row-Complete Reclassification

Every row retains its direct manual/Other L3 semantic claim unless the table
names a residual component.  `L3 only` means the entire row is already covered
by its cited direct/manual or labelled-model relation.  When a row has more
than one component, the residual column is deliberately separate from that
source fact.

### PIC 8259A - 18 rows

| Rows | Final classification of the row's residual | Reason and owner |
| --- | --- | --- |
| PIC-R1 | L1 visibility order | IRR/priority/INT/INTA order is Manual L3; no PIC visibility field or numeric estimate exists. |
| PIC-R2--R4, PIC-F2, PIC-F4--F5, PIC-T1, PIC-T4 | L3 only | Direct 8259A/AT state, ordering or topology. |
| PIC-R5, PIC-F3, PIC-F6, PIC-T3 | L4 physical exclusion | Bus, INTA and CAS waveform facts remain source-backed physical timing. |
| PIC-F1, PIC-F7, PIC-F8, PIC-T2 | L1 delivery order | Manual causal order exists; no elapsed PIC-to-CPU term is estimated or received. |
| PIC-T5 | L1 | The required ownership boundary is known, but the present plan rejects a PIC timing input. |

### DMA 8237A - 16 rows

| Rows | Final classification of the row's residual | Reason and owner |
| --- | --- | --- |
| DMA-R1 | L1 board scheduling | Register programming is Manual L3; no board service duration is inferred. |
| DMA-R2, DMA-F4 waveform part | L4 physical exclusion | Program/bus pulse values remain source-backed physical timing. |
| DMA-R3--R4, DMA-F3 semantic part, DMA-T1--T2 | L3 only | Direct register, mode and AT topology facts. |
| DMA-F1, DMA-F5, DMA-T4 digital DREQ/DACK/EOP part | Input L3 | Existing DMA/device and FDC bindings deliver the named digital event to the sole DMA owner; pulse width remains L4. |
| DMA-F2, DMA-F6--F7, DMA-T3 | Input L3; selected-value split | The clock/service receiver is real.  Model-339's qualified rational clock/service rule is L3; any fallback ratio is L2 data, not an L2 receiver. |
| DMA-F3 interleave part | L1 | Service order is defined, but no universal CPU interleave duration is supplied. |
| DMA-T5 | Input L3 | The copied controller-rule plan is the existing request/grant/page/refresh qualification receiver. |

### PIT 8254 - 17 rows

| Rows | Final classification of the row's residual | Reason and owner |
| --- | --- | --- |
| PIT-R1, PIT-R3, PIT-F2--F7, PIT-T1 | L3 only | Direct 8254 state/mode/gate/output and AT wiring facts. |
| PIT-R2, PIT-R4, PIT-F1, PIT-F8, PIT-T4 | L4 physical exclusion | Bus/edge/range/AC values remain source-backed physical timing. |
| PIT-T2 | Input L3; selected-value split | The PIT rational-clock receiver exists.  The source-qualified Model-339 selection is L3; a fallback ratio is L2 data. |
| PIT-T3 IRQ/refresh output relation | Input L3; L4 speaker presentation | PIT output reaches the already-owned PIC/DMA digital endpoints.  Physical speaker presentation remains L4, not L2 or L0. |
| PIT-T5 | Input L3 | The copied plan is the existing oscillator/output qualification receiver. |

### RTC/CMOS MC146818A - 16 rows

| Rows | Final classification of the row's residual | Reason and owner |
| --- | --- | --- |
| RTC-R1, RTC-R3, RTC-F2, RTC-F5, RTC-T1 | L3 only | Direct RTC/AT state and storage/topology facts. |
| RTC-R2, RTC-F1, RTC-F3, RTC-F7, RTC-T5 | Input L3; selected-value split | `core_machine_rtc_timing_plan` and the RTC clock-domain declaration are real copied inputs.  A direct selected-board value is L3; a ratio estimate remains L2 data. |
| RTC-R4, RTC-F6, RTC-T4 | L4 physical exclusion | Bus/oscillator/reset pulse limits remain source-backed physical timing. |
| RTC-F4, RTC-T2 | Input L3 for IRQ; L1 for CPU delivery | RTC-to-PIC is a direct digital input.  The later CPU observation point has causal order but no elapsed term. |
| RTC-T3 | L1 persistence recovery | The selected RTC/CMOS state is owned, but host persistence/recovery has no timing receiver or proportional model. |

### KBC 8042/NMI - 16 rows

| Rows | Final classification of the row's residual | Reason and owner |
| --- | --- | --- |
| KBC-R1--R2, KBC-R4, KBC-F2, KBC-F6, KBC-T1--T2 | L3 only | Direct UPI/AT host, buffer, keyboard-boundary, ownership and topology facts. |
| KBC-R3, KBC-F5 | L4 pulse duration; L1 delivery order | Command/reset/A20 effects are Manual L3.  The 6-us waveform remains L4; downstream reset/A20 order is L1. |
| KBC-F1 | Input L3 plus L1 firmware schedule | Construction supplies typematic/response/serial values to one KBC owner.  The unmodelled UPI firmware instruction schedule has only logical order. |
| KBC-F3 | Input L3 for delivered serial events; L1 for unselected keyboard command coverage | Keyboard byte delivery has an existing endpoint and profile timing fields.  No complete selected keyboard firmware/device model is present. |
| KBC-F4 | Input L3 | KBC OBF publishes directly through the PIC IRQ endpoint; no separate elapsed queue is invented. |
| KBC-F7 | L4 physical exclusion | UPI oscillator, bus and voltage tables remain source-backed physical timing. |
| KBC-T3--T4 | L1 downstream/firmware policy | Board bindings and separation are L3; CPU/memory phase and POST policy have causal order only. |
| KBC-T5 | Input L3 plus L1 remainder | Existing construction fields are L3 receivers; absent UPI-clock/firmware scheduling stays L1. |

### Intel 8272A FDC - 15 rows

| Rows | Final classification of the row's residual | Reason and owner |
| --- | --- | --- |
| FDC-R1--R2, FDC-R5 selected-identity part, FDC-F2 digital relation, FDC-F4 parallel-seek state, FDC-T1--T2 | L3 only | Direct 8272A/AT command, phase, wiring/category and selected-identity facts. |
| FDC-R3 media grammar, FDC-F1 mechanics, FDC-F4 mechanics, FDC-F5 image/flux grammar, FDC-T3--T5 | L1 | The existing Core owner has bounded causal/media state, but no numerical model or admitted timing/media input supplies the missing physical/firmware semantics. |
| FDC-R4 execution duration, FDC-F3 deadline conversion | Manual L3 fact; L2 conversion | Intel's SRT/HUT/HLT units and 4/8-MHz values remain Manual L3.  Only `CORE_MACHINE_FDC_SEEK_TRACK_TICKS`, the existing unsourced conversion into Core ticks, is L2; no FDC construction-time timing receiver exists. |
| FDC-F2 DMA/IRQ route | Input L3 | FDC uses the existing DMA-request binding and PIC endpoint; ISA service duration remains L1. |
| FDC-F3 AC/electrical tables | L4 physical exclusion | The 8272A AC tables remain source-backed physical timing. |
| FDC-R5 later-device extensions | L0 | No selected 8272A Core logical rule exists for a later-device extension. |

### VADP / IBM EGA - 15 rows

| Rows | Final classification of the row's residual | Reason and owner |
| --- | --- | --- |
| VADP-R1--R3, VADP-R5, VADP-F1 base maps, VADP-F3 source relations, VADP-T1, VADP-T5 ownership path | L3 only or Input L3 | IBM register/state facts are Manual L3; display topology, port decode and timing declaration enter the sole VADP through the copied display/clock plans. |
| VADP-R4 odd/even/shift and unsupported-map interpretation, VADP-F1 extended remap, VADP-F2 chained/character generation, VADP-F4 ISA arbitration | L1 | The manual gives bounded causal/memory relations, but no selected board conversion/consumer model supplies a numerical implementation. |
| VADP-R5 external status wiring, VADP-F5 light-pen signal consumption, VADP-T2 switch/monitor selection | L1 | State/port relationship is known, but no selected external producer/receiver path is currently configured. |
| VADP-F3 conversion value | Input L3; selected-value split | VADP has a real clock/timing declaration receiver.  A source/model value is L3; an unqualified default is L2 data. |
| VADP-F5 monitor/composite facts, VADP-T3 monitor electrical model | L4 physical exclusion | These are source-backed physical display/monitor facts, outside Core's copied-snapshot boundary. |
| VADP-F5 host presentation | L1 | The copied snapshot has a defined product handoff, but no selected host cadence model is part of VADP. |
| VADP-T4 | Input L3 for VADP clock; L1 for ISA wait/grant | One VADP timing owner receives a copied declaration; no board CPU-arbitration term exists. |

### VADP / IBM CGA - 17 rows

| Rows | Final classification of the row's residual | Reason and owner |
| --- | --- | --- |
| CGA-R1--R5, CGA-F1--F3, CGA-F5--F7, CGA-T5 | L3 only | Direct IBM/Motorola digital facts and the one VADP-to-copied-snapshot owner path. |
| CGA-F4 | L1 | IBM defines the special-mode boundary and ordering, but not the complete generation tuple. |
| CGA-T1--T2 | Input L3; selected-value split | The copied profile-to-VADP timing declaration is an existing L3 receiver.  Model-339's qualified 86Box cadence is Other L3; unsourced default/ISA arithmetic remains L2 data. |
| CGA-T3 | L1 | The light-pen register/latch order is Manual L3, but no selected external pen producer, cancellation or consumer is present. |
| CGA-T4 | L4 physical presentation; L1 host cadence | Named digital interface facts remain Manual L3; waveform/phosphor/composite are L4, while host cadence has only the copied-snapshot handoff order. |

### ATA/HDC - 15 rows

| Rows | Final classification of the row's residual | Reason and owner |
| --- | --- | --- |
| ATA-R1--R2, ATA-F3 logical reset, ATA-F4 logical PIO, ATA-T5 | L3 only | Direct ATA-3 grammar, reset/PIO order and ATA-versus-MFM boundary. |
| ATA-R3, ATA-F2, ATA-T1 | Input L3 for configured ports/IRQ/media route; L1 for absent cycle/DMA duration | HDC topology is copied from composition to its sole owner.  The task file does not have an ATA service-time or DMA-arbitration input. |
| ATA-R4 | Input L3 for selected protocol/capability; L1 for unsupported device-command semantics | Existing HDC configuration selects protocol/capability, but source does not select an arbitrary device command set. |
| ATA-R5 | L1 | PIO shape is Manual L3; exact IDENTIFY image/device geometry has no complete selected input model. |
| ATA-F1, ATA-F5 | L4 physical exclusion | Cable/controller electrical tables remain source-backed physical timing. |
| ATA-T2--T3 | Input L3 | Existing HDC/media topology is a one-way selected device/media input to the sole HDC/media owners. |
| ATA-T4 | L1 | One HDC/media/PIC scheduler path is established, but no source-qualified ATA service-time receiver or proportional estimate exists. |

## Reconciliation And Required Follow-up

This removes the old ambiguous use of `L2 receiver`:

1. Existing copied inputs are now **Input L3** even when their current
   selection is a fallback estimate.
2. Only the four factual internal estimates remain L2: fallback DMA/PIT/RTC/
   VADP values where selected, and FDC's fixed seek-tick constant.
3. Every residual that has only causal order is L1; every source-backed
   waveform or physical fact is L4; only a missing logical relation is L0.

The audit found one unrelated code-quality defect while inspecting the RTC
construction path: `src/vm/composition/session/session.c` had a duplicated
`rtc_clock` conditional. T469 S1 removes the inner duplicate and adds default
PC/AT and Model-339 provenance regressions; this classification evidence is
otherwise unchanged.

No controller feature or timing algorithm is claimed closed by this
reclassification.  The source/manual facts, named external-model facts, sole
ownership paths and source/value provenance remain independently required for
future implementation work.
