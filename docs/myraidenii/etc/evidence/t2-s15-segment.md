# T2 S15 Native Segment Transfers And Inhibition

Baseline `9f1928ca4`. This consumes the complete segment-transfer/inhibition
subfamily identified by S14 at V06/V11, not complete V06/V11, all native slots,
PSW/mode/8080, general interrupt priority, timing, Machine or board/game proof.

## Primary Source And Actual Baseline

The pinned [NEC October 1986 manual](../research/references.md), SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`,
printed 6-4 lists both segment-MOV directions and POP segment among the forms
after which NMI/INT wait for the following instruction. Printed 12-7--12-10
defines both MOV directions and far pointer loads; printed 12-146--12-153
defines segment stack forms. Segment MOV loads and POP explicitly mention
single-step inhibition. PUSH and LDS/LES are not listed as post-inhibiting.
Printed 3-3 supplies the architectural PSW fixed-one image bits used by the
independent return-frame expectations, retaining S3's original distinction
between internal writable flags and the saved architectural image.

The actual native baseline `MOV ES, AX` completes without any segment shadow:
`V30:SEGMENT-RESULT:1:0:0:0`. The same native 186-capability predicate prevents
DS/ES MOV and POP from publishing inhibition, and source-direction MOV never
publishes it. This is a ROM-free CPU reproduction, not a reported game fault.

The source summary specifies external NMI/INT for source-direction MOV, while
the individual store descriptions do not expressly qualify BRK inhibition.
This batch deliberately reuses the existing segment-shadow mechanism, including
its step suppression, as an explicit L2 store-BRK model. Tests prove that
deterministic model, not silicon-exact BRK behavior. It introduces neither a
second shadow nor another entry route. General BRK priority/entry and PSW/mode
facts remain V10/V11/V13; no full-entry qualification follows from this batch.
No third-party implementation, table, firmware, ROM or manual payload is imported.

## Production Ownership And Sweep

Native V30 now participates in the existing post-success segment-load/POP
condition, alongside the genuinely distinct old Intel and SS conditions.
Source-direction MOV publishes the same shadow only after its operand write
succeeds. The identical two arms of MOV-load decoding are collapsed to their
one existing decode call. No Intel predicate, operand/segment loader, stack
reader, physical lane, failure rollback or interrupt arbitration is replaced.
There is no new production helper, state, public function/type/layout or path.

The same-class sweep uses:

```text
rg -n '_s_load_sreg\(|_ksa_load_sreg\(|_ksa_load_real_sreg\(|_e_pop_sreg\(|interrupt_shadow\s*=' src/x86-core/cpu/cpu_instructions.c
rg -n 'LSS_|LFS_|LGS_|_e_load_far\(' src/x86-core/cpu/cpu_instructions.c
```

All hits have distinct dispositions:

- MOV 8C, MOV 8E and the shared POP-segment owner are repaired together.
  Native legal ES/SS/DS loads, all four segment sources, register/memory forms
  and PUSH/POP use their original tables and scalar load/storage routes.
- `_e_load_far` serves LDS/LES and Intel-only LSS/LFS/LGS. It is not a MOV/POP
  instruction-policy owner. Native LDS/LES negative witnesses reject adding
  inhibition at this generic helper; later Intel forms remain unchanged.
- `_s_load_sreg`/`_ksa_load_sreg` also serve task, protected/control, interrupt
  and paused-debug operations. Those genuine segment-cache writes do not
  acquire an implicit native MOV/POP policy. Full control/mode/entry receivers
  remain in the original ledger, not certified through incidental coverage.
- STI and protected return's existing INT-only shadow remain distinct.
  ExecInt/debug completion consume the single existing shadow; ordinary
  instruction entry clears it before the next instruction, which can republish
  it for a legitimate consecutive transfer. Reset/RSM/SMM ownership is unchanged.
- FS/GS and 32-bit/protected forms retain Intel-only admission; S12's reserved
  form tests and S13's code-fetch/preview boundary tests remain enabled rather
  than reimplemented in this segment unit. Native PSW/entry/mode and full
  prefix/time contracts remain explicit incomplete receivers.

## Independent Owned Proof

The new CPU-owned `cpu_v30_segment_smoke.c` reuses S14's real-storage fixture.
It checks 1,381 matrix cases plus the initial baseline witness: every native
segment/GP combination, direct and BP addressing, five segment selections,
aligned/odd/FFFF and stack-source overlaps, stack zero/one/tails, far-pointer
destination aliases, and selector zero/FFFF. Explicit operations and encoded
input bytes select mathematical expectations; no production decoder/table is
used as the semantic oracle.

Each case compares the full CPU, including untouched upper GPR halves and
flags. Ordered real provider reads/writes must have the expected address,
native width, direction, accepted prefix and BEGIN/COMMIT/CANCEL phase. One
expected memory copy accounts for accepted writes and is compared against all
one MiB of actual RAM. Every applicable data-provider failure position checks
complete CPU rollback, preserved accepted writes, no invented guest exception
and no published shadow. Preview consumes no data and changes no live CPU or
shadow. Fourteen local test helpers own these actual verification responsibilities;
there is no production framework or new shared test infrastructure.

Every matrix case additionally executes pending INT, NMI and sampled TF through
the actual CPU. MOV/POP defer until the next NOP; PUSH/LDS/LES do not acquire a
shadow. The exact three-word return frame, actual vector, acknowledge count,
updated SS/SP and actual IRET return are checked. Chained MOV ES/MOV DS/POP SS
continue inhibition until their following NOP. IF-clear NMI still arrives;
pending INT remains blocked until actual STI and its following instruction.
Reset clears the pending NMI/shadow through the original reset route.

While building these new witnesses, two independent-precondition mistakes
were corrected before complete suites: architectural PSW stack images include
fixed-one bits which internal flags omit; and each stack-word start wraps with
sixteen-bit SP, so a frame crossing zero is not six linearly consecutive bytes.
One frame checker now supplies the exact expected words to both ordinary and
chain witnesses, retaining S8's separate physically contiguous data-word L2
tail. No production flags, stack model, assertion or timing budget was weakened.

## Actual Delta And Verification

Production +7/-10/net -3 and test/registration +435/-0/net +435, combined
+442/-10/net +432, exclude manifests/documents/artifacts. This meets the
initial production net -6--+6/test net +300--450 estimate. The positive test
cost is independently owned finite form, event, alias and failure proof.
Existing production warning exemptions remain unchanged, not promoted to a
waiver-free whole-tree claim.

Both original Release Ninja/ccache graphs build all targets. First complete
repo-only suites pass x64 667/667 in 304.83 seconds and x86 667/667 in 109.00
seconds, including 178 X86 Core units and receiving App units. Architectures
run sequentially; complete suites need no retry or budget/assertion change.
The new segment unit passes in 0.73/2.01 seconds with its original thirty-second
budget. All registered manifest, Types, dependency, ownership and negative
gates pass. Separate corpus/manifest and documentation governance checks pass.

Frozen production SHA-256:
`CC8FD2C29CFD14306340AC5A10FE71FD3A0E61DD891C7315947148750F9BD8AB`.
New segment unit:
`5056448A2ACAE6FD3E8E725447EAEA3ED29E842D9AF2C62ABFEFD996FBB2ACB8`.
Shared `ef0a6f30e` and NXVM `82a3aee05` are pushed. Both graphs link/deploy all
four PC profiles at retained revision 0563, reusing CPU objects and restoring
default after each width. All eight PE/product/version/lowercase-INI checks
pass; the [artifact inventory](t2-s15-pc-artifacts.json) records exact sizes,
architectures and hashes. Assets retain exactly ten EXEs; the MyNES pair's
hashes remain unchanged. All INIs, media/snapshots/ROM inputs and other Shared
packages remain untouched.

Governance, links, whitespace and actual-change review accompany the delivery.
The coordinator separately reconciled the sixteen-field packet, original
goal, complete admitted segment batch, source/model distinctions, remaining
receivers, actual source/test/manifests and all artifact identities. Shared
`ef0a6f30e`, NXVM `82a3aee05` and evidence `134584075` are accepted. No
original convergence-domain member is silently retired. S15 closes through
the following purely governance P, without another code/artifact change.
Logs under `build/myraidenii/t2-s15-*` are retained for final T2
qualification; all batch build/test/deployment handles have exited.

No external-ROM integration, manual desktop/RDP, physical audio, real board
or game execution is claimed. No MyRaidenII EXE exists. The lossless-colour
[High TODO](../../states/TODO.md) still gates final release. Native store-BRK
behavior is explicitly L2; complete native slot/semantics, mode/8080, general
entry/priority, source-graded timing, Machine and board/game remain open.
This delivery does not close T2 or the original full MyRaidenII goal.
