# T508 Corrective S5: L2 Model Reconciliation

## Decision

The rejected boundaries can be improved without changing the owner graph.
`core_machine_hdc_config.service_ticks` remains the one frozen HDC input and
the Core scheduler remains its only deadline consumer. `core_machine_time_axis`
remains Core-owned observation data; VM can only wait against completed Core
progress in Standard mode.

| Input | Selected value | Evidence and level | Rejected interpretation |
| --- | --- | --- | --- |
| default-PC/AT ATA PIO service | `200` Core elapsed ticks | **Other-L2.** Local 86Box `src/disk/hdc_ide.c` defines `IDE_TIME` at line 111 and schedules its generic read/command fallback with `200 * IDE_TIME` at lines 1844--1886 and 1914--1932: command completion is controller-timer work, not immediate issue. NXVM has no drive-speed/seek model or physical generic-PC axis, so it retains the corroborated 200-step controller quantum in its existing elapsed axis. | It is not a universal ATA 2-ms mechanical assertion and not Manual-L3. ATA-3 gives interface ordering, not one drive-service duration. Bochs separately models a 100000-us seek timer in `iodev/harddrv.cc` lines 559--642; PCjs's ad-hoc delay is not used. |
| DeskPro 386 Model 40 pacing axis | `MACRO_PROPORTIONAL`, `16000000` Core ticks/s | **Other-L2 macro pacing.** The retained Compaq D3PE transcription records "16mHz 80386 processor clock" at lines 8727--8737 and its processor/memory 16-MHz operation at lines 11159--11244. Local 86Box `src/machine/machine_table.c` lines 7127--7142 selects the DeskPro 386 and `src/cpu/cpu_table.c` lines 1450--1470 selects the 16-MHz i386DX entry. | It is not `VERIFIED_PHYSICAL`: NXVM's retirement axis is not proven one CPU cycle per tick, and no source proves complete instruction, bus, and device physical equivalence. |

## Owner and Unit Boundary

The ATA value is deliberately stored in the profile's existing Core elapsed
unit. It produces an HDC-owned absolute deadline; it is not converted by VM
and does not require a second rate. The Model-40 macro rate makes completed
Core progress eligible for Standard pacing while keeping
`physical_time_available == false`. Turbo reaches exactly the same Core
deadlines and omits only host waiting.

This is the strongest common model available: external implementations agree
that IDE/ATA commands are asynchronous controller work, but not on one
portable mechanical latency. The selected scalar is therefore an explicit
Other-L2 macro, not an invented manual number.

## Firmware Consumer Repair

The first integration replay exposed a formerly hidden immediate-completion
assumption in the default INT 13h read/write service: immediately after
writing `20h` or `30h`, it sampled status once and required DRQ. That cannot
consume an asynchronous HDC model. The same firmware route now polls only the
real status port while BSY is set, then preserves its existing ERR/DRQ checks
and PIO transfer. Successful poll instructions publish time through Core's
existing retirement path; the HDC alone reaches its deadline and publishes
DRQ/IRQ. The ATA PIO DOS, HDD boot, and Windows INT 13 trace integration tests
cover that complete consumer path.
