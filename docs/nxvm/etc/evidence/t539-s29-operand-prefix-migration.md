# T539 S29 Operand/Prefix Test Ownership

## Original receiving map

Baseline is accepted S28 P2 `9a4d84c5b`. This S changes tests, their build
registration and boundary checks only. It does not change CPU production code,
public ABI, Shared, MyNES, firmware, INI or executable inputs.

The 538-line operand/address suite had five named groups. All 28 original
execution contexts remain. The 957-line S64 prefix suite had twelve named
groups; all its original program arrays, profile loops and assertions remain
in the receiving tests.

| Original group | Receiving owner and observation |
| --- | --- |
| Operand prefixes and effective addresses (8 contexts) | CPU-only: 16-bit address/operand overrides, SIB, segment default/override and register values. |
| 16-bit code and faults (5 contexts) | CPU-only: 16-bit defaults, both private segment-cache rejections, 32-bit address override and code-limit wrap. The two rejections also have board receivers for real descriptor loading and delivered double fault. |
| Stack forms (7 contexts) | CPU-only: PUSH/POP, PUSHA/POPA, PUSHF/POPF and ENTER/LEAVE, including mixed stack widths. |
| Port strings (2 contexts) | Board-only: actual port provider sees OUTSB writes and INSB reads; copied CPU result and memory are checked. |
| Memory strings (6 contexts) | CPU-only: MOVS, ES destination, limit fault rollback, DF, wrap and STOS/LODS. |
| S64 segment override matrix and last-wins | CPU-only: six segment prefixes and the double-prefix precedence case. |
| S64 operand/address attributes and LOCK | CPU-only: original read/write, width, address and LOCK legality/exception matrices across original profiles. |
| S64 LOCK group legality and writes | CPU-only: original group forms and side effects, including terminal #UD. |
| S64 repeated width prefixes and fixed segment/register forms | CPU-only: original repeated-prefix and no-memory-operand cases. |
| S64 REP MOVS, edge cases, REPNE MOVS and mixed repeat precedence | CPU-only: original count/width/segment/last-wins cases. |
| S64 real IRQ after prefix execution | Board-only: actual PIC request/acknowledge, ISR/IRR, frame IP and copied register result. |

The operand map is 26 CPU executions plus four board executions, with the two
segment-limit faults intentionally observed at both boundaries: 26 + 4 - 2 =
28 original contexts. The prefix suite has eleven CPU-owned named groups and
one board-owned IRQ group. Neither original suite remains a parallel private
CPU execution path.

## Design and review

Both CPU receivers link only `x86-cpu` and use the existing instruction
fixture; no board scheduler, port provider or PIC is imported. The operand
board receiver uses public machine create/freeze/reset, register patch,
memory read/write, run and copied CPU snapshot. Its two fault contexts use
real GDT data-descriptor limit and expand-down bits before guest `MOV DS`,
instead of mutating a private cached descriptor after bootstrap. This checks
the board's descriptor-load and exception-delivery path while the CPU-only
receiver retains the original direct cache assertions. The prefix board
receiver retains only the real PIC/IRQ case and uses public CPU observation.

T337 terminal #UD ownership moves with the prefix CPU receiver. T332 and T344
classify the two board receivers as public constructors. The CPU/PIC boundary
gate now includes both files and the negative fixture rejects four private
access mutations in each. T344 inventories 112 constructors, 119 total, 20
shared tails and 92 explicit shapes. The original direct private-consumer
inventory drops from 84 to 82; S30-S37 retain those pending owners.

## Verification

Both complete x64 and x86 builds pass. Final repository-only unit suites pass
389/389 on each width (38.09 and 37.94 seconds). All 66 specialized gates
pass, including T344's 388-row strict-compilation matrix. The extended
CPU/PIC boundary negative test passes 72 CPU-bus, five board-bus and 72
board-test controls. Six unchanged Shared corpus manifests verify. This
implementation record awaits the actual-commit review and does not itself
accept S29, S18-S40 as a whole, or T539.

The nine test/build paths add 1,514 lines and remove 1,311, net +203. The
four documentation paths are the packet, inventory, evidence and history.

## Coordinator Acceptance

Actual pushed P1 `86fe95201` has the stated 13-path scope and passes
`git show --check`; the post-push worktree is clean and `origin/master` names
the commit. The full receiving map, public-board/private-CPU separation,
complete dual-width units and specialized gates meet the S29 packet. S29 is
accepted. The 82 remaining direct private consumers and the CPU/board
lifetime and Shared relocation are not accepted by this review.
