# x86 corpus

This package selects C11 without extensions in standalone and embedded builds.
GNU/Clang builds enable -Wall -Wextra -Wpedantic -Werror in this package only.

Architecture-specific chips/Core, copied protocols and DOS-style debug/assembly tools.
Products explicitly add this corpus; Common and Lib never depend on it.
Core owns guest execution; there is no host worker, Console, host input loop
or product state machine here. SoftPC keeps its existing original executor.

| Component | Responsibility | Public interface |
| --- | --- | --- |
| core | CPU/FPU execution, guest time, memory/port routes and bounded inspection | machine_interface.h and adjacent *_interface.h |
| chips | CPU and individual device register/state mechanisms; board wiring is external | each chip's *_interface.h |
| debug | Original DOS/X command implementation and copied x86 protocol | debug_interface.h, protocol_interface.h |
| xasm32 | x86 byte/text assembly and disassembly | xasm32_interface.h |

Debug depends on Common Machine, xasm32, Lib Storage and Types. xasm32 depends
only on Types. Public names use x86_debug_/X86_DEBUG_ and x86_xasm32_/X86_XASM32_.
Build targets are x86-debug and x86-xasm32. Private includes stay component-local;
no native platform code or importing-product source dependency is allowed.

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

MANIFEST.sha256 covers every file with exact LF-normalized SHA-256 values.
x86-verify reuses Common's manifest checker with this corpus root; the x86-owned
source/build gate checks allowed edges and private/platform boundaries. It needs
no importing-product paths. Tests and x86 negative probes live in test/x86.
The shared set is src/lib, src/common, src/x86, test/lib, test/common, test/x86.
The four-directory neutral subset omits both x86 directories entirely. Each
test suite builds independently with its own fixtures and shared tools directly
in test/. No receiving emulator is implemented here.

The complete imported corpus also contains chips/{cpu,pit825x,rtc146818,pic8259,
dma8237,fdc8272,hdc,fpu,video,kbc8042,keyboard,ps2mouse,ppi8255,xtkeyboard} and
core. Chips expose copied values and borrowed bus callbacks; Core owns guest
execution/time, not a host worker or product presentation. PC composition is
outside this package. SoftPC does not connect this additional CPU/Core to its
existing executor. X86_BUILD_TOOLS=OFF permits chip/Core builds without Common.
The two retained CPU instruction files keep NXVM's inherited GNU -w setting;
other sources select C11 and the strict shared warning profile. The upstream
Debug format-truncation warning exception remains target-local.
