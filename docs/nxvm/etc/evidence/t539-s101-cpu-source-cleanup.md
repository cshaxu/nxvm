# M5 T539 S101 CPU Source Cleanup

## Result

S101 removes the uncompiled historical CPU source corpus from
`src/app-nxvm/devices`. The sole implementation owner is already-compiled
`src/x86/devices/cpu`; `src/app-nxvm/devices/cpu_bus.c` remains the board
adapter and is not a CPU implementation.

Deleted files:

- `cpu.c`, `cpu.h`, `cpu_interface.h`;
- `cpu_instructions.c`, `cpu_instructions.h`;
- `cpu_timing.c`, `cpu_timing.h`, `cpu_timing_model.c`; and
- `cpu_trace.h`.

The deletion removes **25,920** dead source lines. No canonical CPU source,
public interface, runtime behavior, profile, firmware, asset or INI changes.

## Static-Owner Repair

- `verify_core_cpu_pic_authority.cmake` no longer skips historical CPU copies;
  it rejects every deleted path if reintroduced.
- The CPU-boundary negative fixture removes its owned work directory before it
  copies canonical inputs, preventing old files from a preceding run becoming
  false evidence of a duplicate.
- `Verify-8086DecoderLedger.ps1` and `VerifyExecutorClosure.ps1` now inspect
  canonical `src/x86/devices/cpu` sources.

The complete stale-reference sweep used:

```text
rg -n "src/app-nxvm/devices/cpu|app-nxvm/devices/cpu_(interface|instructions|timing|trace)|app-nxvm/devices/cpu\\.h" . -g "!*build*" -g "!docs/**"
```

Its remaining hits are either the retained `cpu_bus.c` board adapter or the
deliberate forbidden include injected by `test/x86/verify_negative.cmake`.
Neither is a live CPU source or include.

## Verification

- Focused CPU-boundary negative: x64 and x86 pass.
- Focused 8086 decoder ledger: x64 and x86 pass.
- Full repository-only unit suites: x64 **467/467**, x86 **467/467**.
- Full x64 external integration: **20/20**.
- x64/x86: T317 fixed-width vocabulary, T332 fixture lifecycle, CPU/PIC
  authority and documentation governance all pass.
- x86 corpus, all six Shared source/test manifests and executor closure pass.
- `git diff --check` passes.

This is subtraction plus static/test/tool verification only. The already
deployed executable inputs are unchanged, so S101 correctly emits no new EXE.

## T539 Closure Review

The finite ledger now gives every former `app-nxvm/devices` file one final
disposition: an independent-chip extraction with a sole x86 owner, or a
retained board/composition responsibility with the reason recorded in the
ledger. The CPU row was the only incomplete disposition found by S100; S101
removes it. No unallocated chip row remains.
