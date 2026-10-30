# T582 S26 — cache-route query cost

The existing 486 cache calls the Machine cache query for each relevant access, but asks for the more expensive whole-line routing proof only on a cache miss when filling is otherwise permitted. That proof includes bytewise ownership and a whole-line route check, so bypassing it on the assumption that a provider spans the address would risk missing narrower overlays, declines or query failures.

A scratch-only x64 linker wrapper measured the current query on a private PC110 startup run. It counted 22,842,000 ordinary lookups, four reset lookups, and 2,572,582 fill probes, of which 71 returned fill enabled. One in 256 calls was timed with non-serialized host `rdtsc`: 89,226 ordinary lookup samples totaled 4,073,650 units; 10,049 fill samples totaled 1,481,940 units (rough means about 46 and 147 units). The run reached the fixed 10,000,000-instruction, 79,964,294-tick, PC `0003dbc5` checkpoint in 20,011 ms. Counts include the stopping interval and instrumentation, while deterministic sampling, host scheduling, compiler layout and counter behavior limit cost estimates.

The low fill-acceptance count is specific to early startup: [S17](t582-s17-pc110-cacheability.md) observed chipset cache controls changing later, and [S18](t582-s18-cache-design.md) measured a bounded product-path improvement after provider caching. The count neither proves that disabled fills are incorrect nor describes the Game phase. The measured per-call samples do not demonstrate an order-of-magnitude opportunity, and no speculative state cache or route-proof shortcut is proposed on this evidence. No production code, interface, media or executable changed.

Scratch log SHA-256: `A3758BFEF65AA6C8119AAA67A9415BF46426F49198F982B80ACE4C35F9F4757F`.

## Ordinary-access test-register publication audit

The 486 cache reports normal data reads, fills and writes into the existing
TR4/read-buffer circuitry as well as updating replacement state. The T560 S5
source review records the Intel developer manual's warning that ordinary
memory operations can alter these test-register buffers; it does not establish
a deterministic value for every real 486 access.

A deliberately non-production ablation removed the three ordinary-access
`_kma_cache_report` calls while retaining the TR5 cache-test commands. The
existing `cpu_cache_smoke.c` suite still passed, so that suite alone cannot
establish the current ordinary-access publication behavior. A separate scratch
probe using the existing CPU fixture performed a guest load, selected TR3's
read buffer through TR5, then read TR3 and TR4 through actual MOV instructions.
The baseline reported TR3 `00001234` and TR4 `00002588`; the ablation reported
zero for both. This proves the proposed deletion changes the current
guest-visible model, not that either value is a mandatory silicon value. The
ablation was not admitted or used as a speed result, and no production source,
test or artifact changed. Any future optimization must preserve this contract
or separately justify and review a hardware-model change; the existing test
coverage gap should not be mistaken for permission to remove it.

## Game-stage report cost

An independent x64 private-media product-path run instrumented the existing
cache-report function and followed the same Personaware Game input and
sample-30 to sample-34 drawing window as the prior Game probes. The sample-34
frame matched SHA-256
`E5C1AB80C00F84C1B80C1069EFA58FD01D761FDE7AC07F7982F0B0B34AB0DE99`;
the guest exited successfully after 353 seconds.

During the approximate drawing window it counted 9,570,449 reports. A
pseudorandom one-in-256 sample produced 37,491 intervals totalling 2,769,877
host TSC units, about 74 per sampled call including instrumentation. Counts
and timing cover the existing report only, not the enclosing memory access;
stage changes are asynchronous to guest execution, the TSC is not serialized,
and instrumentation and scheduling affect the result. The result identifies
high frequency but does not support this small routine as the source of the
roughly tenfold guest slowdown. It does not authorize dropping the observable
TR state or establish x86/native desktop performance. No source or released
binary changed. Guest log SHA-256:
`9A5FC66259B2AA20A39687897CFA6557ADC2A2A1548DD5397AB8A894DE9004B2`.
