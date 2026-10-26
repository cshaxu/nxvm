# M5 T580 S59 — PC110 ATA two-device construction route

## Boundary

The existing X86-NXVM product configuration already owns two fixed-disk
inputs (`fixed_disk0` and `fixed_disk1`), and the existing Board HDC already
owns one ATA task-file path with master and slave media IDs. Before this
receiver, Machine rejected the second input and only allocated, bound, reset
and inserted slot zero; MyPC110 also left the board's slave ID invalid.

The receiver makes that declaration truthful. Construction now states the
physical fixed-disk slot count. The four existing PC plans expose one slot
where their HDC topology has one physical unit; MyPC110 exposes two and binds
the pre-existing secondary registry ID. Machine performs the existing
allocation, registry binding, reset and startup insertion once per declared
slot. Fixed geometry remains master-only: no CF geometry is guessed or imposed
on the slave.

The X86-NXVM ATA model already carries a Drive/Head-selected unit through its
single task-file and media callbacks. Four master-only guards still rejected
that ATA slave at command, CHS/LBA address resolution and IDENTIFY. They now
allow ATA's existing unit selection; Xebec and the separate Compaq/IBM paths
retain their existing rules. No second ATA controller, public API, queue,
storage backend, PCIC mapping or guest special case is introduced.

## Direct proof

The existing Board HDC smoke fixture binds distinct master and slave media to
the real registry/HDC route. It proves slave selection through Drive/Head for
CHS read, LBA read and IDENTIFY, while preserving the prior master cases.
Machine construction proves both slots allocate/bind and rejects a second
configured disk when the selected board declares only one physical slot.
MyPC110's construction test proves its production plan declares two slots.

Both host widths pass the changed-unit set:

- `x86-nxvm.machine/construction`;
- `x86-nxvm.machine/preparation`;
- `unit.x86-nxvm-board-hdc-smoke`;
- `mypc110.unit/machine-construction`;
- `x86-nxvm.manifest` and `x86-nxvm.test-manifest`.

The unchanged external TFT firmware passes bounded integrations after the
route change: FDC reset (`50.33 s` x64, `57.00 s` x86) and boot handoff
(`270.45 s` x64, `314.03 s` x86). These are non-regression checkpoints only.
They do not demonstrate that the BIOS has consumed a supplied SunDisk/CF,
that PCIC owns storage, or that a selected guest has booted.

## Retained limits

Authentic SunDisk/CF assets remain BYOB external inputs. BIOS storage setup,
normal Setup persistence, PC Card storage, selected guest boot, panel
qualification, desktop/RDP qualification and PC110 artifact publication stay
outside S59.
