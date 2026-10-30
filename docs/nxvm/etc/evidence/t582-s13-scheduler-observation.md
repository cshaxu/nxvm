# T582 S13 — scheduler cost and board deadlines

## S26 board-advance attribution

An x64 task-only linker-wrapper probe sampled one in 1,024 calls during the
same 10,000,000-instruction, 79,964,294-tick, PC `0003dbc5` startup
checkpoint. The instrumented run took 19,516 ms. Counts and sampled host TSC
totals were:

| Existing operation | Calls | Samples | TSC total |
| --- | ---: | ---: | ---: |
| Clock-domain advance | 61,454,707 | 60,014 | 3,082,880 |
| PIT advance | 10,289,341 | 10,048 | 941,140 |
| RTC advance | 10,289,341 | 10,048 | 572,920 |
| PIC refresh | 10,289,341 | 10,048 | 1,493,940 |
| PIC advance | 10,289,341 | 10,048 | 720,100 |
| KBC advance | 10,289,341 | 10,048 | 1,266,740 |

These are inclusive sampled intervals at intercepted link boundaries, not
exclusive timings or whole-machine percentages. Some operations nest, the
clock count includes multiple domains, and deterministic sampling, timer
overhead, compiler layout and host scheduling can distort rankings. The
results are early startup only and were gathered before the later compact
preview implementation. They identify no single board call whose removal
could justify skipping device events or claiming an order-of-magnitude gain.
No production or guest behavior was changed by this observation.

At the same fixed PC110 startup checkpoint (10,000,000 retired instructions, 79,964,294 guest ticks, PC 0003dbc5), task-owned wrappers sampled one in 2,048 calls using the Windows performance counter. Each run used private media and current cached libraries. Instrumentation adds work and changes executable layout; sampling is deterministic, not random, and may alias periodic device behavior. Values are inclusive, so the nested functions must not be added to their parent.

| Width | Elapsed | Publication calls / samples / ticks | Board deadline calls / samples / ticks | Board before / after sampled ticks |
| --- | ---: | --- | --- | --- |
| x64 | 22,433 ms | 10,012,784 / 4,889 / 35,185 | 10,302,393 / 5,030 / 16,908 | 13,191 / 8,501 over 5,026 samples each |
| x86 | 26,946 ms | 10,008,708 / 4,887 / 46,759 | 10,298,185 / 5,028 / 21,566 | 16,116 / 11,458 over 5,024 samples each |

At a 10 MHz counter frequency, mean sampled publication costs were about 0.72 microseconds x64 and 0.96 x86. Board deadline costs were about 0.34 and 0.43 microseconds. These are investigation estimates, not exact exclusive costs or proof of equivalent optimization. No throughput improvement is claimed.

In these samples, the board returned no L1 compatibility, blocked, immediate, or absent deadline cases. x64 deadlines were 1 tick for 18 samples, 2-8 for 163, 9-64 for 784 and over 64 for 4,065; x86 returned 18, 163, 784 and 4,063 respectively. This suggests many per-instruction re-evaluations occur before a board event, but it is not evidence for late Personaware execution or all device configurations.

The production scheduler advances the timeline, arbitration, board clocks, devices and provider in a definite order at every published CPU interval. A safe consolidation must preserve guest-time state at every observable port/memory access, input, interrupt, snapshot, pause and failure boundary, and invalidate any derived deadline after every relevant state mutation. The source-clock and device phases must remain owned by the existing clocks. An unconditional skip, unchecked cached deadline or a second owner would break the contract. Further measure actual mutation rates and late workload before proposing a minimal change.

No Shared production code, API, media or published executable changed. Desktop/RDP performance was not tested. Logs SHA-256: B939C8EC3F2BF0D301E214783F92F5017BAAC5984685FFFEA66D2B55CDE429F2, 05E21B24071385B0299D4C14F38F80F1FB3C2B5E84D05BBEBB4A1D7570EE38E0, ACB3C59613961F4435D16520374E9910EABED3EC45EB6338C960A9849DD2CCB0.

## Idle DMA first-tick candidate

A task-owned candidate queried both DMA controllers before the existing advancement loop and returned when neither had an unmasked request or active channel. It retained the original loop for every other case. This goes beyond S4's proven shortcut because it also skips the first cascade update; correctness would require proving cascade coherence after every request, register, reset and active-transfer transition, including masked requests. It is not production code.

All four pairs reached 10,000,000 instructions, 79,964,294 ticks and PC 0003dbc5. Baseline/candidate times were 21,941/21,458 ms on x64 and 28,195/26,778 ms on x86; the reverse-order candidate/baseline pair took 22,198/21,311 ms on x64 and 26,814/27,885 ms on x86. The x64 direction reversed and its observed variation exceeds the initial gain. No stable cross-width benefit is established, so the candidate is not admitted and the existing first-tick/cascade route remains. Equal checkpoint values do not prove DMA equivalence. Logs SHA-256: 4510CEA2DDDA1259D2EFEEBBBDDEB3D7A74F858B608EA1245873FDDB554BDFA2 and 165E4A68B4C249EC2D0351C97BFC17F5DC342F43B1C10BACFF390E9FD99C9A29.

## Late workload and narrow deadline division

A task-owned x64 product-path probe repeated sampled cost and deadline observation in the approximate 220–240 second window of a 252.72-second run. It exited zero, reached a 640x480 graphics frame and RTC second 20. Its first sampled call defines the time origin; deterministic one-in-2,048 sampling and instrumented executable layout can bias results. Among 4,833 board deadline samples, none reported L1 compatibility, blocked, immediate or absent deadlines; 26 reported one tick, 139 reported 2–8, 751 reported 9–64 and 3,917 reported more than 64. Publication measured 32,628 counter ticks over 4,763 samples, and board deadline 14,088 over 4,833 samples at 10 MHz, approximately 0.69 and 0.29 microseconds inclusive. This resembles early-workload frequency but does not independently verify x86 late behavior, all runtime mutations, desktop use or an optimization. Log SHA-256: 4D27FDCFB8EFB86BA6DA4B20FC6A2B2233C6AE7FD003B34CE85EB661B2C8906D.

A separate generic private candidate used 32-bit division when the already validated and phase-adjusted deadline numerator fit in 32 bits, retaining the original general path otherwise. A 500,000-case deterministic mathematical comparison covered valid and arbitrary phase values and extremes, skipping inputs rejected by the original overflow guard; it is not a production C-path test. Every 10,000,000-instruction comparison reached 79,964,294 guest ticks and PC 0003dbc5. Baseline/candidate times were 21,762/21,870 ms x64 and 27,093/25,960 ms x86; reversed candidate/baseline were 20,982/21,671 ms x64 and 27,012/26,660 ms x86. Both architectures changed direction between pairs. No stable cross-width gain is demonstrated, and this candidate was not admitted. Logs SHA-256: A768E421BE465BFCE3307A92E74661D0195CF81DE2ED3AF2E0FABFA440A4A096 and 4B894877ED1B1F3122600BBBD679E5951141FBB80A8E450B54E4D7F5B5C65F48. No production code or artifacts changed.

## Deadline conversion volume and overflow-guard candidates

A task-owned x64 link-wrapper counted 10,300,201 `source_ticks_until` calls during the fixed 10,000,000-instruction startup run; 1,325 used equal clock ratios, 10,298,876 unequal ratios, and 10,299,832 successful conversions had a phase-adjusted required numerator fitting 32 bits. The counter instruments every call and changes the executable; these counts do not establish late-workload behavior or the cost of an individual division. The checkpoint remained 79,964,294 guest ticks and PC 0003dbc5.

A private combined candidate avoided the 64-bit multiplication-overflow guard division when the delivered tick count fit in 32 bits (both multiplicands are then at most UINT32_MAX), and used 32-bit division when the adjusted numerator fit. Baseline/candidate elapsed times were 21,816/21,648 ms x64 and 27,944/28,127 ms x86. A guard-only candidate measured 23,070/22,316 ms x64 and 26,906/26,929 ms x86. Each is one ordered pair at the same checkpoint; neither demonstrates a material, reliable benefit on both architectures, and neither qualifies arbitrary C execution, overflow, or phase behavior. The experimental branches are not admitted. The existing clock owner and behavior remain. Log SHA-256: 866C46F43960A6F03A3536AEE7086582B6446B410EF95D80122E5B9197EA702F, B0138E0905B4258602645521C48374A1E2D95034F41B1E28991AB0EEDBF65888, EA2FA80C3E443089C195C06FFDF43E1B0C0A26A38CBA008125BB5201DF168D14.

## Deadline stability and device-access boundaries

Task-owned x64 fixed-work probes counted actual CPU-width port accesses and instrumented the existing memory provider route, using private input copies. The port probe observed 437,201 reads and 278,544 writes. A separate provider probe recorded 15,961,280 provider read attempts, including 1,161,464 observational reads and 13,279,531 instruction-fetch attempts, and 1,345,362 provider write attempts; no write-observer callback occurred. Address buckets for read attempts below A0000h / A0000h–BFFFFh / C0000h–FFFFFh / at least 100000h were 2,551,228 / 73,763 / 13,336,288 / 1; corresponding write attempts were 1,135,102 / 155,181 / 55,077 / 2. These are attempted provider routes, not proof of accepted side effects, unique instructions, or immutable ROM. The memory provider contract permits an ordinary read to alter device state; only an observation read is guaranteed not to.

A third probe compared each real board-deadline result with the immediately previous absolute deadline. Among 10,301,763 calls, 9,979,898 retained the previous future deadline, 15 moved earlier, two moved later, 321,828 reached/expired the previous deadline, and 20 were unavailable. All 17 early/late changes coincided with at least one CPU port access since the previous query. This does not rule out other changes, callbacks or unsampled modes. Instrumentation counts through the brief stopping interval and changes executable layout.

A combined probe conservatively treated every port access, non-observational provider read, provider write and write-observer call as a possible mutation. It found 9,974,928 stable queries, but only 2,281,464 of them had none of those accesses since the prior query; the other 7,693,464 did. It observed 14,796,509 potentially operational provider reads and 1,343,741 provider writes. These calls are not necessarily mutations, and identical counters do not prove a cached deadline safe: scheduler callbacks, DMA, interrupts, external input and access failure/partial effects require their own invalidation and exact ordering. However, under this conservative existing contract, the experiment does not justify a large new deadline-cache mechanism merely from the 97% stability figure. The later guest workload and x86 were not measured by these probes. All reached the same 10,000,000-instruction, 79,964,294-tick, PC 0003dbc5 checkpoint; no production change or performance gain is claimed. Log SHA-256: CE662EF968E10CEFB4CD133CB0990AB15BFFDD5FE3FB1D50AA20AE32AEB007D3, 3ED81E9005EC0EDA253D798B72404B7580B66C5B8A261D9D22A42746BA42CD7E, 08F9AA392810631B4E64015D964AB7BE6C77D2B5A9CDFC4D136F145421825979, CA3ECE1AB3499CE2EABE395ED3BE59B82C089DC9649FF79576F41471DDD06EA4.

## Per-byte prefix classification candidate

`ExecIns` classifies the same opcode as a prefix before dispatch and again for continuation (and once more in one V30 branch). A task-owned candidate saved the first result in a local variable for the same loop iteration. It adds no state, handler, table or public API; profile immutability and V30/fault semantics would still require full verification before production. Every run reached 10,000,000 instructions, 79,964,294 guest ticks and PC 0003dbc5. Baseline/candidate times were 23,236/22,049 ms x64 and 27,804/25,716 ms x86; reverse-order candidate/baseline were 22,934/21,468 ms x64 and 29,026/27,682 ms x86. Both directions reversed across the pairs, so no stable gain is established. The candidate is not admitted; no execution path or artifact changed. Logs SHA-256: 3B7389D92C25ED19AB1A21BA0003356074C59BDED0FD75890BBB11A562C2A732 and BEC6D761330DD8B03A0A3B4E62569A591D06C02B9DAFC8D7E6C9D054BC678E7E.

## PIC empty deadline probe

PIC deadline selection scans eight delayed-unmask entries even when the already computed unmasked pending mask is zero. A temporary candidate returned the original success result immediately in that case; argument validation and all nonempty-mask paths remained unchanged. With private copied media, the identical fixed 10,000,000-instruction checkpoint was 79,964,294 ticks and PC 0003dbc5 for both variants. x64 baseline/candidate took 22,055/20,551 ms and x86 took 26,632/27,316 ms. These are single ordered pairs with opposite directions; they do not establish a reliable cross-width gain or full PIC behavior. The candidate is not admitted and no production code changed. Log SHA-256: D97357024C2DA116751A07BDB9DA779152A874F4C2AD5EFD0AE18D712BB832FA.

A reverse-order comparison at the same checkpoint measured candidate/baseline 21,600/22,066 ms x64 and 28,389/28,758 ms x86. The x86 direction changed between pairs and the small differences remain subject to run-order and host variance; these measurements still do not justify a production change. Reverse log SHA-256: E15DDD3170EED40EA7621D4A3E89501E087A93033DE8BAE533C4DA08B1F730C6.

## PC110 firmware route-query probe

The dynamic E/F shadow/Flash provider performs a decode in both the route query and actual read. A task-only candidate recognized an in-range E/F query after checking the same firmware, chip and access prerequisites; the actual read still performed normal chipset decoding. It did not alter ROM or cache eligibility. At the identical 10,000,000-instruction, 79,964,294-tick, PC 0003dbc5 checkpoint, baseline/candidate were 22,247/24,281 ms x64 and 26,740/27,093 ms x86. These are single sequential pairs, but neither shows benefit. The additional fast path is not admitted; no production source changed. Log SHA-256: 609170E0A18CB5E8304226A83FE3A420546297DE0A92ABF613A24DC5F5C75A4B.

## Memory provider pass counts

A task-owned x64 instrumented Machine memory source sampled route resolution during the fixed startup workload. A fixed 1/1024 sampling run observed 24,762 resolutions and 176,004 / 112,908 / 44,568 / 43,000 provider-loop iterations in the raw, replacement, ordinary and fallback passes. A separate xorshift-sampled run observed 24,657 resolutions and 175,056 / 112,317 / 44,752 / 43,008 iterations, about 15.2 provider entries traversed per sampled resolution. The latter reported 19,281 provider and 5,376 RAM outcomes; this is a specific startup checkpoint, not a representative whole-system or time-cost estimate. Both runs completed 10,000,000 instructions at 79,964,294 ticks and PC 0003dbc5. Instrumentation changes execution cost; deterministic and pseudorandom sampling have distinct biases. Raw overlays, aliases, replacement, ordinary and fallback query ordering and observational failure semantics must be retained in any candidate. No production changes. Logs SHA-256: AA4C5987EB8DFFC530B4E728BEBD518BFC288319B1D68A6DB8B2D4CFB12A4F92, D1D07B0170FBE31778F4C5BA9D2877C6539CF3AD88F1B14A407C274ACCC02C8B.

A task-only candidate leaves raw overlays and selected aliases first, then scans the registered providers once into a 64-byte local list of address-matching indices. Replacement, ordinary and fallback queries continue in their original tier and registration order. The existing provider limit is 64; queries are observational under the current contract. No persistent cache, new interface or external state is added. The temporary source diff is +17/-18 lines. An initial prototype was discarded because its generated list used an uninitialized index; no result from that faulty prototype is used.

For the corrected probe, all fixed runs reached 10,000,000 instructions, 79,964,294 guest ticks and PC 0003dbc5. Baseline/candidate times were 20,111/19,379 ms x64 and 25,104/23,787 ms x86; reverse candidate/baseline were 19,047/19,749 ms x64 and 22,961/25,543 ms x86. Six existing Core memory/ROM/registration tests linked to the candidate passed once on both architectures. These targeted results suggest a bounded early-workload opportunity but do not establish complete route semantics, full receiver qualification or playable speed. Validate maximum provider count, registration and invalidation constraints, aliases, failed/declined overlapping queries, CPU-only/reset providers, and actual late workload before admitting a production change. No production code changed. Logs SHA-256 (forward, reverse, targeted tests): 9A164C2C342FB4ABC4055C0416CFD7CF782DD3F0907BF80F67B18D7945A57C28, CA3317155B8F75D9858ED6CDFA787182F5085305B8058860C0D62E76F0EC44CB, C89F7375360AED16AB15B526E923B3306E41A6CAA98C5D992D8ECFAE7DF83612.

A sequential x64 product-path comparison linked the corrected local-list prototype into the existing PC110 factory/worker path and used separate copied CMOS/media. Baseline and candidate exited zero after 252.52 and 252.41 seconds. Last published guest quanta were 740,566,069 and 763,717,628 ticks (approximately 3.1% more for the candidate); RTC reached seconds 22 and 23. Final sampled PPM frames both hashed to 85A35680232CA0AD623DB7DD18D23A79396931ACB133791581D40C37048AEF41. This one ordered pair does not establish equal complete guest state, x86 late behavior, interactive usability or realtime speed. It supports a bounded Shared candidate, subject to the full receiving gate and explicit route-order, alias, ownership, failure and provider-limit tests. No production code changed during S13. Output log SHA-256 (baseline, candidate): E0C3AB6B18F3AA574A5A53C99B1A283E0D595067048E8541114A2D8123784BD7, C0FDB7B1435C9BA3ABDDF04C944F90F04924EE66880E9BDB068B20788F067B45.
