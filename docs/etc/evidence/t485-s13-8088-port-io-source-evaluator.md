# T485 S13 8088 Port-I/O Source Evaluator

`M5:T485:S13:8088-PORT-IO-SOURCE-EVALUATOR:ACCEPTED`

The existing `core_machine_string_io_source_instruction_cost()` remains the
sole decoder-derived IN/OUT receiver.  For 8088 only, it consumes an immediate
private plan from the decoded opcode low bit and applies the rendered Intel
Table-2-21 equation `base + 4 * word_transfers`.  `E4/EC/E6/EE` remain
`10/8/10/8`; `E5/ED/E7/EF` become `14/12/14/12`.  No port, device, wait-state,
external-cycle, prefetch or physical-time state was added.

The focused retirement smoke proves all eight exact results and the existing
string-I/O timing origin.  The fully relinked 299-test current gate passes:
`299/299`, `261.97 sec` test time (`264.30 sec` wall time).  A current-gates
build static checker separately encountered an ACL denial opening `.ninja_log`;
this is recorded as a build-environment issue, not a source or CTest failure.

The patch is one existing-owner branch and eight focused assertions; it adds no
public ABI, decoder, compatibility route or mutable state.
