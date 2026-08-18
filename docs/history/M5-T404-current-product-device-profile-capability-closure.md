# M5 T404: Current-Product Device Profile And Functional-Capability Closure

## Task Record

T404 freezes and reconciles the public NXVM device/profile surface after the
DeskPro and shared KBC functional repairs. It is a coverage-bearing functional
program with a durable support ledger; it does not establish physical timing or
L3 readiness.
## S1 Acceptance

P1 `c4c7916d` freezes the public capability universe, owners, batch order and
completion predicate. Documentation governance passes. S1 is accepted as the
coverage baseline; T404 remains open for Batch A catalog/profile/variant/media
contract audit and any resulting bounded repair.

## S2 Acceptance

P1 `61a4ae98` reconciles the public YAML catalog with the session-start contract, updates the required developer artifact to `vm-0-5-0404`, and records the full Batch A disposition. Direct catalog smoke and the full current gate (285/285) pass; documentation governance passes. S2 is accepted; T404 remains open for Batch B controller/media/display/input functional reconciliation.

## S3 Acceptance

P1 `25cb4839` repairs the shared session media lifecycle: generic fixed HDD loading is private to startup, public HDD replacement is unavailable after publication, and public FDD insertion is running-state guarded and failure-atomic. The focused smoke and full current gate (286/286) pass; documentation governance passes. S3 is accepted; T404 remains open for the residual Batch B controller, display and input functional reconciliation.

## S4 Acceptance

P1 `612aee33` completes the S3 removable-media similar-issue sweep: Console removal no longer bypasses the VM session owner, running removal rejects, and stopped removal clears retained state. The extended lifecycle smoke and full current gate (286/286) pass; documentation governance passes. S4 is accepted; T404 remains open for controller, display and input Batch B reconciliation.

## S5 Acceptance

P1 `82b0da50` makes the VM-public host-input contract total: key and relative-mouse events map to ordered ingress, while unknown kinds reject before queueing. The focused ingress smoke and full current gate (286/286) pass; documentation governance passes. S5 is accepted; T404 remains open for residual controller/display Batch B reconciliation.

## S6 Acceptance And Task Closure

P1 `7db873f0` reconciles the complete frozen Batch B matrix. Current source and registered current-gate regressions prove every public controller, media, display and input route or retain it under a named physical/reference receiver. The latest full current gate is 286/286 and documentation governance passes. T404 therefore closes its functional product-capability scope only; the physical and L3 work remains explicitly transferred.
