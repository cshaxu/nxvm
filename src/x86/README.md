# x86 corpus

This package selects C11 without extensions in standalone and embedded builds.
GNU/Clang builds enable -Wall -Wextra -Wpedantic -Werror in this package only.

Architecture-specific copied protocols, chip mechanisms and DOS-style debug/assembly tools.
Products explicitly add this corpus; Common and Lib never depend on it.
Core owns the guest executor; no host worker, Console, input loop or product
state machine belongs here.

| Component | Responsibility | Public interface |
| --- | --- | --- |
| core | Neutral CPU/FPU execution, guest timeline, bus transactions, RAM/port/ROM routes and bounded debug operations | machine_interface.h and adjacent *_interface.h contracts |
| debug | Original DOS/X command implementation and copied x86 protocol | debug_interface.h, protocol_interface.h |
| xasm32 | x86 byte/text assembly and disassembly | xasm32_interface.h |
| chips/cpu | 8086 through 80386 execution state, instruction decoding and CPU-local timing | cpu_interface.h |
| chips/pit825x | 8253/8254 counters, register protocol, GATE/OUT and input-clock deadlines | pit825x_interface.h |
| chips/rtc146818 | MC146818-compatible calendar, registers, IRQ/SQW and configured-time deadlines | rtc146818_interface.h |
| chips/pic8259 | Single 8259 priority, ICW/OCW, interrupt selection/acknowledge and delivery deadlines | pic8259_interface.h |
| chips/dma8237 | Single 8237A register protocol, requests, priority and input-clock service phases | dma8237_interface.h |
| chips/kbc8042 | Qualified AT controller transport, translation, command/register and IRQ behavior | kbc8042_interface.h |
| chips/keyboard | AT keyboard commands, BAT, scan-set, LEDs and typematic | keyboard_interface.h |
| chips/ps2mouse | Existing three-byte AUX command/report protocol | ps2mouse_interface.h |
| chips/ppi8255 | Qualified Mode-0 direction, output latches and BSR | ppi8255_interface.h |
| chips/xtkeyboard | XT nine-bit serial delivery, FIFO and reset/BAT | xtkeyboard_interface.h |
| chips/fdc8272 | 8272 command phases, drive positions and interrupt causes | fdc8272_interface.h |
| chips/hdc | ATA, Compaq/WD, WD1003 and Xebec command/state families | hdc_interface.h |
| chips/video | Video registers, VRAM, raster state and copied frames | video_interface.h, video_values_interface.h |
| chips/fpu | Existing partial 8087 arithmetic and 8087/287/387 extension completion model | fpu_interface.h |

Debug depends on Common Machine, xasm32, Lib Storage and Types. xasm32 depends
only on Types. Public names use x86_debug_/X86_DEBUG_ and x86_xasm32_/X86_XASM32_.
Core depends only on Types, CPU and FPU. `x86-core` is its production target;
`x86-core-observable` compiles the same implementation for trace-contract tests.
Each machine links one variant. An opaque Core handle owns one copied attachment
binding and finalizes that attachment before execution resources. Products own
board wiring, clocks, topology, firmware choices and media; none are Core build
inputs. Core alone advances guest time. Existing `core_machine_*` names remain
stable. Private layouts are component-local; public copied/value contracts and
bounded operations use `_interface.h` headers.
Build targets are x86-debug, x86-xasm32 and x86-<device-directory>. Private includes stay component-local;
no native platform code or importing-product source dependency is allowed.

CPU depends on Types and the FPU public interface. It owns one opaque execution
context and accepts only copied profile values plus a callback-only bus; a board
owns address mapping, port routing, interrupts and the clock that schedules
execution. The source is the legacy instruction engine moved without semantic
rewriting. Its two historical implementation files retain the old non-strict
diagnostic scope during this location cutover; CPU receipt closes only after the
final receiving audit removes that exception or records a justified successor.

PIT depends only on Types. Its opaque instance owns counter state; the board
owns port addresses, clock conversion and OUT consumers (interrupt, refresh,
speaker). Reset/destroy release live output levels while borrowed sinks are
still alive. Reads may consume latches; an unprogrammed counter preserves its
input bus byte. Advance consumes chip input-clock cycles, not host time; the
deadline query reports the next output change or INVALID_STATE when none is
scheduled. Calls have one execution owner. No chip includes another chip's
private state. Waveform rules are unchanged by extraction.

RTC likewise depends only on Types. It owns the calendar, phases and 64-byte
register/RAM bank. Direct register C reads acknowledge IRQ; reset/destroy
release an asserted output while the borrowed sink is alive. The board owns
index/NMI latches, PIC routing, clock conversion, seed/checksum and timing
provenance. Invalid-month bounds containment does not qualify undocumented
calendar programming as hardware-accurate.

PIC depends only on Types. Each opaque controller owns its registers, priority,
resolved input levels and configured unmask countdowns. The board owns port
addresses, source aggregation and master/slave wiring. Cascade signals cross
as copied values, never peer pointers. Selection is non-mutating; acknowledge
and command-register poll consume requests. Reset preserves configured timing.
Calls have one execution owner; no native wait or host time enters the chip.

DMA depends only on Types. One opaque controller owns its registers, requests,
priority and service phases. The caller grants a local or delegated cascade
slot and advances one input clock at a time. A scoped cycle provider receives
local addresses; the board owns page/lane expansion, memory/device effects,
pair wiring and transaction preflight. A failed cycle releases service without
advancing address/count. Providers are not retained and may change request/EOP
inputs, but cannot recursively advance/reset/destroy the controller. Undefined
reads preserve the bus byte; status reads consume TC bits. Copied signal queries
do not mutate state. Stop execution before destroying the instance.

Keyboard and PS/2 mouse depend only on Types. Controller connections carry
copied endpoint inputs and synchronous byte/signal callbacks, never endpoint
objects. Its only keyboard dependency is the stateless scan-code conversion
table. The board owns ports, IRQ routing, A20/reset, construction and clock
conversion. The controller owns transport queues and delivery phases; the
endpoints own command parameters, BAT/typematic or mouse packet state. Resend
history retains the existing accepted-output semantics. This is the qualified
AT model, not a complete 8042 MCU or new protocol/timing qualification.
Reset preserves configured timings; stop execution before destruction and
keep callback contexts alive until all chips are destroyed. The public headers
define the limited reply/repeat reentrancy; no recursive command or destruction
is allowed. No private state getter is provided for tests or diagnostics.

PPI and XT keyboard each depend only on Types, not one another. The board
supplies copied input pins and owns its receiving latch, IRQ, DIP, parity and
speaker wiring. The qualified PPI subset retains documented baseline behavior
for unsupported mode encodings; it does not claim complete 8255 silicon.
XT keyboard timings are frozen service-unit durations supplied by composition.
Failed byte acceptance retains the completed frame without more serial edges;
receiver readiness or line release retries it. Reset discards pending data.
Neither component owns a board clock, port registry, host input or scheduler.

FPU depends only on Types. Its opaque instance owns stack/control/status,
BUSY/ERROR and remaining completion time. CPU pairing and operand memory cycles
belong to the caller. Advance consumes the existing source-axis L2 interval;
complete_wait consumes its remainder once. Reset keeps the frozen variant and
clears pending work. One execution owner serializes calls and ends all use
before destruction. This preserves the limited arithmetic and timing model;
it does not claim a complete 8087, 80287 or 80387 implementation.

The protocol header is independent of the frontend: product adapters need not
link the CLI to use its values. Aligned typed request/response copies traverse
Machine's existing paused lease and bounded byte rendezvous. Machine does not
interpret operations, addresses, registers or execution plans. No new executor.

The command state is the public opaque debug object, not an allocated forwarding
wrapper. Its fixed argument table is embedded. Output is a growable object-owned
string borrowed until the next submit/observe/open/destroy. Result prompts retain
the original address, byte, flags or colon suffix. Consumers must copy text before
retaining it across a producing call. Disassembly byte count differs from text
length; use only the former to advance an instruction address.

Linear byte ranges fit the 32-bit address space. XM copies overlap in the safe
direction; XS only reports complete matches within its byte count. XU retains
its full instruction count and stops on decoding failure or address exhaustion;
XA ends at exhaustion. XE/XF preserve incremental validation, without rollback
of writes preceding an invalid byte. Relocation changes none of these semantics.

## Build and verification

Video memory inspection returns the same selected CGA/planar bytes as a CPU
read without updating EGA latches. Callers serialize both operations with the
device owner; inspection is not a concurrent snapshot or guest bus cycle.

Keep src/x86, src/common and src/lib as sibling corpora. For example:

```text
cmake -S src/x86 -B build/x86-corpus -DCMAKE_BUILD_TYPE=Release
cmake --build build/x86-corpus
cmake --build build/x86-corpus --target x86-verify
```

For a chip-only receiver, configure with `-DX86_BUILD_TOOLS=OFF` and build
any `x86-<device-directory>` target. This does not configure Common or link
Debug/assembly tools.

MANIFEST.sha256 covers every file with exact LF-normalized SHA-256 values.
x86-verify reuses Common's manifest checker with this corpus root; the x86-owned
source/build gate checks allowed edges and private/platform boundaries. It needs
no importing-product paths. Tests and x86 negative probes live in test/x86.
The shared set is src/lib, src/common, src/x86, test/lib, test/common, test/x86.
The four-directory neutral subset omits both x86 directories entirely. Each
test suite builds independently; test/x86 reuses the neutral machine fixture
from test/common. No receiving emulator is implemented here.
