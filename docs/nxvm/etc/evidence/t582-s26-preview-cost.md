# T582 S26 — lexical preview call cost

A scratch-only linker wrapper counted calls from other object files to the existing CPU lexical preview during a fixed PC110 startup workload. It sampled one in 128 calls with host `rdtsc`; this is diagnostic instrumentation, not guest timing. Both runs retired 10,000,000 instructions, reached 79,964,294 guest ticks and PC `0003dbc5`.

| Width | Preview calls | Samples | Sampled TSC units | Elapsed |
| --- | ---: | ---: | ---: | ---: |
| x64 | 971,583 | 7,590 | 12,396,170 | 18,976 ms |
| x86 | 971,614 | 7,590 | 11,946,882 | 23,406 ms |

This is roughly one intercepted preview per ten retired instructions, with mean sampled intervals of 1,633 and 1,574 TSC units respectively. The wrapper intercepts external symbol references, not necessarily calls within the defining object; the count is therefore limited to that linkage boundary. Deterministic sampling can alias with guest loops, `rdtsc` is not serialized, instrumentation changes layout and host scheduling may affect both elapsed and sampled intervals. The startup checkpoint is not a World Clock steady-state measurement. These values do not establish a whole-process percentage or a performance gain.

The earlier [copy candidate](t582-s26-preview-audit.md) had inconsistent cross-width results and did not prove that omitted memory records were unread on every path. This new count establishes that preview merits consideration, but does not supply the missing semantic proof or demonstrate that the copy candidate improves usability. No production implementation, public interface, tests, or executable was changed.

Scratch log SHA-256: `E032D430545C351F62B143F6E5201A1C820BCD93FABFC2DA88F09E725CF8A73D`.
