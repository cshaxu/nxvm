# T343 S1: Four-Profile Cross-Closure Ledger

## Audit Order And Source Sweep

The final reconciliation follows the approved dependency order: shared
delivery, then form/profile classification, then cross-profile state
composition. It uses the T336 allocation record, T337 shared delivery,
T338 8086/80186 closure, T339 80286 closure, and T340--T342 80386DX closure
as evidence inputs, not as interchangeable test buckets.

The packet's metadata/dispatch/state query found the one primary and `0F`
dispatch, one profile form classifier, one prefix/`LOCK` classifier, one
page-walk owner, one task transition plan, and one final delivery path. The
apparent 16/32 branches occur at documented Intel layout or width boundaries:
operand/address width, 16/32 frames, TSS16/TSS32 images, and VM86 return.
There is no same-semantics parallel construction to merge or an unassigned
profile-specific decoder.

## Final Profile And State Ledger

| Family or state transition | 8086 | 80186 | 80286 | 80386DX | Cross-profile disposition |
| --- | --- | --- | --- | --- | --- |
| Primary real execution: ALU, FLAGS, conditions, data movement, string, stack, control, ordinary I/O | T338 S2/S3 exact real-mode form and rejection proof. | T338 S2/S3 exact real-mode form proof, including the 80186-only extensions below. | Retains applicable real forms; protected behavior is the next row. | Retains applicable real forms with T340 width/prefix proof. | Accepted. No later-profile execution is used as sole proof for an earlier profile. |
| 80186 extensions and profile gates | `60`--`62`, `68`--`6F`, `C0/C1`, `C8/C9` reject. | T338 S4 accepts defined forms. | Retains 80186 forms; adds protected architecture. | Retains forms; genuine 32-bit attributes are T340-owned. | Accepted Intel profile boundary. |
| Prefix, operand/address width, FS/GS, and `LOCK` | T328/T338 retain legal bus-prefix behavior and exact invalid-form rejection. | Same T328/T338 policy with 80186 forms. | T328/T339 retain pre-386 policy and protected consumers. | T340 S2 proves `66/67`, FS/GS, and the 80386 classifier. | Accepted. One `LOCK` classifier with profile/form evidence; width branches are explicit Intel differences. |
| Protected descriptors, selectors, tables, gates, frames, and return | Not applicable beyond real-mode rejection/form boundary. | Not applicable beyond real-mode rejection/form boundary. | T339 S2--S6 proves the 16-bit protected transition matrix. | T340/T341 consume distinct 32-bit table/frame/cache layouts without flattening them. | Accepted. 16/32 frame and descriptor layouts remain deliberately separate. |
| Shared fault, software, external, NMI, and return delivery | T337/T321/T326 retained form-specific real delivery. | Same owner with profile gates. | T339/T321/T326 prove protected gate/frame consumers. | T341 adds VM86/task/paging/debug consumers. | Accepted. `ExecFinal`/`ExecInt` remain the shared delivery owners; source form/state producers retain their own restart contract. |
| Paging, VM86, task/TSS, CR/DR/TR/LDT/table, and ordinary debug | Not applicable or exact profile rejection. | Not applicable or exact profile rejection. | T339 closes the bounded 16-bit protected task/table consumers. | T341 S2--S4 closes CR/DR/TR/table, VM86/task/paging, and DR6/DR7/vector-1 composition. | Accepted. State features begin only at their Intel profile boundary and use one owner graph. |
| CPU-side external coprocessor interface | WAIT/ESC profile boundary only. | Same CPU-side boundary. | Same CPU-side boundary. | Same `#NM`/`#MF` interface; no numerical unit implied. | Accepted external interface; x87 execution is not a CPU-profile closure claim. |
| VME/PVI, 486+ extensions, persistent TLB/test-register model, numerical x87, timing, devices, Windows | Outside profile program. | Outside profile program. | Outside profile program. | Outside profile program. | Explicit external Queue/TODO boundary with admission conditions; not Partial or Missing 80386 rows. |

## Cross-Profile Checks

- **Delivery before forms:** T337/T321/T326 own common producer, frame,
  restart, and PIC/NMI composition; T341 S4 adds ordinary debug without
  changing an earlier-profile route.
- **No cross-profile leakage:** T338's audit proves every 8086/80186 row from
  low-profile evidence; T339 preserves 80286 16-bit layouts; T340/T341 add
  only 80386DX width/system state. Static profile metadata and current-gate
  registrations remain the mechanical backstop.
- **True divergence retained:** 8086 `PUSH SP`, 16/32 stack and frame widths,
  TSS16/TSS32 layouts, FS/GS, VM86, paging, and debug controls are not
  normalized into false shared semantics.
- **No returned implementation:** The audit found no in-scope partial,
  missing, or unclassified form/state row. The only current residuals are the
  explicit external boundaries listed above.

## Result

The M5 four-profile CPU-completeness program is closed at its architectural
boundary: all four configured CPU profiles have one truthful form/state
disposition and cross-profile transition owner. This result is not Windows
readiness, device/timing fidelity, or x87 execution evidence. Those remain
the next independent Queue/TODO concerns.
