# M5 T232: CMOS/RTC Deterministic PC/AT Contract

## Scope

T232 turns the default-profile CMOS byte store into one VM-owned MC146818-
compatible RTC/NVRAM subset. Guest time is derived only from core-owned
`elapsed_ticks`; host wall clock never changes a running guest register. This
does not change the existing BIOS `INT 08h` / `INT 1Ah` BDA tick route, and it
does not claim a complete RTC, host-time synchronization, or battery-backed
file persistence.

## S1: Contract And Probe Design

**Status:** Complete.

### Ownership And Route

```text
core_machine elapsed-tick scheduler
  -> frozen generic execution-provider advance(delta_ticks)
  -> VM session device lifecycle
  -> vm_machine_cmos RTC/NVRAM state
  -> IRQ8 source -> core PIC slave/master cascade -> guest IVT 70h

guest 70h/71h ports <-> vm_machine_cmos registers
guest ROM INT 1Ah <-> same 70h/71h register path
```

- Core owns elapsed tick advancement and calls one generic, frozen execution
  provider after its built-in scheduler boundary. Core does not name or depend
  on CMOS/RTC/VM code.
- `vm/machine/cmos` owns the RTC registers, NVRAM bytes, binary/BCD conversion,
  update/periodic/alarm counters, interrupt flags, and its bound IRQ8 source.
- The default PC/AT profile owns ports 70h/71h, IRQ8, default clock ratios and
  initial CMOS bytes. Composition only binds those immutable declarations.
- Platform may pace host execution but cannot initialize, advance, or write
  guest RTC state.

### Admitted Register And Time Slice

| Area | T232 behavior |
| --- | --- |
| Index/data ports | 70h selects bit 0--6; bit 7 preserves the existing NMI mask behavior. 71h reads/writes the selected register. |
| Time registers | seconds/minutes/hours/day/week/day/month/year/century; default BCD + 24-hour mode. Register B binary and 12/24-hour modes are admitted for reads/writes. |
| Register A | UIP is a deterministic, tick-derived final update window; DV bits retain their reset value; rate select drives the admitted periodic interrupt divisor. |
| Register B | SET freezes time advancement; PIE/AIE/UIE enable their respective IRQ8 causes; DM and 24h select data presentation. |
| Register C/D | C reports IRQF/PF/AF/UF and clears all C interrupt flags plus the RTC IRQ8 source on read. D reports VRT=1. |
| Alarm | Exact BCD/binary match for seconds/minutes/hours asserts AF. Don't-care alarm encodings and century alarm are deferred. |
| Determinism | Profile supplies `rtc_ticks_per_second`; a fixed divider produces one second and optional periodic events. Cold reset restores profile RTC time; paused/debugged time does not advance; NVRAM survives reset but not session destruction. |
| Interrupt | Update, periodic, and alarm causes set C flags. IRQ8 is asserted only when the matching enable is set; reading C deasserts the source. No direct CPU/BDA mutation is allowed. |

### Deferred Behavior

Host-clock synchronization, persistent battery files, NVRAM checksum policy
outside existing defaults, square-wave output, oscillator failure, diagnostic
shutdown semantics, don't-care alarm fields, and RTC-specific ROM alarm
services are deferred. Existing BIOS `INT 1Ah` time/date services remain a
normal 70h/71h port client and are regression-protected, not redesigned.

### Evidence And Stop Conditions

- A CMOS port probe must check BCD/binary and 12/24-hour conversion, SET,
  deterministic second advance, periodic/update/alarm C flags, IRQ8 cascade,
  read-C acknowledgement, reset/NVRAM policy, and pause/reset boundaries.
- A ROM/system fixture must read RTC time/date through `INT 1Ah`, plus retained
  FDD/HDD boot, DOS prompt, Console, and debugger regressions.
- A boundary gate rejects host-clock APIs, direct BDA/CPU mutation, a VM-side
  instruction loop, and unbound numeric IRQ8 updates.
- Stop before S2 if the work needs a second tick counter, host sleep/wall clock,
  direct PIC IRR/CPU mutation, an RTC-specific core dependency, or a user
  visible NXVM Console/startup change.

## S2: Owned Implementation

**Status:** Complete.

- Extended the frozen core execution-provider contract with an `advance` hook.
  The core scheduler supplies elapsed ticks after core-owned device progression;
  the VM session lifecycle forwards that delta to its one CMOS owner.
- Bound default-profile ports 70h/71h, IRQ8, and the frozen 50,000-tick RTC
  second divider through composition. The RTC owns the MC146818 register slice,
  NVRAM bytes, BCD/binary and 12/24-hour conversion, SET freeze, periodic,
  update, and exact alarm flagging.
- The RTC asserts/deasserts only its PIC source. It has no host-clock call,
  no BDA write, no direct CPU interrupt delivery, and no VM-side execution
  loop. Firmware `INT 1Ah` remains the existing guest port client.

## S3: Verification And Closure

**Status:** Complete.

The focused port smoke covers deterministic second progression, BCD/binary and
12/24-hour conversion, SET freeze, periodic/update/alarm flags, the IRQ8
slave cascade, read-C acknowledgement, and cold-reset NVRAM retention. The
boundary gate rejects host-clock calls and requires the elapsed-tick, profile
binding, and PIC-source route.

`current-gates-gcc` passed with **67/67** current CTest smoke tests, including
FDD/HDD boot, DOS prompt, timer firmware, keyboard, MEM, CGA, Console, and
debugger coverage. The retained RAM closure emits its pre-existing CMake dev
warning for an invalid regular-expression escape but passes; it is outside
this task's RTC scope.

Artifact: `build/output/nxvm_0_5_0232.exe`

SHA-256: `B3BDABEEA31E4F61063447905259286AD374EB2661CE7492D21030F58EA7E37B`

Source commit: `8f90a92`
