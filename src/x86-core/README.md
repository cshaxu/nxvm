# NXVM Shared X86 Core

CPU and FPU mechanisms, with no PC motherboard, firmware, media or host-product
policy. `cpu` owns architectural state, decoding, execution, exceptions, bus
requests and timing selection; `fpu` owns its existing partial x87 implementation.
CPU links FPU honestly. Selecting `X86_FPU_PROFILE_NONE` does not install a guest
coprocessor. Public contracts are the adjacent `*_interface.h` files; all other
headers and layouts are private to their owner.

Only Lib Types and this package are allowed dependencies. CPU may consume FPU's
public interface; FPU does not depend on CPU. All existing CPU families, timing
tables, grade restrictions and narrow instruction-file warning exceptions are
preserved. Extraction does not qualify a new CPU or open production admission.

The execution Machine remains outside this package until its separately admitted
neutralization. No empty future machine directory or compatibility CPU path is
provided. PC wiring belongs to the consuming board, not CPU/FPU.

Production targets are `x86-core-cpu` and `x86-core-fpu`; `x86-core-verify` checks
this source package only. The matching test package owns its direct CPU/FPU
tests, decoder programs and boundary checks. Repository qualification against
original timing ledgers remains an outer consumer, not a standalone dependency.
