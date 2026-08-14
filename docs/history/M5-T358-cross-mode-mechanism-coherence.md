# M5 T358: Cross-Mode Mechanism Coherence

## Task Record

T358 is the cross-mode correctness prerequisite between finite instruction
timing and bus-timed PC/AT work.  It audits a shared mechanism from entry
classification through validation, external side effects, fault delivery, and
architectural publication; it does not flatten Intel-required mode, frame, or
layout differences.

## Accepted S1 Progress

S1 was accepted at `aff0ee43`.  Its [I/O-permission ledger](../etc/evidence/t358-s1-io-permission-coherence.md)
maps the sole `_kpa_test_mode()` / `_kpa_test_iomap()` decision boundary and
every ordinary and string caller.  It restores checked error propagation for
the two legacy word `OUT` tails, adds legacy provider-failure nonpublication,
and extends string map permit/deny proof to both restricted protected mode and
VM86.  No provider ABI, TSS layout, or generic exception-delivery mechanism
changed.

The accepted current-gate result is 240/240 passing.  The active developer
artifact is `build/output/nxvm_0_5_0358.exe`, SHA-256
`5606F34537F86B11B3891980B77F8C48273F4F0903C7AB4EEE8F2DF5617D0181`.

## Accepted S2 Progress

S2 was accepted at `e177f6ca`.  Its [exception and IRQ entry
ledger](../etc/evidence/t358-s2-exception-irq-coherence.md) classifies every
current synchronous producer, final-delivery route, serializer layout, and
NMI/PIC caller.  It corrects the accidental 80386-only `ExecFinal` threshold
for 80286 protected-mode `#SS` and `#TS`; valid 16-bit IDT probes now prove
vector 12/vector 10 error-code frames, restart IPs, and handler entry.  The
existing real, 80386 same/outer, VM86, failed-preflight, and PIC proofs are
retained without flattening their real layout differences.

The accepted current-gate result is 240/240 passing.  The active developer
artifact is `build/output/nxvm_0_5_0358.exe`, SHA-256
`59FAC0E73FED23BA47F3F25C1946C8D2949CFB186C480CE9841588E47E754E2E`.

## Remaining Scope

T358 retains its proposal order: exception/IRQ delivery; segment, descriptor,
table, task, and return transitions; paging and linear memory; and shared
prefix/LOCK/width/string classifiers.  These are separate admitted S units;
S1 does not claim VME/PVI, device behavior, bus timing, or x87 execution.
