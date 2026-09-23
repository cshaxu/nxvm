# T481 S1 complete audit universe

`M5:T481:S1:UNIVERSE:OK`

This is an index of the frozen audit rows, not a replacement ledger. Each
referenced closure retains its primary-page provenance, row-level source
classification and detailed results. A later S must cite its family ledger
rather than create a parallel row set.

| Family | Sole audit ledger(s) | Current owner/path | Exclusion or successor |
| --- | --- | --- | --- |
| 8086/80186 execution, form gates and delivery | `t343-s1-four-profile-cross-closure.md`, consuming T338 S2--S5 and shared T337/T321/T326 | Core decoder, profile classifier and final delivery owner | x87 execution and device timing are independent rows. |
| 80286 execution, protected transitions and timing | T343, consuming T339 S2--S7 and `cpu-timing/t436-*` / T357 S6 | Core protected-state, timing classifier and retirement owner | Physical bus timing is not implied by Appendix-B instruction timing. |
| 80386DX execution, paging/VM86/system/debug and timing | T343, consuming T340/T341 and `cpu-timing/t437-*` / T357 S3/S7 | Core profile-specific state/retirement owners | 486+ and VME/PVI remain excluded. |
| PIC 8259A | `t456-s4-pic-retained-command-closure.md`, T456 S5 | Core PIC command/cascade/acknowledgement owner | Electrical topology/physical waveform is outside L3. |
| DMA 8237A | `t460-s5-dma-closure-audit.md`, T462 S4 | Core DMA service and copied-plan rule selection | Unselected board conversion remains L2. |
| PIT 8254 | `t461-s3-pit-integration-closure.md`, T462 S4 | Core PIT phase/OUT-to-IRQ route and copied-plan rule | Power-up/electrical behavior remains L2/excluded. |
| RTC, KBC and 8272A FDC | T450 S10/S12/S14 code-gap ledgers and their retained controller closures | Core RTC/KBC/FDC owners | Source-blocked phase/deadline rows remain explicit later receivers. |
| VADP CGA/EGA/VGA | T352 S1--S4, T480 S1--S5 | VADP sole port/memory/snapshot owner | VGA is a real Core capability but no current profile/card selects it. |
| Fixed disk | T479 S8 and S6 Xebec ledger | Core explicit ATA, Compaq WD and IBM WD1003 personalities | XT/Xebec, ESDI and physical service timing have named later receivers. |
| Core time/deadline and VM pacing | `t474-s1-core-time-convergence-ledger.md`, T474 S2--S5, T472 S1/S2 | Core sole time writer/deadline composer; VM is Standard/Turbo consumer only | No host-generated guest tick; only qualified PIT/RTC form deadlines. |
| Immutable profile inputs and Core-plan route | `t475-s1-profile-resolver-ledger.md`, T476--T478 closures | VM resolver/profile construction copies typed plan inputs to Core | Dynamic media, host resources, devices and Core state are excluded. |

## Reconciliation rules

1. T343 is the sole CPU form/state denominator; its upstream task records are
   evidence inputs, not duplicate T481 families.
2. The controller-specific closure is authoritative for each device. T462 and
   T474 only add board-rule/time composition context; they do not replace
   device-function rows.
3. A manual number or formula remains L3 where its family ledger implements it;
   L2 is reserved for labelled proportional/internal estimates, L1 for causal
   ordering and L0 for absent function/integration.
4. Profile/card selection is distinct from capability existence. Unselected
   VGA and transferred Xebec/ESDI remain rows with their named receiver, not
   unsupported aliases.

`M5:T481:S1:PROVENANCE:OK`

`M5:T481:S1:DEDUP:OK`
