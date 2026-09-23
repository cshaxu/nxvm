# M5 Complete Profiled Mapper 4

## Outcome

Generalize M4's one-size MMC3 path into the declared complete Mapper 004 profile: variable ROM/RAM capacity, PRG-RAM control, mirroring, register pairs, all PRG/CHR bank modes and qualified IRQ behavior. Super Mario Bros. 3 and TMNT III become bounded acceptance receivers.

## Acceptance

Fixtures cover every register pair, bank mode, capacity bound, RAM enable and write-protection transition, fixed/switchable mirroring, A12 debounce and IRQ counter transition. Each Mapper 004 submapper is directly supported with a named behavior or rejected during descriptor admission. Both supplied extended Mapper-4 inputs execute through Window/Console, audio/pacing and lifecycle scenarios on x64/x86 without a Core trap.
