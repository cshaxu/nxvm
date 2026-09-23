# CPU, Bus And Cartridge Design

Supporting [Architecture](../design/ARCHITECTURE.md). The numerical execution
universe is [Opcode Matrix](cpu-opcodes.csv), not a handpicked test program.
Hardware basis: MOS programming manual Appendix A and addressing chapters;
RP2A03 differences, bus and interrupts use the primary research sources in
[References](hardware-references.md). All unsupported choices below are MyNes
coverage policy, not assertions that the physical chip rejects those operations.

## CPU Mechanism And Interfaces

Use one NMOS 6502/RP2A03 decoder and a resumable per-cycle micro-operation engine.
Do not begin with an instruction interpreter that hides bus cycles and then add
a separate accurate engine. CPU owns registers, instruction temporary values,
micro-operation position, IRQ sample and NMI edge latch. Machine owns clock order,
bus wiring and DMA arbitration. At most one bus transfer occurs per CPU tick.

Proposed interface: opaque `core_cpu`; create/destroy; `core_cpu_power`;
`core_cpu_request_reset`; `core_cpu_tick(cpu, const core_cpu_pins *,
const core_cpu_bus *, core_cpu_tick_result *)`; and copied `core_cpu_observe`.
Create borrows no bus pointer: each tick receives the same production contract.
All calls are executor-only. `core_cpu_pins` contains IRQ/NMI/reset asserted levels;
tick result contains boundary/retired/fault booleans and copied PC/opcode/bus facts.
Bus provider has context plus `read(context,u16,u8*)` and
`write(context,u16,u8)`, returning lib_status. CPU never sees RAM/device layout.
CPU bus callbacks do not advance the CPU or call its tick recursively.

Machine executes a master cycle by advancing devices to the selected CPU bus
phase, servicing the CPU or DMA bus transfer, advancing the remaining device
phases, and updating u64 time. In M2 each CPU tick advances one CPU cycle; there
are no dummy device timers. Tick APIs keep future device phase ordering possible
without changing opcode semantics. Debug STEP uses the same machine loop.
Counters are checked before increment; exhaustion is an explicit host limit,
never wraparound. No longjmp, host clock, file access or writable globals.

For each admitted opcode, the matrix fixes byte length, base cycles and conditional
penalties. CPU table metadata also supplies disassembly, but tests independently
derive expected behavior from this design, not the production table itself.
Unsupported opcode detection occurs after its real opcode fetch; records include
that one elapsed cycle and any resulting data latch change, zero retirement,
and faulting opcode PC. PC may have advanced; observation distinguishes it from
the trap PC. No invented rollback of completed bus actions.

## Addressing And Bus Sequences

All addresses wrap to 16 bits inside hardware, all zero-page offsets to 8 bits;
debug ranges instead reject wrapping. Operand words are little-endian. Memory
stack accesses use $0100|S; push writes then decrements S, pull increments then
reads. The internal B flag is not a storage bit; pushed P sets bit 5 and sets B
only for PHP/BRK. PLP/RTI restore N,V,D,I,Z,C and ignore incoming B/bit 5.

| Address mode | Effective address and bus constraint |
| --- | --- |
| Immediate | Operand at next PC. |
| Zero page, indexed zero page | Offset addition wraps; indexed form performs its pre-index dummy read. |
| Absolute | Fetch low then high operand byte. |
| Absolute indexed read | First read uses original high plus indexed low; page carry adds corrected read. |
| Absolute indexed store/RMW | Always perform the index dummy read before write/RMW, even without crossing. |
| (zero page,X) | Dummy base read, wrapped indexed pointer low/high, then operand. |
| (zero page),Y | Wrapped pointer high fetch; same page-carry read or fixed store dummy-read rule. |
| Relative | Signed displacement from PC after operand; taken branch adds next-PC dummy read, page crossing adds intermediate-page read. |
| JMP indirect | High target byte comes from same pointer page when pointer low byte is $FF (NMOS behavior). |

RMW instructions perform read, write of original value, then write of result;
they do not collapse into a single write. Implied/accumulator operations retain
their dummy read cycle. JSR pushes address of its final operand byte; RTS pulls
and increments. BRK consumes its padding fetch, pushes PC+2, then P with B set;
IRQ/NMI push B clear. JMP, stack instructions and returns preserve their full
bus order from the MOS hardware timing tables/Visual6502 traces, not only totals.
The opcode/branch matrix is a timing target for the admitted NMOS forms, not a
claim that untested PPU races or undocumented silicon behavior are covered.

Arithmetic is binary even with D set on RP2A03. ADC widens A+M+C; SBC widens
A+(M xor $FF)+C; C means carry/no borrow; V uses signed overflow, N/Z use the
truncated result. D remains writable/restorable. Compare sets C/N/Z without
changing its operand register; BIT sets Z from A&M and N/V from memory; shifts
move the shifted-out bit into C. Transfer instructions except TXS set N/Z;
load/inc/dec set N/Z; stores, pushes, jumps and branches preserve other flags.
No 65C02-only opcode, decimal arithmetic or "fix" for JMP-indirect wrap is enabled.

## Reset And Interrupt Contract

Power seed is an emulator reproducibility policy: A/X/Y=0, S=0, P=$20, PC=0,
RAM=0, no pending interrupts. Seven reset cycles load $FFFC/D and leave S=$FD,
I=1. These zero seeds are not a claim about every console's power-up RAM.
Warm reset retains RAM and A/X/Y and non-I flags, decrements S by three through
read-only stack cycles, sets I and reloads the reset vector. Reset has highest
priority; NMI has priority over a simultaneously recognized IRQ. Vectors are
NMI=$FFFA/B, reset=$FFFC/D, IRQ/BRK=$FFFE/F.

NMI latches an assertion edge, not a continuously repeating level. IRQ is level
sensitive and respects I at the hardware polling point. Each instruction sequence
marks its poll micro-operation; CLI/SEI/PLP use the pre-change I at their poll,
while RTI's restored I is visible to its later poll. A branch has its poll on the
pre-branch/fetch timing path; do not unconditionally poll only the last cycle.
Interrupt entry consumes seven cycles and zero retired instructions. Design
tests pin IRQ/NMI transitions to each CPU cycle, including I changes and branch
paths. NMI takeover follows the recognition gates in NES-IRQ, not a test of the
raw pin at the vector read: recognition through internal T4 can redirect IRQ/BRK;
the vector-select interval inhibits a half-hijack. A later held NMI lets the first
handler instruction run before service; a pulse disappearing in that inhibited
window can be lost. The already pushed return/status image is retained. CPU
micro-operation states map those internal gates to the externally observed bus
cycles in the reference traces. Sub-cycle pulse widths and reset half-hijacks
are outside the cycle-sampled pin contract; no transistor-level equivalence is
claimed. Reset requests from the product occur only at a safe boundary.

M2 machine wires external IRQ/NMI deasserted; CPU tests drive the same pin input
contract. M3/M4 devices supply those facts through machine wiring, never through
App commands or a second CPU. Step stops at a retired-instruction boundary after
servicing any preceding recognized interrupt. A cycle budget may expire at an
interrupt-entry boundary before the next instruction retires and reports BUDGET;
it never abandons an ordinary instruction halfway merely to meet its budget.

## Bus And Cartridge Addressing

| CPU range | Owner and M2 behavior |
| --- | --- |
| $0000-$1FFF | Machine 2 KiB RAM, index address&$07FF; zero page/stack use this same memory. |
| $2000-$3FFF | PPU register mirror, address&7; M2 read/write/dummy access traps unsupported device. |
| $4000-$4017 | APU/controller/OAM DMA decoding reserved to machine; M2 access traps unsupported device. |
| $4018-$401F | Test-mode hardware is excluded; M2 access traps unsupported device. |
| $4020-$7FFF | No expansion/PRG RAM in the selected board; reads return current CPU data latch, writes update latch but no storage. |
| $8000-$FFFF | Cartridge PRG; mapper-0 maps/mirrors directly, while admitted mapper-1 consumes serial configuration writes. |

Every successful CPU/DMA read updates the CPU data latch; writes drive/update it.
Open bus returns its prior value. A rejected unavailable-device access reports
the attempted cycle/address and makes no device mutation; prior instruction
cycles stay committed. Cartridge bus-conflict emulation is outside the selected
read-only NROM profile, explicitly not generalized to bank-switching mappers.

Bus owns routing, RAM and the CPU latch. Cartridge owns ROM/CHR/mapping and future
cartridge RAM; PPU owns its bus latch, CIRAM, palette RAM and OAM. These are
different physical latches, not duplicate CPU state. Peek supports RAM and PRG
ROM only; open bus and MMIO return UNSUPPORTED because sampling them as memory
would misrepresent effects. Poke prevalidates the entire range as RAM/mirrors.

## iNES 1 Mapper-0 And M4 MMC1 Acceptance

The parser is bounded pure byte logic. Exact accepted header: bytes 0..3
`4E 45 53 1A`; byte 4 is 1 or 2 PRG banks; byte 5 is 0 or 1 CHR bank; byte 6 is
0 or 1 (horizontal/vertical nametable wiring); bytes 7..15 all zero. Thus battery,
trainer, four-screen, other mappers, console types, NES 2.0, non-NTSC extensions,
explicit PRG-RAM extensions and dirty padding reject as unsupported. Short header
or bad magic/length is invalid input. This is a deliberate strict first-profile
policy; it is not the definition of all legal iNES files.

Expected file size is exactly 16 + PRG*16384 + CHR*8192. Mapper-0 remains
bounded at 40976 bytes; the M4 Dr. Mario MMC1 profile admits exactly 2 PRG
banks and 4 CHR banks, so the current media maximum is 65552 bytes.
Overflow-check before forming offsets or allocation; reject both truncation and
trailing bytes. CHR=0 creates zero-initialized 8 KiB CHR RAM; CHR=1 keeps immutable
8 KiB CHR ROM. M2 owns but does not render CHR. Byte-8 zero is ambiguous in older
iNES practice; for this explicitly selected basic NROM profile it allocates no
PRG RAM, rather than inferring Family BASIC or another board. Format/profile is
shown by `status`, and actual board expansion needs separate compatibility scope.

`core_cartridge_create(out, bytes,size)`, destroy, CPU/PPU read/write and copied
metadata are executor/hardware-only operations. Create copies PRG/CHR and returns
no borrowed spans. PPU $0000-$1FFF uses CHR; CPU and PPU accesses never alias host
pointers through public interfaces. Horizontal wiring maps logical nametables
0,1,2,3 to physical 0,0,1,1; vertical to 0,1,0,1. No mapper plugin registry or
speculative per-mapper class hierarchy: add cohesive mapper logic when admitted.

## Required Proof

CPU closure needs all 151 rows plus rejection of all 105 other bytes, not only
a known test ROM reaching a final address. Address-mode cases cover no-cross,
cross, zero-page/stack/address wrap and side-effectful dummy operations. Arithmetic
covers all A/M/C combinations for ADC/SBC and both D values. Interrupt/reset tests
assert ordered bus traces, stack values and cycle counts, not only final PC.
ROM tests cover each rejection bit/byte, exact-size boundary, both PRG layouts
and CHR variants. [Verification](app-core-verification.md) assigns these to the
future production test owners without claiming any test has run already.
