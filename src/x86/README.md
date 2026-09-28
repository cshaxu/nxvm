# x86 corpus

This package selects C11 without extensions in standalone and embedded builds.
GNU/Clang builds enable -Wall -Wextra -Wpedantic -Werror in this package only.

Architecture-specific copied protocols, chip mechanisms and DOS-style debug/assembly tools.
Products explicitly add this corpus; Common and Lib never depend on it.
There is no executor, Console, input loop or product state machine here.

| Component | Responsibility | Public interface |
| --- | --- | --- |
| debug | Original DOS/X command implementation and copied x86 protocol | debug_interface.h, protocol_interface.h |
| xasm32 | x86 byte/text assembly and disassembly | xasm32_interface.h |
| devices/pit825x | 8253/8254 counters, register protocol, GATE/OUT and input-clock deadlines | pit825x_interface.h |
| devices/rtc146818 | MC146818-compatible calendar, registers, IRQ/SQW and configured-time deadlines | rtc146818_interface.h |
| devices/pic8259 | Single 8259 priority, ICW/OCW, interrupt selection/acknowledge and delivery deadlines | pic8259_interface.h |

Debug depends on Common Machine, xasm32, Lib Storage and Types. xasm32 depends
only on Types. Public names use x86_debug_/X86_DEBUG_ and x86_xasm32_/X86_XASM32_.
Build targets are x86-debug, x86-xasm32, x86-pit825x, x86-rtc146818 and x86-pic8259. Private includes stay component-local;
no native platform code or importing-product source dependency is allowed.

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

Keep src/x86, src/common and src/lib as sibling corpora. For example:

```text
cmake -S src/x86 -B build/x86-corpus -DCMAKE_BUILD_TYPE=Release
cmake --build build/x86-corpus
cmake --build build/x86-corpus --target x86-verify
```

For a chip-only receiver, configure with `-DX86_BUILD_TOOLS=OFF` and build
`x86-pit825x`, `x86-rtc146818` or `x86-pic8259`. This does not configure Common or link Debug/assembly tools.

MANIFEST.sha256 covers every file with exact LF-normalized SHA-256 values.
x86-verify reuses Common's manifest checker with this corpus root; the x86-owned
source/build gate checks allowed edges and private/platform boundaries. It needs
no importing-product paths. Tests and x86 negative probes live in test/x86.
The shared set is src/lib, src/common, src/x86, test/lib, test/common, test/x86.
The four-directory neutral subset omits both x86 directories entirely. Each
test suite builds independently; test/x86 reuses the neutral machine fixture
from test/common. No receiving emulator is implemented here.
