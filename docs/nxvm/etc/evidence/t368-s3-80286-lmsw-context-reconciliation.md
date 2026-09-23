# T368 S3: 80286 LMSW Context Reconciliation

## Authority and bounded question

Intel's *80286 and 80287 Programmer's Reference Manual*, order 210498-005
(1987), Appendix B remains the timing authority.  It gives `LMSW r/m16` a
successful-retirement value of `3` for register and `6` for memory in real
address mode or protected mode at CPL0.  The relevant source is retained
read-only at [Bitsavers](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf);
no manual content is imported.

S2's `#GP(0)` did not demonstrate a privileged-instruction implementation
defect.  This S isolates the state setup that preceded it, preserves the
existing `_s_load_cr0_msw` CPL check, and admits no timing for a faulting form.

## Reconciliation

`timing_80286_lmsw()` first exercises real-mode LMSW.  Its direct memory row
sets CR0.PE, then reuses the same machine for the protected-mode bootstrap.
`timing_80286_boot_protected()` previously reloaded segments and EIP but did
not clear CR0.  Consequently the intended real-address bootstrap began with
PE still set: its protected-only setup attempted to execute under stale
segment-cache state and faulted before the later CPL0 LMSW probe was reached.

The local bootstrap now explicitly clears CR0 before it asks the shared fixture
to load real-mode segments.  It then loads the GDT, enters PE through LMSW,
loads protected data/stack selectors and far-jumps to the CPL0 code selector.
The later register and indexed-memory LMSW rows demonstrably retire with the
existing `3` and `6` classifier results.  No production executor path changed:
the original `_s_load_cr0_msw` privilege check was correct for the stale state.

## Similar-issue sweep and verification

- Searched `tests/machine/core_machine_80286_instruction_timing_ledger_smoke.c`
  for `timing_80286_boot_protected`, and `tests/support/core_machine_cpu_fixture.h`
  for `test_core_machine_fixture_reset_real_mode`.  The shared helper promises
  segment/EIP reset only; it must not silently clear control registers for its
  other callers.  The single bootstrap that requires a real-address CR0 state
  now establishes it locally.
- Rebuilt and ran `core-machine-80286-instruction-timing-ledger-smoke`; it
  reports `M5:T357:S6:80286-INSTRUCTION-TIMING-LEDGER:OK`.  Its expected
  `#UD`/`#GP` diagnostics cover non-retiring negative cases.
- Rebuilt and ran `core-machine-msw-s63-smoke`
  (`M5:T316:S63:MSW:OK`) and `core-machine-80286-protected-mode-smoke`
  (`M5:T257:S6:80286-PROTECTED-MODE:OK`); their expected diagnostics preserve
  the privilege and invalid-form boundaries.

## Disposition

This resolves the S2 fixture-state transfer and retains only the existing
Appendix-B `3/6` successful LMSW publication.  It does not close the remaining
successful prefix/default classes, the complete 80286 retirement ledger,
PC/AT bus/device timing, physical time, or IBM 5170 L3.
