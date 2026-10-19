# T3 S16: Low-Control Data Decode And COP Fixture Ownership

## Frozen Receiver And Evidence

Baseline04a378964. This consumes B02/B03's400h--7ffh operational decode
receiver left explicit by S15, not the entire B02/startup or B07/B08 algorithm
universe. The selected Raiden II map defines ordinary RAM through3ffh and
again from800h, with control devices between. It does not define a generic
writable backing for holes in that control window.

Fresh read-only research checks the original selected-board map in MAME
c1425a1af00b88aa07477b1b23a30ed5e5f856ff, raiden2.cpp SHA-256
FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A.
The shared inherited map and selected overrides were both inspected. This is
an L2 map/owner fact, not copied source/dispatch data, a schematic, silicon
timing or a measured physical open-bus value. No fetched payload is retained.
The first web observation failed to fetch; the subsequent read-only response
matched the frozen hash before the fact was used.

## One Board Decode, No Second Bus

Board adds one existing Core FALLBACK route for400h--7ffh. Its query accepts
only a byte; query is observational and the Core registration bounds that
callback. Its read/write callbacks return existing INVALID_STATE. This is
explicit unimplemented software capability, not an assertion that hardware
raises a CPU exception on an electrically unconnected address.

Installed replacement/standard COP, input, sound and bank handlers win before
fallback, regardless of installation order. The fallback introduces no stored
bytes, device model, alternate clock, worker, public header or new API.
The one registered range owns the window bounds; the callback does not copy
the other devices' address map or recursively query them.

A fallback claiming every whole multi-byte span would be incorrect: it could
intercept valid adjacent GPR words before Core discovers their narrower
device routes. Byte-only admission deliberately leaves the existing Core
scan/split algorithm in charge of those transfers. A mixed access retains
the existing accepted prefix; there is no rollback, retry or local bus loop.
No wider query workaround is installed in Board.

The separate Shared whole-span query classification defect recorded in S15
still applies: a query wider than the available route can report RAM even
though actual data access uses the byte providers. This batch qualifies data
access and every byte's classification, not that broader public query.
Its original Shared TODO/approval boundary remains mandatory; no assertion
requiring the incorrect wide-query result is added. B02 cannot be called
fully qualified from this receiver alone.

## Existing Test Ownership, Not New Test Volume

The prior transfer/update/collision matrices placed deliberately failing
operands next to actual COP registers. Their readable prefixes depended on
ordinary RAM in Board control holes. Keeping that setup would either fail
before the intended stage or force a production bypass for test convenience.

Those three existing matrices now instantiate the actual private COP with a
neutral V30 Core and its existing public memory and attachment interfaces.
The fixture's short advancement callback invokes the real COP safe point and
reports its failure into the existing Core fault path. It has no CPU loop,
timing model, copied COP, new production entry, fabricated hardware board or
Shared implementation access. Reset calls the actual COP reset; destruction
finalizes COP once through the bound Core attachment. Construction failure
before binding releases the unbound COP; after binding, finalization clears
the pointer before the final null-safe release. The stack fixture outlives
all callbacks and Core destruction.

The tables themselves are byte-identical after CR/LF normalization against
04a378964:13 transfer rows,25 update rows and14 collision rows. All expected
effects, command-port variants, memory failure stages, lifecycle fault
assertions, result latches, prefix preservation and reset assertions remain.
No old case was discarded or redefined to fail earlier. Actual Core DMA and
guest instruction execution are still used, rather than directly invoking
an algorithm and assuming the bus works.

The Board continues to test its real COP register decode, refusal, mode/
decimal/reset/busy behavior, IRQ, inputs, sound and program-bank wiring.
Its reset/admission test now upgrades the prior final504h acceptance-only
check to an actual V30-issued5205 trigger, safe-point execution, four-byte
RAM copy and unchanged expected retirement/HLT result. Thus moving the
isolated matrices does not eliminate proof of production Board integration.

All tests remain inside the existing board-bus executable: no new file,
framework, target, fixture borrowing or redundant CPU qualification suite.
The five local fixture helpers express actual create/advance/reset/finalize/
destroy ownership; they do not expose a test-only production capability.

## Complete Operational Window Proof

The existing executable adds a bounded window matrix:

- All1024 control-window bytes query as a provider in both directions,
  before and after reset.
- Ten representative hole, Video/protection-owned and boundary locations
  reject operational reads, inspection and writes at1/2/4/8-byte widths.
  This proves refusal, not those devices' unimplemented behavior.
- Four-byte GPR writes, operational reads and inspections retain both words;
  a too-broad fallback would reject this valid path and fail the test.
- A3ffh--400h write retains its accepted RAM byte at3ffh, then fails.
  A7ffh--800h write fails before touching the seeded RAM byte at800h.
- An actual V30 store to unknown400h faults through the production Board/Core
  route, rather than silently updating hidden RAM.

S15's per-unmodelled-range mixed-write checks now reject at the first
preceding unknown Board byte instead of treating it as RAM. That former
accepted-prefix scenario moves to the real3ffh/400h boundary above; it is not
silently deleted. S15's old426h RAM query becomes a one-byte provider query.
Wide negative-capability COP spans and all original individual-lane checks
remain unchanged.

## Actual Delta And Scope Review

By git diff --numstat against04a378964, production board.c adds38/removes3,
net35; the existing board_bus_smoke.c adds173/removes26, net147. These are
within the admitted estimates. There are three new private production
callbacks required by the existing registered memory-device contract and one
route. No new state, public interface, manager or second memory owner exists.
The route installation count now derives from the actual array instead of
maintaining another literal count.

Production COP, Shared, other Apps, source/test build files, manifests and
release inputs are unchanged. The local product package has no manifest to
regenerate. The positive test delta funds the bounded window matrix and one
actual-COP fixture reused by all three original failure matrices, not one
test per address/command or a new generic harness.

Frozen production SHA-256:
AEE8B92AAD1CE6D9037A9ECC23FA5FE1759DC1824F07E24B3AB8E7C65E03312E;
frozen test SHA-256:
B94571EA6BA3D1B6A7B98A022B5AA1BD16838C7B1C12BE5A510A411BE9D9BAC4.

## Verification And Retained Limits

The original x64/x86 cached graphs complete strict affected builds; focused
Board/COP tests pass both widths. Full687-unit runs use four workers and the
existing600-second aggregate containment, architectures sequentially. Final
results/hashes and protected-input guards are recorded after terminal runs.
There are no temporary task directories or background probes to clean.

No game ROM, desktop, RDP, real-set startup, full integration suite or playable
game is executed or claimed. The current MyRaidenII Core is not linked into
the ten existing released EXEs, so this receiver does not rebuild unaffected
products. The original numeric/math,2a05,unresolved/DMA algorithms, Video/
Sound and genuine startup/gameplay receivers retain their required scope.

## Final Executor Qualification

Frozen-source full units pass687/687 on both widths, zero failures/skips:
x64 122.30s; x86 133.90s. Both owned runner sessions are terminal, exit0.
LastTest.log SHA-256, x64:
D5446342D8BB23C255A749BD16BA8E461EF421656642B0436FE01C69DA71E936;
x86:
D5E78894567FD6F91ABFACAB1507D63592FC326C418C2AA0810AB3565BCEA686.
Final source/test hashes remain exactly the frozen identities above.

Post-test guards match all ten released EXEs,33 tracked assets and6 protected
media-role records. Documentation governance and whitespace checks pass.
No task temporary directory was created; build contains only.ccache/.ninja.
No cleanup operation, external input change, Shared or other-App mutation
occurred. Full unit success does not qualify the explicitly unexecuted
integrations, desktop/RDP or playable game, nor close T3/M1.

## Coordinator Acceptance

Sequential coordinator git show review of acc9cc10c accepts all five changed
files against the original packet. The fallback has one range owner, runs
after installed devices, refuses data without storage and delegates mixed
accesses to the existing Core splitter. The actual COP fixture owns no second
CPU loop or implementation; its binding/finalization lifetime is valid and
all original matrix rows/assertions survive. Production Board guest success
and failure proof remain present. No public interface, Shared change, source
import, artifact or protected-input alteration is hidden in the delivery.

P1 totals423 additions/29 removals/net394: production38/3/net35,
existing test173/26/net147 and records212/0/net212. All are within the
admitted estimates. This acceptance changes only governance, not the frozen
source/test qualification. S16 closes its operational receiver, not all of
B02, T3 or M1; all stated query/algorithm/startup limits remain explicit.

## Archived S16 Packet

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation: MyRaidenII M1 T3 S16 after accepted S15 04a378964; one session, sequential coordinator/executor. |
| Admission And Approval | Owner's complete MyRaidenII implementation approval and delegated ordinary same-T ordering; MyRaidenII source/test/records only, no Shared change. |
| Objective | Consume B02's remaining low-control data-route receiver: unknown400h--7ffh accesses must not read/write ordinary RAM. Preserve installed COP/input/sound/bank devices, actual CPU delivery, cross-boundary accepted-prefix behavior and ordinary RAM outside the window. |
| Non-goals | No Shared/API change, hardware open-bus guess, Video/DMA/RNG/geometry algorithm, complete-span query repair, source import, real-set startup or gameplay qualification. T3's original outstanding receivers remain required. |
| Reference Baseline | Clean pushed04a378964; current Board routes, COP device and Core fallback/byte-splitting implementation; fixed MAME c1425a1af00b88aa07477b1b23a30ed5e5f856ff selected-board map, SHA-256 FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A. |
| Candidate Proposal | ../proposals/m1-board-cop.md; B02/B03/B07/B09 in ../etc/evidence/t3-s1-board-contract.md; S15 complete COP inventory and explicit low-control receiver. |
| Files And ABI Surface | Board source, existing Board/COP unit and MyRaidenII evidence/index/Current. Private fallback callbacks only, no public header/state. Estimate production+30--50/-2--8; existing test+120--180/-20--40; records+160--240. |
| Applicable Rules | Architecture Board owns undecoded physical windows, installed devices retain priority, one Core bus/scheduler; Coding bounded local callbacks and existing fixture reuse, no decoder duplication or test-only production entry; Source policy map facts only; Execution complete-window disposition, preserve failure stages, actual-P review and bounded cleanup; Documentation explicit query/model/startup limits. |
| Verification | Every single byte in400h--7ffh queries as a provider for read/write before/after reset. Representative previously unclaimed and Video-owned ranges reject read/inspect/write at byte/word/wider sizes, never guessed data. Boundary3ffh accepted write prefix remains,7ffh rejection leaves800h unchanged. Existing input/sound/bank/COP/decimal and wider readable GPR data paths retain behavior. Keep all transfer/update/collision cases and exact failure stages on an isolated actual COP/Core fixture, not fake writable Board holes. Prove a real Board guest COP transfer and an actual CPU unknown-control fault. Strict cached x64/x86 builds and focused tests, then all687 units per width sequentially,4 workers/600-second aggregate. Docs/whitespace/input/release guards and cleanup. |
| Expected Markers | One FALLBACK route uses byte-only query and terminal INVALID_STATE read/write, after real devices; Core's existing byte scan/split remains sole cross-range owner. No local wide-query workaround. Existing COP tests instantiate the actual private COP with neutral Core only where arbitrary memory/failure layouts are required; Board wiring remains directly tested. |
| Asset Needs | Synthetic ROM-free tests only; no disposable task root planned. Persistent Ninja/ccache reused. Preserve ten releases,33 tracked assets and6 protected media-role hashes. |
| Reporting Requirements | Report route-priority evidence and estimates before code, fixture ownership/failure-stage sweep, actual added/removed/net code, final qualification/remaining query debt, full P push and coordinator review. |
| Stop Conditions | Shared/public capability, protected input, source copying, guessed hardware success/open-bus, duplicate dispatch, weakened original failure assertions or expansion into unrelated devices. |
| Exit Criteria | The complete low-control window has an explicit operational non-RAM disposition, existing real devices and cross-lane access remain correct, all original failure-stage assertions survive in their proper fixture, dual-width proof/gates/guards pass, complete P and acceptance are pushed. Public wide-query classification and real startup remain explicitly unqualified. |
| Original Owner Request | Implement playable MyRaidenII under single-session dual-role governance with coherent batches and reused tests; routine same-T planning delegated, Shared changes separately reviewed. |
| Similar-Issue Sweep | Review read/write/inspection/CPU/cache behavior, fallback widths and exact overlap priority. Preserve a valid neighbouring provider instead of intercepting mixed spans. Do not seed isolated COP failure operands into actual Board control holes or duplicate Board timing merely to test COP effects. |
