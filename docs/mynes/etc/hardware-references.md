# Hardware Reference Register

Research checked on 2026-09-20 for the App/Core design. These are reference
documents, not imported code, tests or ROMs. Source use follows
[Source Policy](operations/policy/source-policy.md). No manual PDF is committed.

## Authority And Conflict Resolution

1. MOS Technology's original NMOS MCS6500 manuals establish documented CPU
   instructions and bus sequencing. A modern WDC 65C02 manual is not a substitute.
2. NES-specific RP2A03/RP2C02 behavior overrides generic MOS behavior. NESdev and
   Visual6502 record hardware/transistor research; they are not official Nintendo
   manuals. Attribute them accordingly and preserve their test conditions.
3. Actual pinned Lib/Common headers and source establish the integration ABI.
4. MyNes chooses product scope, error policy, initial RAM seed, palette, CLI, limits
   and conversion algorithms. Such choices are labeled as policy, not hardware.

When sources disagree, retain the exact claim and hardware variant, inspect the
earliest primary evidence and add a named probe; never resolve a discrepancy by
copying another emulator. A trace from a reference emulator is corroboration,
not authority. Existing external test ROM names are research references only;
their bytes/source need separate rights review before any future use.

## Consulted Sources And Claim Boundaries

| ID | Author/archive and consulted portion | Design use |
| --- | --- | --- |
| MOS-P | MOS Technology, *MCS6500 Microcomputer Family Programming Manual*, January 1976, second edition, publication 6500-50A; instruction Appendix A and addressing discussion. [Readable archival transcription](https://manuals.plus/m/ce8689ce6ac07440f0c0d8f95f2f921c49692e32e38ce7094e025e399e38551d), [Bitsavers scan locator](https://bitsavers.org/components/mosTechnology/6500-50A_MCS6500pgmManJan76.pdf). | Documented instruction forms, registers, arithmetic/flags and addressing. No NES decimal-mode inference from generic MOS text. |
| MOS-H | MOS Technology, *MCS6500 Microcomputer System Hardware Manual*, January 1976, second edition, 6500-10A, Appendix A. [Archival transcription](https://xotmatrix.com/6502/6502-single-cycle-execution.html). | Ordered per-cycle read/write, addressing, stack/control-flow sequences. Physical timing states differ from an emulator's instruction counter. |
| NES-CPU | NESdev [CPU ALL](https://www.nesdev.org/wiki/CPU_ALL), [Errata](https://www.nesdev.org/wiki/Errata). | RP2A03 binary ADC/SBC despite D, NMOS JMP wrap and status-bit distinctions. |
| NES-RESET | NESdev [CPU power-up state](https://www.nesdev.org/wiki/CPU_power_up_state), whose original observations identify an RP2A03G on NES-CPU-07. | Reset stack/flags and power-versus-reset distinction; observed values are not a universal RAM guarantee. |
| NES-IRQ | NESdev [CPU interrupts](https://www.nesdev.org/wiki/Interrupts), [Visual6502 interrupt timing](https://www.nesdev.org/wiki/Visual6502wiki/6502_Timing_of_Interrupt_Handling), [interrupt hijacking](https://www.nesdev.org/wiki/Visual6502wiki/6502_Interrupt_Hijacking). | Poll timing, I-flag latency, NMI recognition gating and vector takeover. The latter explicitly derives from transistor networks and simulator experiments. |
| NES-MAP | NESdev [CPU memory map](https://www.nesdev.org/wiki/CPU_memory_map). | CPU RAM/device/cartridge address ownership and mirroring. |
| INES | NESdev [iNES](https://www.nesdev.org/wiki/INES), [NES 2.0](https://www.nesdev.org/wiki/NES_2.0). Community-maintained emulator file-format definitions, not physical cartridge headers. | Header fields and format distinction. MyNes's strict accepted subset is separately specified. |
| NROM | NESdev [NROM](https://www.nesdev.org/wiki/NROM), [Programming NROM](https://www.nesdev.org/wiki/Programming_NROM). | Mapper-0 PRG/CHR wiring and limits; distinguish basic NROM from Family BASIC/other RAM boards. |
| MMC1 | NESdev [MMC1 / iNES mapper 001](https://www.nesdev.org/wiki/INES_Mapper_001) and [Programming MMC1](https://www.nesdev.org/wiki/Programming_MMC1). | Five-write serial register, reset, PRG/CHR banking and nametable controls; board-specific RAM variants remain explicitly scoped. |
| MMC3 | NESdev [MMC3](https://www.nesdev.org/wiki/MMC3) and [Programming MMC3](https://www.nesdev.org/wiki/Programming_MMC3). | Register-selected banking and PPU address-edge-qualified IRQ behavior; no inference that all mapper-4 boards share every board variation. |
| PPU-REG | NESdev [PPU programmer reference](https://www.nesdev.org/wiki/PPU_programmer_reference), [PPU scrolling](https://www.nesdev.org/wiki/PPU_scrolling). | Register effects, separate scroll latches and palette buffering. |
| PPU-TIME | NESdev [PPU rendering](https://www.nesdev.org/wiki/PPU_rendering), [frame timing](https://www.nesdev.org/wiki/PPU_frame_timing), [sprite evaluation](https://www.nesdev.org/wiki/PPU_sprite_evaluation). | NTSC dot/scanline scheduling, odd-frame behavior and sprite pipeline. |
| CLOCK | NESdev [cycle reference chart](https://www.nesdev.org/wiki/Clock_rate), [NTSC video](https://www.nesdev.org/wiki/NTSC_video). | Rational NTSC master frequency and CPU/PPU dividers. Initial phase and tie ordering are MyNes's declared alignment. |
| COLOR | NESdev [PPU palettes](https://www.nesdev.org/wiki/PPU_palettes). | Six-bit base color and emphasis must survive per-pixel capture. MyNes's RGB palette/quantizer is its own approximate presentation policy. |
| PAD | NESdev [standard controller](https://www.nesdev.org/wiki/NES_controller). | Strobe, eight-button serial order and post-eight read behavior; not App keyboard defaults. |
| DMA | NESdev [DMA](https://www.nesdev.org/wiki/DMA). | OAM/DMC bus arbitration, halt alignment and repeated read implications. |
| APU | NESdev [APU](https://www.nesdev.org/wiki/NES_APU) and [APU Mixer](https://www.nesdev.org/wiki/APU_Mixer), which attributes the nonlinear model to Blargg's measurements/reference. | Channel/event responsibilities and mixer model; no native host audio design is inferred from hardware. |
| LOCAL | Shared source 0fb40f48: common/machine, common/session, common/ui; lib/kvm-base, kvm-window, kvm-console, storage and base public/source contracts. | Actual callbacks, lifecycle states, 128/1536 debug bounds, 80x25 text, 256-color Window, narrow paths, missing audio capability and growable control queue. |

## Structural And Style References

Owner-directed source inspection for organization and style, not hardware facts:

- NXVM `a10445eb2c269329d2f9d40141b8873dd187bf58`: `src/core/machine/`
  inventory, `machine_interface.h`, `machine.h`, `machine.c`, `memory.h` and
  `timeline.h`. Basis for adjacent device files, opaque public machine, private
  assembly state, owner-prefixed operations and explicit clock/provider records.
- SoftPC `2b17749a12c1132d9c9c65754008befbcb764546`: `src/app/` inventory and
  `main.c`, which delegates configuration/composition to adjacent owners. Basis
  for the four-part product layout; its specific CLI and direct stdio usage are
  not adopted MyNes contracts.

No source was copied or transliterated. These pins do not refresh the accepted
shared corpus and are not build dependencies. Exact style mapping belongs to
Source Layout; mandatory MyNes rules remain authoritative.

## Access And Reproducibility Notes

Bitsavers PDF retrieval returned HTTP 403; do not report a downloaded/hashed scan.
The MOS programming content was inspected through the archival transcription;
OCR can corrupt symbols, so opcode facts are independently organized as the
151-entry matrix and bus cycles cross-checked against MOS-H. The hardware
transcription identifies original publication, edition and appendix. Direct
NESdev page opens sometimes returned 403; the search service supplied indexed
page content, including the full Visual6502 hijacking discussion. Retrieval mode
does not turn a wiki into a vendor specification. Stable edition identifiers are
recorded for MOS; mutable wiki pages are identified by page and consultation date,
not by an invented revision hash.

The design imports no third-party implementation or diagram. Numerical opcode
facts are a newly organized factual ledger. Where exact hardware timing becomes
a runtime claim, the verification matrix requires observable bus/cycle cases;
documentation citation alone is not successful emulator validation.
