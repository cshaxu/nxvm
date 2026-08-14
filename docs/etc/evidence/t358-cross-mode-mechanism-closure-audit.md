# M5 T358: Cross-Mode Mechanism Closure Audit

## Audit Basis

T358's approved proposal requires one decision-to-publication ledger for five
shared CPU mechanisms spanning real, protected, and virtual-8086 execution.
This closure audit re-reads that proposal, the accepted S1--S5 evidence,
current source owners, Queue, TODO, and the full current gate. It does not
promote CPU form coverage, physical timing, devices, or later-CPU state into
T358.

## Requirement Reconciliation

| Proposal mechanism | Accepted owner and evidence | Closure disposition |
| --- | --- | --- |
| I/O permission | `_kpa_test_mode` / `_kpa_test_iomap`, ordinary and string port handlers; [S1 ledger](t358-s1-io-permission-coherence.md). | Accepted. CPL/IOPL/TSS-map and VM86 permit/deny reach one checked provider boundary with failure nonpublication. |
| Exception and IRQ delivery | `ExecFinal`, real/protected serializers, `ExecInt`; [S2 ledger](t358-s2-exception-irq-coherence.md). | Accepted. Real, 16/32 protected, VM86, synchronous fault, NMI/PIC and restart/frame distinctions retain their separate Intel layouts. |
| Selector, descriptor, table, task, and return transitions | Segment loaders, gate/return serializers, task plan/commit; [S3 ledger](t358-s3-selector-transition-coherence.md). | Accepted. No second materializer or width-construction fork remains; genuine 16/32 TSS and frame layouts remain specialized. |
| Paging and linear access | Common translation prepare/commit, logical accesses and final delivery; [S4 ledger](t358-s4-paging-linear-coherence.md). | Accepted. One walker and A/D publisher serve guest, fetch, system-transition, and fixture/debug callers; cross-page cancellation and CR2/#PF boundaries are retained. |
| Prefix, LOCK, width, and strings | Prefix state producers, `ExecIns`, shared string consumers, timing preview; [S5 ledger](t358-s5-prefix-classifier-coherence.md). | Accepted. Illegal 80386 LOCK group forms are rejected at the shared classifier and repeated 66/67 previews agree with execution. |

## Similar-Issue And Transfer Review

The five owner-ledgers collectively searched all adopted callers and found no
remaining duplicate permission test, exception serializer, selector/TSS
materializer, page walker/A-D publisher, or prefix decision path. The two S5
repairs were shared-owner corrections, not local test exemptions.

The excluded state remains explicitly owned elsewhere: complete instruction
timing and LOCK timing rows transfer to the next Queue candidate; physical
LOCK/bus/prefetch and device-service behavior transfer to the ordered
bus-timed/service/cycle-exact candidates; VME/PVI, persistent TLB/test-register
state, numerical x87, and Windows behavior remain Queue or TODO boundaries.
No T358 mechanism row is left Partial, Missing, or unclassified.

## Verification And Result

Fresh configuration plus the default parallel current gate passed 240/240.
The developer artifact verifier passed and `nxvm_0_5_0358.exe` remains SHA-256
`59FAC0E73FED23BA47F3F25C1946C8D2949CFB186C480CE9841588E47E754E2E`.
Documentation governance and `git diff --check` passed before acceptance.

T358 is closed. The next Queue candidate, complete instruction-timing corpus,
consumes this coherent CPU mechanism baseline and must return any newly found
cross-mode discrepancy to its named mechanism owner rather than create a
parallel route.
