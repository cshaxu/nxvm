# T456 S2: PIC Logical Acknowledgement Contract

## Scope and authority

This evidence consumes `PIC-F8`, `PIC-T2` and `PIC-T5` from the retained
18-row T450/T456 PIC ledger.  The normative Intel 8259A `231468-003` source,
rendered in T456 S1, specifies that the first 8086/88 `INTA` pulse transfers
the selected request from IRR to ISR and that the second supplies the vector.
It does not provide a Core-usable elapsed duration; this change deliberately
makes no electrical timing claim.

## One-owner implementation

Before this S, the sole CPU route was `scan`, non-mutating `peek`, CPU
interrupt-frame/vector entry, then `get` (the PIC IRR-to-ISR transition).
That made the architectural vector entry observable before the first logical
acknowledgement.

The same sole route is now:

1. CPU observes a pending request through PIC selection.
2. CPU opens and commits one `CPU_INTERRUPT_ACKNOWLEDGE` transaction.
3. `pic.c` alone selects and acknowledges the request, moving IRR to ISR and
   returning its reserved vector.
4. The existing CPU interrupt-entry path consumes that vector and writes the
   frame.

No PIC state moved to CPU or VM, no public ABI changed, and no second delivery
or timing path was added.  The new transaction kind records a logical phase
only; its zero address/value/detail fields do not encode a fabricated bus
duration or waveform.  Transaction reset remains the sole cancellation owner;
the focused test confirms reset leaves no active owner or retained counters.

## Similar-issue sweep

`rg -n 'core_machine_pic_(scan|peek|get)_interrupt' src tests` found one
production `get` consumer: `src/core/machine/cpu_instructions.c`, now fixed.
All remaining `get` consumers are PIC-local controller smokes; they directly
exercise the controller acknowledgement owner and are not CPU delivery paths.
`scan` and `peek` remain non-mutating PIC observation APIs, with no production
delivery bypass.  No additional production hit requires a TODO transfer.

## Verification

`core-machine-pic-phase-s2-smoke` records the transaction trace through the
public Core trace provider.  It proves a pending IRQ0, a contiguous CPU
acknowledgement begin/commit pair, at least one subsequent CPU frame-memory
write, IRR clear/ISR set after delivery, and reset without retained transaction
state.  The established protected-mode rejection smokes additionally prove
that a gate/TSS validation fault preserves CPU and stack atomicity while the
already acknowledged external PIC request remains in ISR rather than being
incorrectly restored to IRR.

- `cmake --build --preset current-gates-gcc --target core-machine-pic-phase-s2-smoke`: pass.
- `ctest --test-dir build/mingw-gcc-x64 --output-on-failure -R '^current\\.(core-machine-(pic-(irq-lifecycle|command-priority|ocw3|lifecycle-s4|phase-s2)-smoke|hardware-delivery-s3-smoke|pit-irq0-s2-smoke|rtc-cmos-s3-smoke|kbc-controller-smoke|kbc-aux-port-smoke)|vm-cmos-rtc-port-smoke)$'`: 11/11 pass.
- Isolated full `current-gate` CTest replay: 293/293 pass in 103.78 seconds.

The changed mechanism is one transaction enum member, its bounded validator,
the existing CPU delivery sequence, and one focused smoke/registration.  The
retained `peek` observer is not a duplicate dispatch path.  ICW3/SNGL topology
consumption remains the distinct S3 receiver; electrical INTA waveforms and
spurious-race behavior remain outside this logical L3 contract.

The ten tracked source/build/test paths add 156 and remove 18 lines (net
+138), counted with `git diff --cached --numstat` and excluding documentation.
The 122-line focused smoke supplies the observable phase and reset proof;
production changes are 18 added and 6 removed lines across the four owning
Core files.  No wrapper, compatibility path, duplicated PIC state, or second
CPU dispatch mechanism remains.
