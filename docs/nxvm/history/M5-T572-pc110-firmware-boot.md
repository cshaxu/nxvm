# M5 T572 — PC110 Real-Firmware POST Admission

T572 closes the bounded real-firmware admission receiver, not PC110 product
qualification. `app-mypc110` now supplies one App-owned PC110 plan to the
ordinary X86-NXVM Machine/Product route. The route embeds only the approved
external TFT BIOS/font inputs during build, validates their hashes, and has no
runtime ROM loader, BIOS interception, synthetic POST path or PC/AT facade.

The real BIOS executes through the normal executor. On both widths, the
diagnostic crosses the descriptor self-test and processor-only shutdown/reset,
then executes at least 8,000,000 guest instructions. The checkpoint identifies
that execution remains within unqualified real firmware; it is not evidence of
completed POST, Setup or DOS boot. No MyPC110 executable is published.

Two generic owner fixes were required: 80386-and-later `SGDT`/`SIDT` preserve
the complete six-byte base even through the default operand-size encoding, and
a valid no-VADP board retains a PIT deadline. The full input provenance, L2
wiring decisions, exact checkpoint and transferred guest boundary are recorded
in [T572 S1 evidence](../etc/evidence/t572-s1-real-firmware-post.md).

The archived [original proposal](M5-T572-pc110-firmware-boot-proposal.md)
retains the wider POST/Setup/DOS planning scope. The queued MyPC110 product and
guest-qualification receiver must next capture a finite trace of the first
remaining CPU-visible service, interrupt or device boundary, then add only its
source-backed owner route.
