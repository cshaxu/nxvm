# Long-Term Review Ledger

Only unplanned debt belongs here; [Queue](QUEUE.md) owns candidate receivers.
The public-component entries were transferred to the open
[T564 S6--S13 plan](../proposals/m5-softpc-rgb-frame-import.md) by owner
direction; transfer is not a repair or a claim of implementation.

- [ ] **MyPC110 peripheral extensions and fidelity (`TODO(Medium)`).** The
  current bounded device qualification retains external PC Card I/O-card
  transport, UART host serial/IR/modem connection, dynamic AC/battery/lid
  modeling, complete ES488/OPL2 synthesis, and physical panel fidelity as
  distinct future receivers. Keep the current single guest clock and actual
  component ownership; do not infer them from a memory card, UART loopback or
  fixed AC status. Admit a focused receiver when a concrete guest or external
  endpoint and verification corpus exist; until then software using those
  absent features can fail or produce approximate output. See
  [S23 coverage](../etc/evidence/t585-s23-device-coverage.md).
- [ ] **MyPC110 additional RAM configurations (`TODO(Medium)`).** The present
  PC110 plan implements 20 MiB and rejects other explicit sizes. Audit possible
  additional RAM sizes and their chipset, CMOS and open-bus mapping before
  implementing them; do not silently accept a size that is not constructed.
  Do not treat a lower RAM setting as the T582 performance solution. See
  [S39 evidence](../etc/evidence/t582-s39-execution-convergence.md).
- [ ] **X86 Core masked memory-span routing (`TODO(High)`).** A multi-byte
  access crossing an address bit cleared by the active mask can reach the wrong
  backing addresses. A provider that accepts the query but returns UNSUPPORTED
  on the operation also exposes an incorrect fallback. Design and test one
  consistent route covering RAM, provider decline, aliases, partial writes,
  bounds, parity and observers; the S39 partial experiment was not retained.
  See [S39 evidence](../etc/evidence/t582-s39-execution-convergence.md).
- [ ] **MyPC110 usable startup and guest execution performance (`TODO(High)`).**
  T582 delivered bounded optimizations, but the latest private-media runs still
  needed minutes to reach the sampled interface. Establish a repeatable
  representative improvement and desktop/RDP usability before considering
  this goal met; see [S39 evidence](../etc/evidence/t582-s39-execution-convergence.md).
- [ ] **Model40 x86 external-throughput qualification (`TODO(Medium)`).** The
  fixed 180-second Turbo DOS-installation terminal is not presently a
  differentiating CPU-semantic result: the isolated accepted S20 executable
  exceeds the same budget on the current host while current source reaches at
  least as much guest progress. Preserve the unchanged ROM/CMOS/media and
  terminal; admit a separate IBM PC/runtime throughput investigation only with
  a reproducible host baseline. Do not solve it by extending the timeout,
  changing the profile, or inventing guest time.
- [ ] **DevBox exploration (`TODO(Low)`).** Requires an explicit product goal,
  trust boundary and corpus; not an automatic extension of these machine builds.
- [ ] **Browser storage (`TODO(Low)`).** Native file-backed storage already has
  its Lib owner. OPFS/IndexedDB/HTTP-backed pending I/O is a separate host
  capability, admitted only with a future browser consumer and no second
  guest-time or storage path.
- [ ] **Linux runtime verification (`TODO(Medium)`).** Portability source is
  not runtime support proof. Admit POSIX runtime validation when an approved
  environment exists; no WSL installation is implied.
