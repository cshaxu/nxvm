# T576 S2: PC110 protected unreal-data reload

## Boundary

The admitted external TFT BIOS reaches `F000:605F` after T575's matching-cache
receiver.  Its `POP DS` consumes selector `0008` while PE is set.  The selected
GDT descriptor is exactly zero and no visible data register has selector
`0008`, so the T575 receiver correctly does not apply.

External PC110 projects describe real-address-style segment reloads during
this firmware's protected-mode execution.  That is L2 behavioral evidence
only.  This task does not copy their implementations, use their PC110-specific
selector exceptions, or claim physical CPU behavior.

## Delivery

X86 Core adds an explicit opt-in L2 policy:
`X86_CORE_CPU_PROTECTED_DATA_RELOAD_VISIBLE_CACHE_OR_REAL_ADDRESS`.
It changes only a protected **data** reload with an all-zero descriptor.  It
first runs the existing matching-visible-cache receiver; only if no matching
cache exists does it use the existing real-address data-segment initializer.
The result is `selector << 4`, a 64 KiB 16-bit writable data segment.

Strict remains the default.  The prior cache-only policy is unchanged.  Code,
stack, nonzero invalid descriptors, far transfers, gates, return frames and
privilege rules remain strict.  MyPC110 alone selects the new policy.

## Verification

- `x86-core.cpu/cpu_segment_selector` proves strict rejection, existing cache
  reuse, and the no-cache `0008`-style real-address result.
- `mypc110.diagnostic/real-firmware-post` now stops at `F000:6060` after the
  real BIOS executes the second `POP DS`; it observes `DS=0008`, base `00000080`
  on x64 and x86.
- All seven MyPC110 unit/diagnostic tests pass on both host widths.
- Source and test X86 Core manifests verify after the policy and regression
  changes.

A bounded exploratory run with the controlled watchpoint removed executes past
the new receiver, reaches the existing `INT 10h` path, and later fails after
the BIOS transfers through an uninitialized vector.  It records no C000 entry
and no EC/ED shadow sequence.  This establishes the next need: code/stack/far
control behavior would have to be independently modeled.  S2 deliberately
does not expand a data-only L2 policy into such a general loose-protected-mode
emulator.

No POST, Setup, DOS boot or published MyPC110 artifact is claimed.
