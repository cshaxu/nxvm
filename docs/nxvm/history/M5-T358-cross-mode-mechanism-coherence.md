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

## Accepted S3 Progress

S3 was accepted at `14212791`. Its [selector and transition
ledger](../etc/evidence/t358-s3-selector-transition-coherence.md) maps every
current selector/cache materialization route from validation through
publication. It found no remaining construction divergence: separate real,
protected, VM86, 286/386 TSS, gate, and IRET layouts are retained only where
Intel requires them. It reconciles the two outer-return owner smokes with the
accepted S2 80286 `#SS` vector-12 delivery contract; no production path
changed. The accepted current-gate result is 240/240 passing and the developer
artifact SHA-256 remains
`59FAC0E73FED23BA47F3F25C1946C8D2949CFB186C480CE9841588E47E754E2E`.

## Accepted S4 Progress

S4 was accepted at `d55c99f4`. Its [paging and linear-memory ledger](../etc/evidence/t358-s4-paging-linear-coherence.md)
maps page-walk validation, the sole A/D publisher, cross-page cancellation,
guest operands, instruction fetch, system-transition callers, and test/debug
linear access. No second walker, direct A/D writer, or guest bypass was found.
The retained 80386 permission differences and zero-byte segment feasibility
checks remain intentional.

S4 also reconciles a reproducible current-gate host-scheduling flake without
changing guest behavior: all seven current host-thread lifecycle smokes are
`RUN_SERIAL` under CTest, with an assertion that each remains in the canonical
gate. Two fresh parallel current-gate runs passed 240/240. The active developer
artifact SHA-256 remains
`59FAC0E73FED23BA47F3F25C1946C8D2949CFB186C480CE9841588E47E754E2E`.

## Accepted S5 Progress

S5 was accepted at `f2892ebf`. Its [prefix-classifier ledger](../etc/evidence/t358-s5-prefix-classifier-coherence.md)
maps runtime and preview prefix recognition, segment/width/repeat state,
LOCK classification, and all string-family consumers. It repairs two shared
construction divergences: 80386 LOCK now rejects read-only or invalid group
forms before operand decoding, and preview treats repeated `66`/`67` as the
same presence attributes used by execution. The retained prefix owner and
timing-preview smoke prove legal write forms, full illegal-form
nonpublication, and repeated width behavior.

The accepted current-gate result is 240/240 passing. The active developer
artifact SHA-256 remains
`59FAC0E73FED23BA47F3F25C1946C8D2949CFB186C480CE9841588E47E754E2E`.

## Closure

The [task-level closure audit](../etc/evidence/t358-cross-mode-mechanism-closure-audit.md)
accepts all five proposal mechanisms and transfers only the named timing,
physical-bus, device, later-CPU, x87, and Windows boundaries. T358 is closed;
the next Queue candidate consumes this mechanism baseline.
