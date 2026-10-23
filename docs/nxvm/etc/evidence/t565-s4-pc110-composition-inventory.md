# M5 T565 S4 — PC110 composition inventory

## Method and source grade

This inventory compares the current NXVM construction graph with the pinned
Open-Source-PC110 checkout `02ccf2e8e19b298a2cf04c5d430278eb087cc368`.
That checkout is research evidence only.  Its `Discovery/Chipset/readme.md`
section 13j, `Discovery/Bowman/readme.md`, `Discovery/Pluto/readme.md` and
`Discovery/65535/readme.md` distinguish observed port/configuration facts from
reconstructed device behavior.  No code or firmware is imported.

## Construction and controller disposition

| Candidate | Current owner | PC110 disposition | Evidence / reason |
| --- | --- | --- | --- |
| `x86_nxvm_machine_construction_begin/finish` | X86 NXVM Machine | Reuse | It only publishes one App-prepared construction and has no AT identity. |
| `x86_core_machine_firmware_provider` | X86 Core Machine | Reuse | It accepts immutable App-supplied ROM mappings synchronously; it does not impose an AT slot layout. |
| `x86_nxvm_machine_pc_at_construction_create` | X86 NXVM Machine | Exclude | Its fixed PC/AT firmware roles and board preparation are not a PC110 contract. |
| `x86-nxvm-board-base` full plan | X86 NXVM Board | Do not reuse as a PC110 shortcut | It carries selected AT/XT topology defaults: controller wiring, display, media and reset assumptions need separate PC110 proof. |
| PIC, PIT, DMA, RTC and 8042 chip components | X86 NXVM chips | Candidate, one endpoint at a time | PC110 research establishes integrated compatible cores and standard port families, but not the complete board timing/IRQ/DMA/KBC wiring. |
| `x86-core/chips/vl82c420` | X86 Core | Reuse through one PC110 board binding | S3 owns its protocol state.  The App, not Core, owns physical port decoding and any qualified reset image. |
| F65535 display, DOS/V font window, FDC/IDE, PCMCIA, keyboard/power and audio | Later PC110 receivers | Not admitted here | Each has a distinct ASIC/protocol or panel/default contract.  Attaching an AT equivalent would fabricate behavior. |

## Confirmed configuration endpoint map

The next board binding may attach exactly these configuration windows without
inventing a controller default:

| Window | Ports / behavior | Owner | Qualification boundary |
| --- | --- | --- | --- |
| SCAMP | `74h` index, `76h` data, 7-bit index | PC110 board binding → VL82C420 | The live dump and BIOS helper both mask the index with `7Fh`. |
| Block2 | `24h` index, `25h` data, 8-bit index, four-read gate | PC110 board binding → VL82C420 | The exact enable reads and relock mechanism remain in S3; later board work must prove their guest-address route before installing it. |
| EC/ED | `ECh` index, `EDh` data | PC110 board binding → VL82C420 | It is the separate shadow/cache/ROM-decode bank; gate/defaults are not silently inferred. |
| RTC | `70h`/`71h` | Later RTC binding | Existing RTC chip is reusable, but its IRQ/reset/timing attachment needs a PC110-specific route proof. |

The source confirms that SCAMP and Block2 are distinct windows with selected
overlapping state, not interchangeable byte arrays.  S3 deliberately keeps
opaque storage and does not claim the unresolved cross-window transformations.
That is sufficient for a port adapter test, not for DRAM, shadow or suspend
behavior.

## App boundary and next implementation

The sole legal composition remains:

```text
app-mypc110 machine plan
  -> PC110 board / firmware binding
  -> x86-nxvm neutral construction publication
  -> x86-core executor and immutable firmware mapping
```

S5 may create the first `app-mypc110` source/test root only for a typed
firmware binding and a direct configuration-route adapter.  It must not create
an executable, select an INI model, enter a guest run loop, substitute AT
preparation, or attach an unqualified endpoint.  A direct unit test must prove
that exactly one selected port owner delegates to the S3 chip; it cannot claim
POST, reset defaults or a bootable machine.

## Retained blockers

- The TFT asset pairing is prepared externally, but its build embedding waits
  for the App-local firmware binding and later real-firmware qualification.
- Original DSTN firmware remains unavailable.
- VL82C420 power-on values, ROM shadow effects and cross-window register
  transforms remain unqualified.
- Bowman/Pluto routing, F65535/DOS-V display, font ROM use, Super-I/O/storage,
  keyboard/power, PCMCIA and audio retain their dedicated later receivers.

No production source changes in S4 are justified by these facts.  The inventory
is the bounded evidence handoff for the first PC110 App-local route receiver.
