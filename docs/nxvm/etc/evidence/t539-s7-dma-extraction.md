# T539 S7: DMA Extraction

Baseline: 0ceb739f6. Targets: Shared and NXVM, in separate P commits.
The owner authorized automatic bounded S admission. This delivery implements
the [DMA boundary](../architecture/t539-s7-dma-boundary.md), not a new hardware
qualification or a change to existing L1/L2/L3 classifications.

## Ownership And Source Disposition

- `src/x86/devices/dma8237` owns the single controller's registers, flip-flop,
  request/mask/priority, temporary byte, grant and service phases. It links only
  Types. The state is opaque; there is no App, Common, peer-controller, RAM,
  page-latch, machine-clock or host-resource dependency.
- `src/app-nxvm/devices/dma_bus.*` is the relocated board adapter. It retains
  sparse ports, page/spare latches, byte/word address expansion, paired grant
  routing, request-provider lifetime tokens and physical transaction ordering.
  XT constructs one actual controller; AT constructs two. Chip state is not
  mirrored in the adapter.
- The cycle provider is borrowed for one advance call. It receives a local
  address/channel/kind and, for M2M, a copied temporary byte. Failed memory
  preflight does not consume the device byte or commit chip address/count.
  External irreversible effects are not claimed to be rollbackable.
- Board master-clear still clears its page/spare latches, as before. Delegated
  cascade rotation remains at grant; ordinary rotation remains at execution.
  The formerly cleared secondary software-request-zero bit has no board-visible
  role: primary requests, not that software slot, determine cascade eligibility.
- The obsolete accelerated production advance helper is deleted. All real and
  test transfers now use the same service-phase implementation. The old chip
  implementation and headers are not retained as a parallel path.
- DMA construction now returns allocation/registration status. Partial port
  publication is rolled back before destroying chip contexts; a pre-existing
  registration error is preserved. Machine construction propagates that error.
  The transaction fixture now destroys its newly owned DMA instances.

## Test Disposition And Review

- Independent `dma_contract_smoke` uses public registers/signals and copied
  cycle effects: 144 channel/mode/type/timing/success-failure combinations,
  M2M read/write phases, temporary register, reset and cascade-without-transfer.
- The existing board test retains the 126 first-service matrix and page/lane,
  priority, demand/single/block, software request, TC, autoinit, EOP, wrap,
  cascade, M2M failure and stale-binding scenarios. Register assertions use
  real port reads; TC is read once after the operation because reads clear it.
  Mask behavior is tested by controlled DREQ inputs, not a private-state getter.
- The test-only shared fixture advances until observable transfer/release;
  it does not contain a second phase model. M2M half-cycle checks use explicit
  input clocks. FDC, Xebec, Model-40 and transaction callers are migrated.
- Model-40 reset checks now observe no pending work; independent chip tests
  cover reset register clearing/masking and cascade behavior. This intentionally
  replaces private representation checks rather than exposing a full-state ABI.
- Boot diagnostics use nondestructive copied DMA signals instead of private
  register dumps. Boot success criteria are unchanged; diagnostic observation
  does not consume architectural status or disturb the byte flip-flop.
- Negative dependency probes reject DMA importing App/private PIC headers or
  linking Common. Source/build boundary, manifests and all callers are checked.

## Verification And Delivery

Both standalone chip suites pass 13/13, including manifests and negative probes.
Full units passed 342/342 per width; final review removed a duplicate assertion
and moved a cascade TC read to follow its operation. Final reruns pass
342/342 per width (19.40s x64, 19.83s x86). Default integrations pass
20/20 per width (11.70s x64, 12.10s x86). All remaining boots pass once:
XT 21.57s/24.69s, 5170 38.16s/45.30s, Model 40 71.01s/82.43s (x64/x86).
The specialized static aggregate and all six shared manifests pass. One earlier unit aggregate exited -1 before completion;
no assertion diagnosis was reported by that invocation, so it is not counted
as accepted evidence. The subsequent complete run passed.

Documentation governance, 32 local document links and diff whitespace pass.
Commands are the existing `run-unit-tests`, `run-integration-tests` and
`verify-current-specialized-gates` targets; remaining profiles are configured
one at a time, their product/boot target built, and their sole boot row tested.
No successful boot is repeated for a multiple-round matrix. Both reusable
NXVM build trees are restored to the default profile.
Shared implementation P1 is 53b4be21d, pushed to origin/master. NXVM delivers
the receiving adapter, tests, documents and eight products in its separate P2.

## Actual-Change Acceptance

Coordinator-role review inspected pushed Shared 53b4be21d and NXVM 217125697
against the complete brief: chip/board ownership, scoped callbacks and copied
values, first-service phases, cascade priority, TC/read side effects, reset,
physical transaction failures, construction rollback and all consumers.
Test review checked the original scenario mapping and final corrected TC-read
ordering, not just green test counts. Public observations replace private
representation checks without a second state owner or full-state getter.

All required verification and eight receiver artifacts are complete. Shared
P1 and NXVM P2 have separate target scopes and are pushed to origin/master.
The temporary S7 standalone build trees are removed; the reusable NXVM trees
remain for the next batch. No outstanding item remains in this bounded S.
S7 is accepted and its active packet is removed. T539 remains open.

Production C/H changes total +1231/-1280 (net -49), including the new Shared
chip and removed App implementation. Test C/H changes total +526/-225 (net
+301), including the independent matrix and one reusable board fixture.
Counts use `git diff --numstat` plus new-file line counts against the baseline;
documentation, CMake, manifests and binaries are excluded. The fixture growth
replaces forbidden private-field coupling; no new generic device framework or
second DMA engine is introduced.

All eight deployed files have PE machine 8664/014C as named, embedded 0.5.0539
and no compiler-debug sections. Runtime Debug is retained. These are optimized
Release artifacts built from this delivery, not a different test-only route.
The four owner INIs and MyNES artifact inputs are unchanged.

| File | SHA-256 |
| --- | --- |
| nxvm_model40_0_5_0539_x64.exe | 93929C878F82786C8B8F1543B5E5EE2C83B4750E4C397F4AD178854C7B5A357D |
| nxvm_model40_0_5_0539_x86.exe | 366D4C29363D9EC5C25BA3C03607F3D6DA7A5ADE5B0B4BCC5AF06C8E1DE70A6F |
| nxvm_default_0_5_0539_x64.exe | ACB1758E7B5A7F8549BA029798D6DDAA63DA6E6C057D0B9024209938D30149BF |
| nxvm_default_0_5_0539_x86.exe | 432A23CD717DAC694D899FE930A42F1FA18FDC962AC14531FEC146B57FF63574 |
| nxvm_xt_0_5_0539_x64.exe | B98AEDDAF92D9EF868660F5AC25B4A7A3380CDA43FB0DDF936708EC4F98C5775 |
| nxvm_xt_0_5_0539_x86.exe | CEC455C8E51AD1E2052AC6B56C714BA08BFFB45CBAFBCD507C6EFB98FDEA7241 |
| nxvm_at_0_5_0539_x64.exe | 4D56019565F0DF1F0F1C52DF6B15B17D8F513893977D2723B596BFDB135395D6 |
| nxvm_at_0_5_0539_x86.exe | F81C17CF99CFA67B6C09EB7D4D151FFFE446B765E0B64AA2FE2C5BAD784E5A5F |
