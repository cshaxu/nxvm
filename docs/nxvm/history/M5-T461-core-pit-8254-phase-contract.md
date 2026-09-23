# M5 T461: Core PIT 8254 Phase Contract

T461 closes the selected 8254 chip contract through one Core PIT owner.

- S1 accepted the manual-first P1--P18 ledger and code-gap audit.
- S2 accepted `a1ea0fa1`: P1--P14 CR/CE load, latch/status, modes 0--5,
  GATE/retrigger/rewrite and mode-3 CE rules have focused proof.
- S3 records the sole channel-0 OUT to PIC IRQ0 route, `DMA -> PIT -> PIC`
  scheduler order and reset/finalize release lifecycle.

P15 is closed at the selected logical board boundary. P16--P18 remain explicit
L2 boundaries for reset phase, board clock conversion and physical topology.
The full current-gate passes 294/294. The stripped Release artifact is
`build/output/nxvm_0_5_0461.exe`, SHA-256
`D875AB15C1E63A3FFBBC1A064315AE9516773A65F70FC4783957FE21DE45829A`.
