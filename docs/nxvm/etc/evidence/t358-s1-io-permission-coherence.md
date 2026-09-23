# T358 S1: Cross-Mode I/O Permission Coherence

## Authority and scope

Intel 80386 protection rules for `IN`, `OUT`, `INS`, and `OUTS` are the
architectural authority: real-mode I/O is permitted; protected-mode I/O with
`CPL <= IOPL` is permitted; protected-mode I/O with `CPL > IOPL` and all
virtual-8086 I/O use the current 32-bit TSS I/O-permission map.  A set bit,
an unavailable busy 32-bit TSS, or an out-of-limit map access produces `#GP`
before I/O side effects.  VME/PVI are not part of the 80386 contract.

## Decision-to-commit ledger

| Stage | Shared owner | Real / permitted protected | Restricted protected / VM86 | Publication boundary |
| --- | --- | --- | --- | --- |
| Entry | all ordinary `E4`--`E7`/`EC`--`EF` and string `6C`--`6F` handlers | select operand, port, and transfer width | same | no provider or architectural destination publication yet |
| Permission classification | `_kpa_test_mode()` | bypasses map when `CR0.PE=0` or `CPL <= IOPL` and `VM=0` | delegates every byte of the access to `_kpa_test_iomap()` | no side effect |
| TSS map validation | `_kpa_test_iomap()` | not reached | requires 80386, valid busy 32-bit TSS, valid map base/limit, and every covered port bit clear | any failure produces `#GP` before provider call, register/memory/index publication, or string progress |
| Provider transaction | `_p_input()` / `_p_output()` | same shared path after permission | same shared path after permission | input begins a CPU port-read transaction then writes destination; output reads source then begins port-write transaction; provider failure cancels transaction and produces the existing error route |
| String commit | string transfer helpers | `INS` writes ES then advances DI; `OUTS` reads source then advances SI | identical only after map permit | denied/faulting transfer leaves ES destination/DS source, indices, and provider state unchanged |

The caller sweep is complete for this owner: the six width branches in string
input/output, all eight ordinary immediate/DX forms, and their 16-/32-bit
operand branches invoke `_p_input()` or `_p_output()`.  No alternate permission
map, provider ABI, or direct port execution route was found.

## Reproduced repair and evidence

The pre-80386 word-output tails of `OUT_I8_EAX` and `OUT_DX_EAX` invoked
`_d_imm()` / `_p_output()` without `TYPE_TRACE_CHECK_RETURN`.  They shared the
same error producer but not the owner-level short-circuit contract used by all
other ordinary and string callers.  T358 restores the common checked-return
boundary; no helper, TSS layout, provider, or delivery mechanism changes.

`core_machine_port_io_s55_smoke` now runs provider-failure nonpublication for
both the 8086 legacy and 80386 paths.  The retained
`core_machine_80386_protected_io_timing_smoke` executes `INSB` and `OUTSB`
with allowed and denied maps in both restricted protected mode and VM86,
including destination/source index and provider publication checks.  Its
ordinary matrix already covers all eight immediate/DX forms in permitted
protected, map-restricted, and VM86 contexts; `core_machine_tss_iomap_port_smoke`
retains map bound and IOPL-bypass coverage.

The focused mechanism marker is `M5:T358:S1:IO-PERMISSION:OK`.

## Transfers

This closes only the shared I/O-permission mechanism.  Exception/IRQ frame and
stack-transition semantics, segment/task/table transitions, paging, VME/PVI,
device behavior, bus timing, and x87 remain separate T358 or Queue work.
