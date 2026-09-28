# T539 S1: Independent Chip Design Review

Research against 996a19a17, 2026-09-27. This is a proposed design for owner
review, not an implemented ABI or a hardware-completeness certification.
The [proposal](../../proposals/m5-shared-chip-extraction.md) owns task scope;
the [finite inventory](../evidence/t539-chip-migration-ledger.md) covers all
81 tracked files. Review covered declarations, dependency edges, execution and
signal boundaries, construction, scheduling, callers, CMake and test ownership;
it was not a new line-by-line audit of every instruction's semantics.

## Proposed Layout And Dependencies

```text
src/x86/
  devices/
    cpu/          cpu_interface.h; existing execution/decoder/timing internals
    fpu/          fpu_interface.h; implemented extension behavior
    pic8259/      pic8259_interface.h; one controller instance
    pit825x/      pit825x_interface.h; 8253/8254 selected behavior
    dma8237/      dma8237_interface.h; one four-channel controller
    rtc146818/    rtc146818_interface.h; RTC/CMOS mechanism
    kbc8042/      kbc8042_interface.h; qualified AT-controller behavior
    keyboard/     keyboard_interface.h; existing attached-device mechanisms
    ppi8255/      ppi8255_interface.h; implemented Mode-0 subset
    fdc8272/      fdc8272_interface.h; command/execution/result mechanism
    hdc/          hdc_interface.h; explicit existing controller personalities
    video/        video_interface.h; one video state/VRAM owner
  xasm32/         unchanged
  debug/          unchanged
src/app-nxvm/
  devices/        temporarily retains board/machine owners during T539
  profiles/       fixed-machine construction and wiring
  machine/        Common-machine adaptation
test/x86/devices/ mirrors extracted components
test/app-nxvm/    retains board, profile, product and external integration tests
```

Names above are proposed directories, not promises that all modeled devices
are Intel chips or that all variants are fully implemented. HDC/video are
adapter families, not individual silicon. The existing non-Intel devices are
required by the four receivers and must not disappear during extraction.
The later board task, not T539, owns `x86/ibmpc`; do not create a second machine
scheduler there while the old one still owns execution.

- A component uses Lib Types and its own headers. Public entry points end in
  `_interface.h`; other files are private. No private peer headers, App headers,
  Common executor, host clock/thread, KVM or file handle enters a chip.
- Board composition binds signals and memory/I/O operations. Devices do not
  locate each other, own a service registry, or receive a whole-machine pointer.
  Small synchronous signal callbacks are appropriate; no queue/thread per chip.
- CPU owns segmentation, paging, instruction retirement and exceptions. Board
  owns physical address decode, RAM/ROM maps, A20 wiring, DMA arbitration,
  interrupt connections and the sole guest scheduling axis. CPU state is not
  duplicated for diagnostics or timing.
- Each instance owns its state. Use opaque handles created/destroyed during
  machine construction; callbacks borrow contexts with documented lifetime.
  One execution owner calls mutating APIs. Stop it before unbinding/destruction.
  No locks or reference-count framework added to make unsupported concurrency
  appear safe. Creation failure unwinds only resources actually created.
- Register operations use chip-local selectors, not hard-coded PC port maps.
  Bus results preserve failed-access and effect-order semantics. Reentrancy is
  limited to documented signal propagation, never arbitrary recursive execution.
- Do not mandate identical clock units for unlike current models. Each timing
  contract names its input unit and deadline meaning. Board conversion retains
  rational remainders; native PIT cycles and source-axis service intervals must
  not be silently equated. Expose ready-now / next deadline / no scheduled work /
  unavailable timing distinctly where currently meaningful. Preserve existing
  ordering, grade and approximation; extraction is not an L3 upgrade.
- Do not introduce a universal device base/vtable. Start with owner-local bus,
  signal and media contracts. Share a value contract only when two real users
  have the same semantics; no duplicate RAM, media cache or second state owner.

## Per-Component Findings And Required Changes

### CPU and FPU — high boundary risk

`cpu_instructions.h` stores concrete RAM, ports, transaction, paired PIC and FPU
pointers. `cpu_instructions.c` performs PIC selection/acknowledgement directly;
`cpu_timing.c` and `cpu_timing_model.c` depend on `machine.h`. These are real
dependencies, not just old names. The execution context also has a live firmware
interrupt callback, bound in `machine.c`, with IVT/provider logic in
`machine_firmware.c`.

Required diff: preserve instruction tables and handler style; replace physical
bus/INTA/extension connections with bounded contracts; keep CPU translation and
fault rules inside CPU; make timing consume CPU-owned state plus copied bus
observations rather than machine private fields. Keep original success, fault,
partial-effect, REP, HLT and retirement distinctions. Debug captures copied
register/segment/control state through the CPU boundary. No raw CPU pointer in
diagnostic callbacks. FPU owns its registers/BUSY/ERROR/completion; CPU owns its
instruction-side extension rules. Existing limited FPU support is not a claim
of complete 8087/287/387 emulation.

The firmware interception hook needs a separate live-consumer disposition before
CPU extraction: retain required behavior in board/provider integration through a
reviewed bounded execution boundary, or prove obsolete consumers removable.
Do not carry BIOS-specific frame types into the shared CPU or silently delete
the hook because normal ROM boot currently works. This is an implementation
decision gate, not a resolved dead-code finding.

CPU timing generates two catalog includes through NXVM CMake. Extraction must
give the shared CPU target self-contained generated inputs and retain ledger
provenance, without a hidden dependency on an App build directory. Current CPU
families remain intact; this work adds neither V30 nor 486 support.

### PIC 8259 — paired chip and board wiring currently mixed

`pic.h` embeds cascade peer pointers. Initialization in `pic.c` accepts a master
and slave and registers 20h/21h/A0h/A1h. Interrupt acknowledgement operates on
the pair. Required diff: one controller state per instance; local command/data
access, IRQ inputs, INT output and explicit cascade/acknowledgement interaction.
Board binds the XT single chip or AT pair and maps ports. Preserve ICW/OCW,
poll, priority, SFNM, spurious-interrupt and acknowledge-phase behavior. Do not
replace the pair with a simplistic precomputed vector callback that drops phases.

### PIT 8253/8254 — relatively close to independent

Existing output callbacks, gate inputs and next-output query are useful. The
remaining coupling is port registration and shared machine/controller type
definitions. Move register/state/waveform mechanisms as one component with its
existing variant selector; expose four local register selectors. Keep PIC IRQ0,
DMA refresh, speaker routing and input-clock conversion in board composition.
Do not create two nearly identical timer implementations.

### DMA 8237 — substantial board extraction

`dma.h/c` mix one-chip channels/modes with PC page/spare registers, primary and
secondary controllers, word addressing, direct physical memory operations and
shared transaction latches. Initialization binds the PC/AT port map and the
advance path inspects peer state. A global atomic binding-token allocator also
exists; its stale-binding protection must survive relocation, not be discarded.

Required diff: isolate one four-channel controller's request, priority, modes,
address/count, compressed timing and terminal-count phases. Board owns page
registers, address expansion, byte/word lane mapping and cascade connections.
Provide explicit bus request/grant and transfer-result boundaries; preserve
memory validation before irreversible device effects and all bus holds/EOP/DACK
ordering. Do not collapse a transfer into an unconditional memcpy callback.

### RTC/CMOS — remove controller and machine-plan dependencies

`rtc.h` includes the machine contract and PIC type; `rtc.c` drives a PIC source.
Move the 64-register RTC/calendar/interrupt mechanism and timing input to its
own boundary with IRQ output. Board owns 70h/71h decode, NMI gating and seed
application policy. Retain the existing CMOS/RTC ownership distinction; no
invented 128-byte extension, host wall-clock progression or copied PIC state.

### AT KBC, attached keyboard/AUX and XT PPI/keyboard — high regression risk

`kbc.c:core_machine_kbc_apply_output_port` writes RAM A20 directly and requests
CPU reset; construction binds PIC and fixed 60h/64h ports. One structure mixes
controller buffers, keyboard commands/BAT/typematic and auxiliary-device state.
Expose controller output lines and IRQs; board implements reset/A20 effects.
Separate controller transport from attached-device command state along the
existing single reply path. Preserve serial byte admission, ACK/BAT deadlines,
inhibit/release, reset and IRQ ordering. No BIOS-specific synthesized replies.

This is an AT-controller behavior model, not an executable generic 8042 MCU.
The XT PPI header explicitly limits itself to the XT Mode-0 attachment, not
8255 Modes 1/2. Factor the implemented register/direction mechanism from XT DIP,
speaker, NMI and keyboard wiring, naming the supported subset honestly. XT
keyboard currently holds a concrete PPI pointer; use line/byte handshake
contracts instead. AT and XT keyboard protocols must not be forcibly merged
merely because both deliver keys. AUX state is a device responsibility, not an
extra board-owned mirror of the controller's output queue.

### FDC 8272A and drives — separate controller from PC adapter

`fdc.h/c` combines command phases with DOR/DIR/CCR, fixed ports, PIC/DMA
connections, mechanical drive inputs and a diagnostic register. An explicit
`DESKPRO_REFERENCE` unready-read branch changes result status in
`core_machine_fdc_complete_unready_read`.

Required diff: preserve command/execution/result, IRQ/DRQ/TC and byte deadlines;
move PC adapter decode, motor/selection wiring and diagnostic-port policy to
board integration. Media stays an opaque query/read/write service, not Lib
storage inside the chip. Distinguish controller present-cylinder bookkeeping
from real drive position only where the protocol requires both; do not mirror
them everywhere. Before removing the DeskPro branch, determine whether its
observed response follows controller variant, physical drive inputs or a
documented board approximation. Renaming it to a neutral flag is not decoupling.
Preserve receiver behavior and evidence until that decision is reviewed.

### HDC personalities — not one universal ATA chip

Current family implements ATA PIO, Compaq/WD 40MB, WD1003/ST-506 and Xebec XT
paths, with media IDs, PIC and DMA connections. Keep explicit personalities and
their actual protocol/state boundaries. Board owns ports, IRQ14/IRQ5, DMA and
shared 3F7 routing; controller owns command phases, DRQ, IRQ and service timing.
Do not infer that all personalities share ATA semantics. Internal task-file vs
Xebec code may be split along actual protocol responsibility without a plugin
framework or duplicate CHS/media owner. Preserve per-sector DRQ waits and
failure behavior. No new ESDI implementation is implied by moving the family.

### Video adapters — one state owner, not a generic Intel chip

`vadp.c/h` owns CGA/EGA/VGA-related and Compaq extension state, but directly binds
ports, physical memory windows and dirty observers, and reads text memory
through machine RAM. Keep one register/latch/VRAM/cursor/frame-generation owner.
Expose register/MMIO and copied display observations; board installs address
decode and routes backing-memory access without duplicating VRAM. Preserve
planar mapping vs display geometry vs disabled/text fallback distinctions.
Common/Lib presentation, fonts and host surfaces do not enter this component.
Split internal files only where mechanisms warrant it; do not create separate
mode/frame owners for CGA/EGA/VGA, nor claim a complete standalone 6845 model.

## Non-Chips, Build And Tests

Machine construction/plan, scheduler/timeline, physical memory/port routing,
transaction arbitration, ROM/firmware services, media registry and Model-40 D4
mapping remain board/adapter-owned. T539 must reconnect these consumers through
public chip APIs; moving them unchanged into Shared would hide the coupling.

`src/x86/verify_corpus.cmake` currently recognizes only debug/xasm32 and a
three-level include grammar. It will reject the proposed nested devices tree.
Extend the explicit component map and negative tests, not a blanket devices
exemption. Current standalone x86 CMake also unconditionally assembles Common;
chip-only builds need a real Types-only dependency closure. NXVM's executor and
observable targets must link shared chip targets rather than compile a second
production copy; observable instrumentation is not itself duplicate source.

Pure chip cases move to mirrored `test/x86/devices`; existing tests that construct
whole machines, inspect board ports, test refresh/cascade/ROM or use profile
configuration stay NXVM tests. Refactor fixtures at their real ownership boundary,
preserving all scenarios. Examples: PIT waveforms/readback are chip cases;
PIT IRQ0 and machine divider are board cases; DMA channel semantics vs Model-40
DMA mapping are distinct. Mixed tests split assertions rather than blindly move.
Each chip gets independent build/reset/register/signal/deadline/error-path proof.
All affected unit suites run for each implementation S, then required external
integration and eight receiving NXVM artifacts at T closure. Preserve existing
MyNES behavior and follow the rules for any genuinely affected Shared receivers.

## Recommended Decision Gates And Sequence

Owner review is needed before production on: (1) ecosystem scope including the
required non-Intel adapters; (2) one-instance PIC/DMA contracts rather than paired
PC controllers; (3) qualified KBC/PPI subsets, not newly promised full chips;
(4) CPU firmware-hook disposition and FDC unready-response ownership; (5) explicit
clock units and effect-order contracts without rewriting the timing model.

After review, admit coherent batches rather than preassign speculative S numbers:
PIT/RTC first to prove a real standalone boundary and build/test path; PIC and
DMA with their board connections; KBC/keyboard/PPI; FDC/HDC; video; CPU/FPU with
timing/debug/firmware boundaries; final cross-family and all-file closure.
CPU contract design must precede these batches where shared bus/INTA decisions
affect them; putting its large source migration later does not defer that design.
Every batch reconnects production and removes its old copy before closing.

Diff is more than relocation: public state opacity, peer connections, timing
inputs, board splits, tests and build guards change. PIT/RTC are smaller boundary
changes; PIC/video medium-to-large; CPU/DMA/KBC/FDC are high-risk semantic-boundary
changes. Exact changed-line totals cannot honestly be estimated before these
decisions. Review mechanical move/name diffs separately from executable changes;
do not reformat the instruction handler corpus or rewrite working algorithms.
