# M5 T540 S59 Callback and Firmware Binding Audit

## Decision and exact diff

No production or test code change is needed. The source audit found one
install point for the **14** Core-facing board providers and `board_owner`
in `machine_board.c` during board creation. They cover deadline; refresh
request/completion; DMA ticks/request/advance; PIT ticks and PIT/PIC advance;
PIC pending/acknowledge; shutdown reset; media, RTC and peripheral advance.
The neutral scheduler and CPU use only those bounded calls. The shutdown
callback retains its `const core_machine *` signature because the existing
scheduler test substitutes an independent probe as `board_owner`.

Board creation failure calls the single Core destroy path. Runtime callbacks
are synchronous on the machine worker; no board callback remains queued on a
separate host thread. During destruction, the board finalizer destroys its
chips and releases the attachment. The subsequent CPU, port and memory
finalizers only release allocations/provider entries; they do not invoke
the board callbacks. Clearing 14 provider slots immediately before freeing
the whole Core would not change reachability or safety, so no redundant
revoke mechanism was added.

Firmware has one binding path. `core_machine_bind_firmware_provider()` rejects
a second provider, invokes configuration within the Core firmware operation
guard, then lets the board choose the F0000h reset alias only if the actual
reset prefetch window is covered by ROM. On failure it rolls immutable ROM
mappings back to the pre-bind boundary and clears provider/context/operation
state. At normal teardown, NXVM destroys Core before its profile plan, so
the provider/context remain alive through the last invocation. No alternate
ROM table, mailbox or profile-specific Core branch was found.

The S59 packet's original wording asked for callback slots to be explicitly
"revoked before board release". That would be a redundant write to an object
being destroyed, not a real lifetime boundary. The actual invariant is that
no callback can be invoked after board release; the synchronous destroy path
and finalizer inspection establish it. This audit corrects the packet wording
without weakening the lifecycle requirement.

## Verification and artifact decision

- Focused firmware, ROM route/alias and scheduler tests: **8/8** on x64 and
  **8/8** on x86.
- Complete repository-only units: x64 **469/469**, x86 **469/469**.
- Both-width `verify-current-specialized-gates`: pass.
- S59 changes only NXVM documentation. The S58 [eight verified boot
  checkpoints](t540-s58-absent-memory-owner.md) and optimized 0540 artifact
  hashes remain the exact executable baseline; no EXE rebuild is required.

S59 closes the callback/firmware binding audit. S60 receives the neutral
private header; T540 remains open.
