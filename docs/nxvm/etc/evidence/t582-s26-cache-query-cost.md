# T582 S26 — cache-route query cost

The existing 486 cache calls the Machine cache query for each relevant access, but asks for the more expensive whole-line routing proof only on a cache miss when filling is otherwise permitted. That proof includes bytewise ownership and a whole-line route check, so bypassing it on the assumption that a provider spans the address would risk missing narrower overlays, declines or query failures.

A scratch-only x64 linker wrapper measured the current query on a private PC110 startup run. It counted 22,842,000 ordinary lookups, four reset lookups, and 2,572,582 fill probes, of which 71 returned fill enabled. One in 256 calls was timed with non-serialized host `rdtsc`: 89,226 ordinary lookup samples totaled 4,073,650 units; 10,049 fill samples totaled 1,481,940 units (rough means about 46 and 147 units). The run reached the fixed 10,000,000-instruction, 79,964,294-tick, PC `0003dbc5` checkpoint in 20,011 ms. Counts include the stopping interval and instrumentation, while deterministic sampling, host scheduling, compiler layout and counter behavior limit cost estimates.

The low fill-acceptance count is specific to early startup: [S17](t582-s17-pc110-cacheability.md) observed chipset cache controls changing later, and [S18](t582-s18-cache-design.md) measured a bounded product-path improvement after provider caching. The count neither proves that disabled fills are incorrect nor describes the Game phase. The measured per-call samples do not demonstrate an order-of-magnitude opportunity, and no speculative state cache or route-proof shortcut is proposed on this evidence. No production code, interface, media or executable changed.

Scratch log SHA-256: `A3758BFEF65AA6C8119AAA67A9415BF46426F49198F982B80ACE4C35F9F4757F`.
