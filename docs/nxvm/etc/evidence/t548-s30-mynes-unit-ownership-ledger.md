# T548 S30 — MyNES Unit Ownership Ledger

## Scope and comparison rule

This ledger covers all 44 current `test/app-mynes/unit` C entries.  A test is
App-owned when it selects MyNES's 6502/NES Core, mapper, cartridge, driver,
snapshot or provider binding.  Calling an Emulator API does not make that
behavior an Emulator test: the shared receiver owns only the neutral protocol,
while MyNES owns its concrete implementation and callback result.

## MyNES Core receivers

| Entry group | Entries | Canonical behavior and disposition |
| --- | --- | --- |
| 6502 instruction and observation | `addressing`, `alu`, `alu_exhaustive`, `branch`, `control`, `indirect_wrap`, `load_semantics`, `logic_compare`, `memory_increment_semantics`, `opcode_profile`, `register_semantics`, `rmw`, `shift_semantics`, `stack_control`, `status_instruction`, `store_addressing`, `interrupt`, `reset`, `run_step_equivalence`, `cycle_ledger`, `decode_ledger` | MyNES owns its 6502 decoder, addressing, flag, trace, interrupt, reset and cycle implementation.  No Lib/Emulator/Product receiver implements or selects this CPU.  Retain all entries; their exhaustive, ledger and trace contexts are distinct rather than duplicate spellings. |
| NES bus, timing and peripherals | `bus`, `dma`, `pacing`, `controller`, `apu`, `ppu`, `palette` | MyNES owns CPU/PPU/APU/DMA/controller bus wiring, pacing and NES frame palette production.  Shared components only transport copied frame/audio/input data.  Retain. |
| Cartridge and mapper personalities | `cartridge_contract`, `mapper_mapping`, `mapper1`, `mapper2`, `mapper3`, `mapper4` | Cartridge header admission, mapper bank mapping, serial register behavior, mirroring and IRQ interactions are MyNES device facts.  Retain. |
| Persistent media and snapshots | `battery_persistence`, `snapshot`, `media_failure_contract`, `media_title_suffix`, `owner_rom_probe` | MyNES owns ROM/Battery media transaction, MNS1 state and title behavior.  Lib Storage proves file mechanics separately, and Emulator proves generic Machine/session lifecycle separately.  Retain. |
| Complete NES machine | `machine` | The selected NROM construction, run/observe path and Core integration are the highest-level MyNES Core regression.  Its context is not reproduced by unit tests for individual CPU or shared layers.  Retain. |

## MyNES Product receivers

| Entry | Canonical behavior and disposition |
| --- | --- |
| `product/config_smoke` | MyNES INI keys, defaults and strict `display`/`console_control` input validation.  The shared monitor and Session do not parse MyNES configuration.  Retain. |
| `product/composition_smoke` | MyNES composition rejects absent startup media before Session/UI creation and unwinds its concrete driver.  Shared Emulator composition only owns its generic ordering.  Retain. |
| `product/battery_failure_smoke` | Failed battery persistence blocks the destructive MyNES media replacement.  This is a MyNES driver sequencing rule.  Retain. |
| `product/command_smoke` | One MyNES command-provider/router binds generic monitor syntax to NES ROM admission/ejection, save/load, nested NES Debug and runtime callbacks.  The generic fixed-command/help/unknown-command grammar remains covered by `test/emulator/product/monitor_smoke.c`; this receiver proves only MyNES-specific execution and extension behavior.  Retain. |

## Result

No entry has an equal-or-stronger receiver in Lib, Emulator, Product or another
MyNES entry.  No behavior is moved or deleted.  The only apparent overlap—the
top-level fixed command words—has distinct context: Emulator tests the grammar
with a neutral mock; MyNES tests the concrete NES provider effects.  This keeps
one canonical test for each generic contract without erasing product binding
coverage.

This is a documentation-only audit.  It changes no C/H, registration,
manifest, production behavior, firmware, media, INI, snapshot or executable
input.  S31 remains the sole final dual-width qualification batch.
