# M5 T540 S32 Neutral Construction Boundary

## Actual source review

The one public `core_machine_create` and `core_machine_create_from_plan`
routes still enter `core_machine_create_internal`. That function now performs
the original pre-allocation neutral and board validity checks, invokes one
private `core_machine_neutral_create`, then performs the unchanged board
registration sequence. It does not publish `out_machine` until all board
construction succeeds. The neutral phase owns allocation, CPU/FPU, copied
retirement qualification, timeline/clocks, transaction, CPU bus, port, RAM,
A20 policy and reset-ROM fallback map. It has no PIC/PIT/DMA/KBC/VADP/XT
personality branch or product-name choice. Board callback binding and keyboard
timing inputs occur only after neutral construction succeeds, before the
original port checkpoint and device order.

The failure chain remains explicit: null output is set before validation;
invalid neutral/board input returns before allocation; allocation failure is
`NO_MEMORY`; invalid qualification, CR-MOV CPU choice and clock/timeline
values release the candidate; failed FPU/CPU creation releases their owned
objects; bus/RAM/A20/reset map failures use the sole Core destroy route. A
board failure still destroys the one constructed Core and never publishes a
result. `create_from_plan` retains success-only plan copying and S33 retains
topology application/rollback. No second public constructor, owner state,
guest clock, timing formula or configuration parser was added.

The code diff is one owner-local `machine.c` file: **91 lines added, 73
removed, net +18**. The increase defines the private neutral validation and
construction boundary, while moving the existing effects once rather than
duplicating them. S33 will separate board controller creation and plan apply.

## Verification

- Full repository-only units: x64 **469/469**, x86 **469/469**.
- Current specialized gate target: **102/102** build/gates passed.
- Existing allocation, invalid-config, plan, CPU/FPU, port, memory and
  rollback tests are covered by both complete unit runs.
- External floppy boot checkpoint: default, XT, 5170 and Model 40 passed once
  per width, **8/8**.
- Eight optimized 0540 products rebuilt. `objdump -f` verifies four
  `pei-x86-64` and four `pei-i386`; `objdump -h` finds no `.debug` sections.
  SHA-256 by profile, x64 then x86:
  - Model 40: `EF4C9A7C38B9FCB3188A2F9CB1BF66F3569E2E0CC78D48DA8EF0AE6076AB070F`, `258B4B80D01611CF8383F58BCF0344A8659222D5EBCA3018AE039F14BA18C63D`.
  - Default: `166FB1F34F3F8C0D331A18AFE400E60CD7A67633355198F512D71502E3D6C3E6`, `16675F324A3045C1250CFD8B77845AD75DF2EB8188FF3F85237192A22CF09CF0`.
  - XT: `0DE91321377F16E7DAF6340A8182AC7614945E582CD847971FD2AC83C34C0434`, `DAC32A974AAF3E37298C0FAC2A25C755A87C3C900CB3875721C2514BAE89709F`.
  - 5170: `1DB297E35F5D530DC4FA70B3703F3E6233260F73CDD35766B446EB6DEAC75D34`, `09560C17B3DC87DD8C96F2C6DB1ABF70E6AABD4728A116E83CAC77C1639676F2`.

No Shared/MyNES file, INI, firmware source or protected asset changes. The
four profile choices and timing grades remain unchanged. T540 is open.
