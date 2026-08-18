# T399 S4 B4 Compaq HDC Reference Disposition

PCjs DeskPro configuration is exact at the machine-selection level but binds
`/machines/pcx86/ibm/hdc/47mb/unformatted-at5.xml`: an IBM Type-5 47 MB AT HDC.
It is not the selected Compaq WD 40 MB controller. Its HDC behavior is therefore
generic-AT evidence and is rejected for B4 under the T399 proposal.

Project-owned regressions nevertheless retain the existing logical boundary:
`core-machine-compaq-hdc-s5-smoke` proves command, data-request, IRQ14
assert/acknowledge, error and reset; the machine integration smoke proves its
port composition; and the Model-40 smoke proves synthetic BYOB/session binding,
normal read, IRQ lifecycle and reset. All passed on 2026-08-17. They do not
supply an external DeskPro-specific reference, physical media, ECC, service
cadence, DRQ phase or board-time proof.

B4 is rejected for reference-derived timing acceptance. The existing Compaq
40 MB physical fixed-disk media semantics and DeskPro physical-observable
device-timing TODO entries remain its sole receivers. No production code or
Core/VM interface changed.