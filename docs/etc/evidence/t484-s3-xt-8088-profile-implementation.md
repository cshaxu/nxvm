# T484 S3 B1 8088 Profile Implementation

`M5:T484:S3:XT-8088-QUEUE:OK`

`M5:T484:S3:XT-FIXED-PROFILE:OK`

## One CPU Owner

`CORE_MACHINE_CPU_PROFILE_8088` is a distinct Core profile.  It uses the
existing 8086 semantic decoder/executor through the explicit shared-semantics
predicate; no decoder, architectural-register state, VM fetch path or timing
state is copied.  The 8088-specific queue capacity is four bytes and its sole
producer reserves and fetches one byte inside the existing Core CPU execution
context.  The owner compacts consumed bytes before it reserves another byte.

The explicit producer is deliberately not a physical-bus-duration claim.  The
Intel four-clocks-per-16-bit-transfer relation remains a later XT timing input;
this B1 implementation retains compatibility/L2 execution cost rather than
borrowing the 8086 timing table or inventing a 4.77 MHz guest clock.

The focused queue regression verifies the four-byte capacity, one-byte
producer transition, retained already-prefetched byte after a memory write,
and the one Core queue flush used by control-transfer paths.  It does not
claim a physical BIU refill schedule.

## Fixed Construction Declaration

`ibm-5160-model-268` is a resolver-owned, copied declaration with only the B1
facts: 8088, 256 KiB and no FPU.  It allows no session options and has no port,
memory-window, IRQ or DRQ declaration.  Its sole enabled bit identifies this
construction-only Core input; it is not a board-device binding.

The normal session request authority recognizes that exact profile.  It
rejects CPU, FPU, RAM and media variants, resolves the fixed declaration, and
then returns `TYPE_STATUS_UNSUPPORTED`.  This is intentional: the current Core
minimum-memory and session plan require the B2 XT board/memory topology, so a
256 KiB XT must not be misrepresented as a runnable AT-derived plan.

## Focused Verification

The following completed after the implementation:

- `core-machine-cpu-profile-gate-smoke` current-gate CTest: pass.
- `vm-xt-5160-268-profile-smoke` current-gate CTest: pass.
- `core-machine-cpu-fpu-profile-smoke`: pass.
- `vm-session-profile-smoke`: pass.

The final full command, `ctest --test-dir build/mingw-gcc-x64
--output-on-failure --no-tests=error --label-regex '^current-gate$' -j 4`,
passed all 297 current-gate tests.  The first run exposed two real integration
issues, both corrected before this result: the 8086 decoder ledger now accepts
the explicit shared-8086-semantics predicate while retaining its 8086 timing
selector check, and the pre-existing generic reservation contract remains
unchanged for non-8088 profiles.  Only the 8088 profile executes the producer.

The optimized stripped Release target `vm-0-5-0484` produced
`build/output/nxvm_0_5_0484.exe` (1,217,787 bytes), SHA-256
`9AEB6BA6F9084270A9902717D60E921E2CD0164189477B7B541BDF3A26F36650`.

The CMake owner-test inventory now has 180 pure and 3 mixed targets; the new
fixed-profile smoke is a current-gate test rather than an unregistered proof.

## Simplicity Accounting

Across S3 P1 and its gate correction, the tracked production/test path diff is
294 added and 31 removed lines (net +263), excluding documentation, generated
files and the Release executable.  The positive delta is the one new immutable
XT declaration and its focused proof; no parallel decoder, VM fetch cache,
timing model or board-device path was added.  The retained production paths
are the shared Core semantic executor, its 8088-only producer, and the single
session request authority.
