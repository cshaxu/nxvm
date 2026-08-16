# T386 S17: Model 40 Dual DMA Topology

`M5:T386:S17:DUAL-DMA-TOPOLOGY:OK`

`M5:T386:S17:DMA-WORD-CASCADE:OK`

`M5:T386:S17:DMA-RESET-BINDING:OK`

## Scope And Ownership

T384 S1 selects two 8237A-5 controllers for the original Model 40: byte
channels 0--3, word channels 5--7, and the first controller cascaded through
word-controller channel 4. The documented four-megahertz DCLK and one wait per
DMA cycle are board-timing questions, not functional configuration.

Core owns the reusable two-controller 8237A mechanism, its register ports,
request binding, cascade scheduling, word-address transfer semantics and reset.
S17 makes the supported generic topology explicit in frozen DMA wiring: two
controllers, cascade channel 4, and an embedded FDC channel constrained to the
byte controller. Core carries no Compaq identifier, address or device policy.

VM owns selection. Both retained PC/AT composition and private Model-40
composition explicitly request the generic topology; Model 40 selects FDC DMA2.
The Model-40 code neither reimplements DMA controller state nor manufactures a
test endpoint.

## Functional Proof

- `core-machine-dma-rtc-authority-smoke` rejects a malformed generic topology
  before configuration, retains the accepted topology, both peer links, FDC
  binding and both reset masks.
- Retained `core-machine-dma-channel-smoke` proves generic byte/word channel
  transfer, sparse secondary-controller ports, page semantics and
  request/cascade scheduling.
- New `vm-model40-dma-s17-smoke` creates only the selected private session. It
  proves the frozen two-controller/channel-4 configuration, secondary port
  ownership, FDC DMA2 binding, a primary request visible through the cascade,
  and reset clearing controller request/mode state while retaining binding.
- Focused CTest passes all three controls. The serial current gate passes
  272/272 tests. T345 direct-ownership verification reports 219 rows: 159
  owner-test, seven embedded-production, one type-foundation, two safely
  separable, 50 mixed/inherited and 57 residual-production entries.
- The runnable developer artifact is vm-0-5-0388,
  build/output/nxvm_0_5_0388.exe, SHA-256
  1B870632BA6A1F3EA15296285CBF13A76CD03BC8D4E9201E4CD5D8345D93FC04.

## Similar-Issue Sweep

The S17 sweep searched all tracked DMA-wiring callers, controller port and
cascade code, Model-40 composition and DMA-focused machine tests. Every
existing composition caller now declares the same supported generic topology;
the default PC/AT selection is a matching shared consumer, not a Model-40
exception. Core's controller implementation already owns both chips, word
channels, channel-4 cascade and reset, so it is validated rather than copied.
No second transaction path or test-only production entry point is added.

No selected Model-40 onboard device currently owns channels 0, 1, 3, 5, 6 or
7. They remain correctly unbound controller channels, not absent hardware and
not synthetic product endpoints. An expansion-card device contract requires a
separate device/profile admission.

## Transfer Boundary

S17 is selected board-functional progress, not a timing or L3 result. DCLK
frequency, one-wait service duration, DMA/PIC/CPU arbitration, bus availability,
HRQ/HLDA/DACK/AEN pin behavior, and expansion-card endpoint behavior transfer
to the DeskPro board and device timing candidate. The selected 1.2 MB drive,
remaining CECG behavior, D4 parity/firmware copy, reset arbitration and other
ledgered device functional receivers remain open under T386.

## Coordinator Acceptance

P1 3aafba33 delivers the complete generic topology validation, explicit VM
selection, dedicated Model-40 regression, artifact update and evidence. The
coordinator rejected its initial acceptance only for a locally visible source
formatting defect in the new smoke and its missing terminal newline. P2
a616904b corrects only those delivery defects; it changes neither behavior nor
scope.

The coordinator reviewed the full P1--P2 source range and every changed
production, test, build and documentation file. Core has only generic
controller-count/cascade validation and no new Compaq policy. VM selects the
generic topology without duplicating DMA state or transactions. The caller
sweep covers every wiring initializer; the new smoke uses no production
test-only API. Focused controls, the 272/272 serial current gate, artifact,
T345 ownership and documentation governance all pass from the reviewed graph.

S17 is accepted as bounded Model-40 functional progress. It does not claim
DCLK or bus timing, expansion endpoint support, full selected-device
completeness, firmware execution or DeskPro L3 closure. T386 remains open for
the next functional ledger receiver.
