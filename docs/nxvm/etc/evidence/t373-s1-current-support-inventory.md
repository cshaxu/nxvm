# T373 S1: Current Product-Support Inventory

## Decision

The tracked product has two concrete PC/AT descriptors, not three completed
baseline machines: `default-pc-at` and `ibm-5170-model-339`. The former is a
generic 80386/EGA/ATA-capable product configuration; the latter is the selected
80286/512-KB/CGA/no-HDC Model-339 configuration. CPU selection supports 8086,
80186, 80286, and 80386 contracts, but it does not select a 5150/XT or DeskPro
board. No DeskPro Model 40 or PC/XT 5160-268 descriptor, ROM binding, or
device topology exists yet.

Consequently, source presence or an individual smoke is not a claim of device
completeness or L3 timing. The inventory freezes the repository-side support
surface for later source qualification; it does not decide the final public
surface or machine membership.

## Production-Surface Matrix

| Surface | Current production owner and evidence | Current classification | Baseline/closure receiver |
| --- | --- | --- | --- |
| CPU profiles and transaction foundation | `core/machine/{cpu,transaction,machine}`; VM profile contract accepts 8086/80186/80286/80386. T368 closes accepted 80286 successful retirement. | Present, but CPU profile alone is not a physical machine. | DeskPro Model-40 CPU candidate and 8088 CPU-profile candidate own their distinct architectural/bus work; Model-339 phase timing consumes no reopened semantics. |
| Memory/ROM/reset, PIC, PIT, DMA, RTC/CMOS, NMI and port ownership | `core/machine/{memory,rom_mapping,machine,pic,pit,dma,rtc,transaction,timeline}` plus PC/AT descriptor routes. Existing smokes and T366--T371 prove selected logical ownership/order. | Present logical foundation; selected-device completeness and board timing are not proven. | Capability ledger classifies each selected instance; 5170 functional closure then 5170 phase-timing closure. |
| Keyboard and 8042 input | `core/machine/kbc`, default-profile keyboard mapper and BIOS keyboard service; descriptor maps IRQ1 and IRQ12. | Bounded AT keyboard path present; KBC/AUX path is product-visible in the generic descriptor. | Ledger decides Model-339 membership; 5170 functional closure owns selected AT keyboard/8042, while AUX/mouse remains current-product closure unless a later baseline selects it. |
| AUX mouse | `kbc`, default-profile mouse mapper, platform request bridge, `vm-kbc-aux-guest-smoke` and DOS mouse-driver smoke. | Bounded product capability; not selected Model-339 evidence. | Current-product device-capability closure must L3-close it or remove it; advanced AUX remains TODO. |
| FDC and removable storage | `core/machine/fdc`, VM FDD provider, FDC/IRQ6/DMA2 route, FDC BIOS services and FDC/DOS smokes. | Bounded boot/controller path present; completeness and duration remain unproven. | Capability ledger selects the exact 5170 field-upgrade requirement; 5170 functional then timing closure own it. |
| ATA/HDC and HDD | `core/machine/hdc`, VM HDD provider, generic descriptor HDC ports/IRQ14 and ATA/INT13/DOS smokes. | Bounded ATA PIO/HDD product capability; explicitly absent from Model 339. | Current-product device-capability closure; never a Model-339 or IBM MFM/ST-506 substitute. |
| CGA/VADP | `core/machine/vadp`, default-profile CGA firmware and CGA text/graphics/640 tests; Model-339 descriptor selects CGA VRAM and suppresses EGA leaves. | Selected Model-339 digital capability is present but not complete/timed. | Ledger freezes exposed CGA surface; 5170 functional closure owns selected behavior, 5170 timing closure owns cadence/contention, and current-product closure owns any exposed non-5170 breadth. |
| EGA/VADP | `vadp`, generic descriptor EGA aperture/register leaves and EGA BIOS/system/DOS smokes. | Bounded generic product capability; Model-339 descriptor suppresses it. | Current-product device-capability closure must L3-close retained EGA paths or remove them; broader EGA/VGA stays TODO. |
| Firmware services | Default-profile generated BIOS, FDC/HDC/RTC/CGA/keyboard firmware services; Model-339 descriptor removes HDC service and declares only abstract IBM Rev.3 slot. | Product-generated firmware behavior is present; no external vendor ROM is a product dependency. | Ledger records abstract slot requirements only; later machine profile work owns external-ROM admission, never this S1 inventory. |
| Unimplemented peripheral families | No production source hit for serial, parallel, game port, speaker/PPI or joystick controller terms; TODO retains their bounded admissions. | Not currently supported. | Retain TODO; do not add to any baseline merely because historical hardware might have carried it. |
| Future machine profiles | No production DeskPro/Compaq/5160/5150/XT descriptor or composition hit. | Absent, not test-only support. | DeskPro Model-40 and PC/XT 5160-268 profile/capability audits must select and justify them before implementation. |

## Evidence And Sweep Method

The inventory reviewed descriptor roles, route and firmware-service tables in
`src/vm/profile/default_profile/pc_at_profile.{c,h}`; session controller/media
composition in `src/vm/composition/session/{machine_devices,media}.c`; all core
machine owner files; CMake target registration; relevant focused tests; the
current capability baseline; and TODO. The principal static queries were:

```text
rg -n -i "\\b(cga|ega|vga|vadp|fdc|floppy|ata|hdc|ide|dma|pic|pit|rtc|cmos|kbc|keyboard|aux|mouse|nmi|parity|serial|parallel|gameport|ppi|speaker|rom|bios|profile|pc_at|ibm_at|xt|5150|5160|5170|deskpro)\\b" src tests CMakeLists.txt CMakePresets.json docs/etc/evidence/current-capability-baseline.md docs/states/TODO.md
rg -n "VM_PROFILE_DEFAULT_PC_AT_(DEVICE|ROUTE|FIRMWARE)_" src/vm/profile/default_profile/pc_at_profile.{c,h}
rg -n -i "\\b(serial|parallel|gameport|game-port|lpt|com[0-9]|speaker|ppi|8255|joystick)\\b" src CMakeLists.txt tests
```

The last query finds only build-parallel wording, not a production peripheral
controller. All source hits above have a disposition in the matrix; no test
fixture is treated as a public product device merely because it names a CPU or
controller type.

## Exact Transfers

S2 must use primary machine documentation to turn the locked names into exact
DeskPro Model-40 and PC/XT 5160-268 bills of materials, and reconcile the
existing Model-339 descriptor with the complete selected device set. S3 then
classifies each matrix row as complete, partial, empty, test-only, or removed,
names the earliest functional owner, and separates it from the later timing
owner. No code, asset, timing scalar, or L3 conclusion is created by S1.
