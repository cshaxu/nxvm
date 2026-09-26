# Long-Term Review Ledger

Only unplanned debt belongs here; [Queue](QUEUE.md) owns admitted candidate
receivers. The [former debt disposition](../etc/history/retired-machine-plans/README.md)
preserves every old entry and its retirement/transfer, not an assertion of repair.

## CPU, Time, And Debugging Debt

- [ ] **Internal descriptor vocabulary (`TODO(Low)`).** Preserve data layout
  and debugger behavior; admit only a caller-inventoried source cleanup, not a
  broad CPU rewrite.
- [ ] **Debug control-register mutation (`TODO(Medium)`).** Debug writes can
  create control/paging states unlike guest MOV CR. Admit a Core-owned bounded
  mutation contract with explicit raw-override policy and unchanged Debug UX.
- [ ] **Broader x87 execution (`TODO(Low)`).** Existing ESC/WAIT/interface
  behavior is not complete 287/387 numeric execution. Admit only a separately
  selected coprocessor, primary function/timing ledger and owned corpus;
  preserve existing no-FPU and exception behavior.
- [ ] **VME/PVI extensions (`TODO(Medium)`).** Ordinary 386 VM86 is retained,
  but later virtual-interrupt extensions need exact selected-CPU evidence.
  Admit only if the frozen CPU variant actually requires them.
- [ ] **CPU fault reporting (`TODO(Medium)`).** Re-admit from a reproducible
  missing outcome at the existing Common/Debug boundary; keep first-fault
  evidence and one execution path.
- [ ] **Assembler checkop (`TODO(Low)`).** Admit an owned diagnostic regression
  when a real defect requires it; no per-instruction reassemble comparison.

## Hardware And Compatibility Debt

- [ ] **Unselected peripheral breadth (`TODO(Low)`).** ELCR/PCI/APIC, advanced
  AUX, light-pen/composite effects, serial/parallel/game ports and host speaker
  breadth require a named board consumer and source contract. If selected by
  an XT/AT/DeskPro/default/PC110 BOM they transfer to controller qualification; otherwise
  they are not implicit product features.
## Timing-Fidelity Debt

- [ ] **Beyond digital timing (`TODO(Medium)`).** Analog/pin/physical-media
  behavior is not proven by register order or a proportional model. Admit a
  bounded source-backed receiver only when a selected product needs it; never
  hide required digital behavior here.

## Architecture And Portability Debt

- [ ] **Shared Console failure rollback (`TODO(Medium)`).** T538's exploratory
  native fixture observed a 120x60 cooked history buffer with a 40x13 viewport
  restored as 120x13 after injected raw-reader startup failure, before any frame
  write. Risk: offscreen monitor history loss on that failure path. Admit a
  separate Shared buffer-switch/rollback review with before/after native proof;
  the approved raw-frame backing-capacity repair does not claim this recovery.
  [Evidence](../etc/evidence/t538-boot-pairs.md).

- [ ] **Browser storage (`TODO(Low)`).** Native file-backed storage already has
  its Lib owner. OPFS/IndexedDB/HTTP-backed pending I/O is a separate host
  capability, admitted only with a future browser consumer and no second
  guest-time or storage path.
- [ ] **Direct-production strict compilation (`TODO(Medium)`).** Reconcile
  legacy mixed-target warning exemptions against current source ownership when
  that domain changes. Require target-local diagnostics and regressions, not
  a blanket global suppression or an assumed inherited strict build.
- [ ] **Linux runtime verification (`TODO(Medium)`).** Portability source is
  not runtime support proof. Admit POSIX runtime validation when an approved
  environment exists; no WSL installation is implied.
- [ ] **DevBox exploration (`TODO(Low)`).** Requires an explicit product goal,
  trust boundary and corpus; not an automatic extension of these machine builds.
