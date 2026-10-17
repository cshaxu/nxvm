# T2 S12 Native ModRM Admission

Baseline `f55f5caec`. This batch consumes the native primary ModRM admission
subfamily of V03, not every native instruction's semantics or full V03.

## Source, Reproduction And Sole Owner

The pinned [NEC October 1986 manual](../research/references.md), SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`,
section 12 and Appendix A, provides the native encoding fields. The existing
pure scanner already checked memory-only forms, group extensions and minimum
capabilities. Actual primary dispatch checked metadata using a placeholder
ModRM of zero; some handlers read an operand before diagnosing its real form.
The baseline owned test fails with `V30:FORM-RUNTIME:c0:30:0:1:0`: reserved
Group-2 /6 performs one data read. Existing unsupported-form diagnostics are
the contract being fixed, not an assertion about undocumented silicon #UD.

The runtime now obtains actual native ModRM through the existing code-fetch
owner before invoking the handler. It reuses the same format predicate and
metadata/minimum-capability owner as pure scan. The two private format helpers
are renamed `_kdf_has_modrm` and `_kdf_modrm_form_valid` to reflect their common
decode role; no alias, new table, decoder, state, public API or layout exists.
The original opcode-level rejection remains before the added check. Rejected
carry-repeat targets retain their earlier admission boundary. NEC 0F retains
its existing own actual-ModRM admission; Intel never enters the new branch.

The existing `_s_read_cs` owns prefetch/provider reads and recorded instruction
bytes. Its offset is wrapped to sixteen bits at this call, including FFFF-start
instructions. Re-reading bytes outside the prefetch window remains possible,
as in the existing NEC path; this batch does not certify physical fetch counts
or electrical timing, and does not add another instruction cache to hide them.

## Owned Proof And Similar-Issue Sweep

The ROM-free CPU unit extends the existing real-storage instruction fixture
with local provider counters and an explicit code-read refusal. Its encoding
expectations are independent test data, not calls back into the production
format predicate.

- 24,320 scan cases cover nineteen constrained primary opcodes, every ModRM
  and no/ES/CS/SS/DS prefix. Each rejected form also exercises observational
  preview and actual execution: diagnostic stop, unchanged complete CPU image,
  no delivered guest exception and no data read/write or port access.
- Nineteen actual valid instruction witnesses retain LEA, MOV immediate,
  POP, LES/LDS, byte/word INC/DEC/NOT/shift, segment moves, BOUND, indirect
  near/far CALL/JMP and PUSH. Truncated input lengths are rejected by pure scan
  before each full witness is accepted. This does not qualify all legal
  instruction results, flags, stack images or segment semantics.
- Four invalid forms at IP=FFFF prove actual ModRM wrapping and rejection
  without operand effects. Four code-provider refusals at wrapped address zero
  preserve IP and report diagnostic failure without invented guest faults.
  A legal C7 immediate-store crossing FFFF proves continuation and one write.

The sweep reconciles Group-2 /6, Group-3 /1, Group-4/5, POP/MOV-immediate,
segment-register encoding and memory-only forms with the existing metadata
and shape owners. Unconstrained arithmetic ModRM still follows its existing
handlers; no additional rule table is necessary. Intel control/system quirks
remain solely in the original predicate. Native NEC format admission, old
Intel instruction/fault-order tests and prior V30 tests remain unchanged.
FPU, LOCK/long malformed streams, mode/8080, timing, Machine and real board
requirements keep their original ledger receivers; no native fault behavior
or complete V30 eligibility is inferred from these synthetic tests.

The sweep also identifies a retained fetch-boundary obligation: NEC 0F reads
its actual ModRM using `eip + 1` without normalizing that starting offset.
If its suffix is at FFFF, the call can address CS+10000 instead of CS+0000;
the native logical translator deliberately does not mask data offsets. This
is source-level evidence, not an executed NEC boundary reproduction. V04/V14
must qualify the complete instruction-fetch start/span mechanism, including
this NEC case and multi-byte reads, before native fetch is called complete.
S12's new primary one-byte lookahead wraps explicitly; its tests do not prove
all existing fetch callers. No per-handler workaround is added to the NEC route.

## Delivery And Verification

Shared `9d069e6e4` and NXVM artifacts `663ce268d` are pushed.
Both original Release Ninja/ccache graphs build all targets. First complete
repo-only unit suites pass x64 665/665 in 284.78 seconds and x86 665/665 in
102.77 seconds, including 176 X86 Core units and receiving App units. The new
unit passes in 0.14/0.20 seconds. Architectures run sequentially without retry,
budget change or assertion removal. The direct repaired unit also emits
`V30:FORM-ADMISSION:OK` after the added truncation assertions.

Source/test manifests and registered Types, inward dependency, ownership and
negative gates pass. A separate `-Wall -Wextra -Wpedantic` syntax review
records existing unused warnings and the trace macro's empty-else warning,
including its newly added use. The build retains exactly the old warning
exceptions; no new exemption is introduced and no waiver-free claim is made.
Documentation governance, relative links, whitespace and actual-change review
are required for final coordinator acceptance.

Both original graphs link/deploy four PC profiles per architecture, reusing
objects and restoring default. The [artifact inventory](t2-s12-pc-artifacts.json)
records eight PE widths, sizes and SHA-256 values at retained revision 0563;
product-name, version and lowercase-INI tokens are checked in every EXE.
MyNES pair, all INIs/media/snapshots/ROMs and other Shared packages remain
unchanged. No external-ROM integration, desktop/RDP, physical audio, actual
board/game execution or MyRaidenII EXE is claimed.

Actual change excluding manifests/docs/artifacts: production +17/-4/net +13;
tests +220/-0/net +220 (219-line unit and one registration); combined
+237/-4/net +233. This is smaller than the test estimate because the existing
fixture supports the complete finite encoding matrix without a new framework.
Two private helpers are renamed/reused, not added; no public API/type/layout,
persistent state, handler replacement, second executor/cache/clock/queue exists.
Source SHA-256:
`B43741E4F755AD34D45E9AAB1AEC018977041D80CE79CA727B07D6340AB52815`.
Test SHA-256:
`EE205985BA4BFF6F27A486C184D887310EAEAA32F882AA0764473644AA818B7E`.
Ignored logs under `build/myraidenii/t2-s12-*` retain baseline, repaired build,
full-unit, warning and artifact proof for final T2 acceptance. No manual/table,
external implementation, trace or protected input is imported.
