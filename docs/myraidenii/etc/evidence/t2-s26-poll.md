# T2 S26 Native POLL Facility

Admitted after accepted S25 `748876509`. Current holds the complete packet.
This consumes native V14 logical pin/wait and sampling cadence, not complete
electrical timing, general priorities, neutral Machine admission or gameplay.

## Primary Source And Before Witness

[NEC October 1986 V20/V30 user manual](https://www.dosdays.co.uk/media/nec/NEC-V20-V30-Users-Manual.pdf), SHA-256
`FABBFCD19D4A2FCDAAC9F0BC06B00BF0A6A4BEEDAFBD3FF16ACCD075A374EA4C`,
printed 12-171 / PDF 216 was rendered and visually inspected. Native opcode
9B is POLL: wait while the input is high, release at active low, preserve
flags and perform no data transfers. The stated formula is 2 + 5n. The
existing CPU wait API schedules its first later sample after initial base
cost plus one five-clock interval, then subsequent intervals at five.

Before repair the existing V30 path inherits Intel's three-clock base.
The added actual-execution unit fails with
`V30:POLL:FIRST-SAMPLE ticks=8`; it requires 7 for NEC. No primary instruction
decoder, board timing owner or foreign implementation is substituted.

The existing begin-wait owner now selects NEC's two-clock base. Existing
Intel 8086/8088, 80186, 286 and 386 bases and five-clock ESC restart are
unchanged. There is no new public signature, enum, stored state, wait owner,
thread, clock or interpreter. V30 retirement timing still reports unallocated:
this is the logical wait-deadline facility, not completed instruction/bus
accounting or Machine eligibility.

## Owned Proof And Retained Boundaries

The existing native CPU unit owns 35 cases: three PC positions (even, odd,
FFFF wrap), four arithmetic/IE flag seeds and ready/blocked input; five Intel
cadence witnesses; actual enabled-INT/NMI entry and RETI; reset abort; 8080
9B SBB E isolation; absent callback readiness; and failed fetch before any
pin sampling. Complete CPU equality proves the instruction/idle/release/NOP
effects; preview never samples the pin. Waiting refreshes neither fetch nor
retire another instruction. Zero/partial/exact elapsed time, a retained-high
sample and subsequent active-low release share the existing public wait APIs.
No data writes or ports may occur. Source-backed cadence does not promote
the general IRQ/POLL electrical restart policy beyond its existing logical
model. The pin callback's existing true=blocked interpretation is unchanged.

The tests use the existing CPU bus fixture and its callbacks, with one small
owner-local pin/counter provider. They do not add another RAM implementation,
test target or production entry. The 8080 and interrupt paths still execute
their real instructions, not host-injected return PCs. Existing tests remain.

## Similar-Issue Sweep

Reviewed WAIT, begin_wait, poll_wait, advance_wait, wait query, ExecInt,
execution init/reset, FPU ESC restart and emulation opcode selection.
The only inherited base requiring a change is native V30. Intel WAIT and
ESC cadences are distinct contracts, not duplicate paths to consolidate.
All later polls already use five clocks at the sole wait owner. Native
FPU configuration and 8080 9B retain their existing admission/interpretation.
The complete V15 timing selector and V16 Machine gate remain open.

```text
rg -n 'wait_poll_remaining_ticks|begin_wait|execution_poll_wait|execution_advance_wait|wait_test_asserted' src/x86-core/cpu -g '*.c'
```

## Verification And Delivery

Both original Release Ninja/ccache graphs build all targets and pass full
repository units, 674/674 per width (187 X86 Core), in 300.84/118.42 seconds.
The native unit takes 0.160242/0.153440 seconds and emits
`V30:POLL:OK cases=35` and the existing native-family success marker.
All 498 strict rows and Types/dependency/ownership/negative/manifest,
documentation and whitespace checks pass. Historical CPU warning exceptions
remain; this does not claim exemption-free compilation of every source.
Both final suites pass without retries or changed budgets. The expected
before witness is retained separately from final qualification.

The eight current PC 0564 EXEs are rebuilt and deployed. Each width passes
14 selected receiving integrations (5 default, 3 each XT/AT/DeskPro), then
one restored-default identity. Both original graphs are restored; profile
switches compile zero CPU objects, reusing the qualified object. Ten current
PE/name/architecture/size/hash identities pass; MyNES 0044 hashes, INIs,
external ROM/media and snapshots remain unchanged. No manual desktop/RDP,
physical audio, full external/native integration suite or game proof is
claimed. [Results](t2-s26-results.json) and
[release identities](t2-s26-artifacts.json) retain exact observations.

Shared P1 `e9d56309b` and receiving NXVM P2 `77da954af` are pushed. The
qualified CPU source SHA-256 is
`619542026A4F5CD71658DE4D13CC0309FE27BF026F802F57AF30B04F1E45290A`;
the native test is
`CA2ED6C7FFF00DA45DD1787800164589FB091661A543BAD56E4BEB00E97FFA2E`.
Both changed Shared files and manifests have identical raw/staged LF bytes.

Actual counted production is +4/-3/net +1, tests +159/-0/net +159, total
code net +160; the two manifests separately add/remove two lines. Method:
`git diff --numstat 748876509 -- src/x86-core/cpu/cpu_instructions.c test/x86-core/cpu/cpu_v30_native_smoke.c`.
Documentation/generated/artifact paths are excluded. The increase is owned
proof, not another CPU or wait path. No production helper/state/public API
is added. The three test-local functions count the pin, prepare the existing
fixture and verify execution; none is a public test-only production entry.

Executor self-review inspected the actual source/test/manifest changes,
the existing wait and interrupt/reset/8080 owners and unchanged consumers.
The complete sixteen-field packet is preserved in evidence delivery before
coordinator review. All final handles exited; the owned POLL rendering is
removed. The pinned source and indexed qualification logs remain for the
next CPU/timing receiver. T2 and the playable objective remain open.
