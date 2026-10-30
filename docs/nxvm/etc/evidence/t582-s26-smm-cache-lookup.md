# T582 S26 — SMM write-path lookup audit

The prior World Clock observation recorded 27,652 SMM entries in about 60
seconds. The current SMM entry writes the 512-byte save image in 128 ordered
four-byte physical operations; RSM reads 128 ordered operations. If each entry
has a successful matching RSM, this implies about 7.08 million physical
operations over that interval. The recorded entry counter does not itself
prove every RSM succeeded or measure host time spent in these operations.

On an 80486 write, `_kma_write_physical` looks up a cache way before calling
`_kma_write_bus`; the latter performs another lookup after the write and
external-cycle callbacks, to publish the write-buffer cache-hit fact. It is
tempting to pass the first result directly. That is not established equivalent:
the callback boundary permits changes to the memory view, and existing
`x86_core_cpu_execution_invalidate_memory_view` invalidates cached lines.
Machine device writes can also notify the CPU through the snoop path. The
normal CPU-owned transaction filters its own write from that snoop, but this
alone does not prove that every provider or callback preserves the cache view.

The candidate is therefore not adopted or benchmarked. A useful next probe
must measure the two lookups in the real World Clock workload and establish
whether a same-transaction invalidation can occur before the completion lookup;
any optimization must preserve that ordering, the individual bus operations,
faults, write-buffer accounting and SMM timing. No CPU, timing contract,
public interface, test or executable was changed by this audit.
