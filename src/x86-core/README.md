# Shared X86 Core

Standalone CMake projects are `shared-x86-core` (source) and `shared-test-x86-core` (tests).

CPU, FPU and neutral execution mechanisms, with no PC motherboard, firmware
selection, media or host-product policy. `cpu` owns architectural state, decoding, execution, exceptions, bus
requests and timing selection; `fpu` owns its existing partial x87 implementation.
CPU links FPU honestly. Selecting `X86_FPU_PROFILE_NONE` does not install a guest
coprocessor. Public contracts are the adjacent `*_interface.h` files; all other
headers and layouts are private to their owner.

Only Lib Types and this package are allowed dependencies. CPU may consume FPU's
public interface; FPU does not depend on CPU. Machine consumes their public
contracts and owns the single guest timeline, bus transactions, memory/port
routes, bounded execution, lifecycle and copied observation. CPU/FPU never
depend on Machine. All existing CPU families, timing
tables, grade restrictions and narrow instruction-file warning exceptions are
preserved. Extraction does not qualify a new CPU or open production admission.

PC wiring, device order, A20 policy and Boolean pin adaptation belong to the
consuming board. Machine receives explicit construction values, address decode
and bounded attachment callbacks; it has no PC default, second host worker or
compatibility Core path. Immutable ROM mapping is a byte/route mechanism, not
a BIOS implementation or a firmware selection policy.

The existing memory route resolver admits CPU-only physical overlays; DMA
retains ordinary memory decoding. A board-owned decode transition may invalidate
the affected CPU cache/prefetch view without advancing time. This is distinct
from a snooped memory write and does not honor the write-snoop disable bit.

Production targets are `x86-core-cpu`, `x86-core-fpu` and `x86-core-machine`.
`x86-core-machine-observable` is the same execution source built with existing
trace observation enabled for owner tests. `x86-core-verify` checks
this source package only. The matching test package owns its direct CPU/FPU/Machine
tests, decoder programs and boundary checks. Repository qualification against
original timing ledgers remains an outer consumer, not a standalone dependency.
