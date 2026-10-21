# T7 S43 Sound Unsupported-Surface Inventory

## Scope

This is a static reconciliation of the sole local Sound Z80 executor. It does
not read an owner ROM, retain guest bytes, addresses, instruction traces or
media data, and changes no production, test, build or artifact path.

The selected-set result after S42 is an existing Board-runtime
`LIB_STATUS_UNSUPPORTED` status. `board_advance()` can receive that status from
the existing Sound advance call. Within Sound, the status can arise from an
unimplemented instruction family or an unmapped device access. The runtime
result does not itself select either cause.

## Complete Static Disposition

| Executor surface | Current disposition | Can produce the observed status | Evidence grade and receiving work |
| --- | --- | --- | --- |
| Base Z80 decoder | Register transfers, ordinary loads/stores, arithmetic, stack, branch, call/return, restart, I/O and selected flag/rotate forms are present. The remaining default rejects unimplemented documented singleton forms, including decimal adjust, reverse carry rotate and carry complement. | Yes: the sole base default returns the existing unsupported status. | Zilog UM008011-0816 supplies instruction semantics as L3; a later finite base-family batch must implement and test the complete admitted group, not one observed checkpoint. |
| CB decoder | Rotate/shift, bit test, reset and set classes are covered through one shared transform helper. SLL is recorded as L2. | No for an opcode-class gap: all four CB classes dispatch before the helper. | Retain S42 regression; no new CB repair is selected. |
| ED decoder | Interrupt mode, return-from-interrupt, one 16-bit add-with-carry pair transfer and one block-copy form are present. The remaining documented extended families (other 16-bit arithmetic/load pairs, accumulator/refresh transfer, decimal rotate, port-register and remaining block transfer/compare/I/O forms) reach its default rejection. Undefined extended encodings remain deliberately unsupported. | Yes: its default returns the existing unsupported status. | Zilog UM008011-0816 is L3 for documented forms. A later ED batch must distinguish CPU forms from unmapped port behavior. |
| DD/FD decoder | Documented implemented index-pair, index-high/low, indexed-memory, indexed ALU, stack and prefix-ignored base paths are routed through the single indexed executor. Repeated index prefixes are not independently modelled. | Indirectly: its fallback delegates to the base/ED executor, whose status can propagate. | Existing S41/S42 tests cover finite delivered forms. Any repeated-prefix rule needs an explicit Z80 authority and its own bounded batch. |
| DD/FD-CB decoder | All rotate/shift, bit, reset and set classes use the single indexed executor and existing memory route. | No opcode-class gap in the four decoded classes. | S42 covers transform write-back/register-copy; no new repair is selected. |
| Sound memory and I/O map | The sole read/write map deliberately rejects unknown memory or I/O routes. | Yes: an unowned device route returns the same existing unsupported status through the decoder. | Board/Sound map ownership is product-local; no map widening is justified by the terminal alone. |
| Interrupt admission | The current executor uses unsupported internally as its existing no-interrupt sentinel, then continues ordinary execution. Nonzero interrupt modes are not a terminal source in that path, but the sentinel conflation is a separate correctness debt. | No for this terminal path as currently wired. | Do not fold it into decoder/map work; it needs an independently admitted interrupt-contract repair. |
| Clock and PCM advance | Overflow/queue conditions return their own non-unsupported statuses. | No. | Retain existing Sound timing/PCM tests. |

## Next Receiver

The runtime terminal has two remaining finite classes with the same status:
an instruction-decode gap or an unowned Sound map access. Selecting either
implementation solely from that shared status would be a guessed repair.

The next S must therefore add only a resettable, category-only local
observation at the existing Sound error return: decoder-family versus
memory-map versus I/O-map. It must not retain the operand, program counter,
address, byte, instruction sequence or media data. Board may copy that existing
Sound observation for the current Probe; no second executor, bus route, queue,
thread or Shared contract is justified.

This inventory does not claim title, gameplay, audio fidelity, hardware
accuracy or release qualification.
