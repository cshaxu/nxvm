# M5 T25 S2 RAM/A20 Authority

`memory.c/.h` now own the retained full-PC RAM implementation. Its one `t_ram`
object is embedded in `vm_composition_live_machine`; `vram` directly aliases
the bound object and no global RAM storage remains. `machine_memory.c/.h`
retain the independent M3 minimal-model API and are never constructed by full-
PC composition.

Windows GCC passed these automated gates:

- `M3:T1:S1:CORE-CONTRACT:OK` and `M3:T1:S2:MACHINE-INSTANCE:OK`;
- `M5:T23:S1:LIVE-MACHINE:OK` and `M5:T24:S1:CPU-AUTHORITY:OK`;
- `M5:T25:S2:RAM-AUTHORITY:OK`, covering same-object identity, A20-off
  aliasing, A20-on separation, and the historical high-ROM read mirror;
- expected `#UD` CPU-stop and probe checks, then their success markers;
- `M5:T14:S3:VM-DEBUG-TARGET:OK`, `M3:T3:S1:FULL-PC-PROFILE:OK`, retained
  product Console smoke, and dependency DAG with zero known migration edges;
- a bounded `HELP`/`EXIT` run of `nxvm-m5_t25.exe` with the retained banner
  and command surface.

The local task artifact is `build/output/nxvm-m5_t25.exe`, SHA-256
`C698D5D330E8AD9C4F7EE0D2812DCA7536F13527CA8F592B327A3DF334700E33`.

The actual FDD DOS-prompt run remains a P2 manual checkpoint. The automated
reset-vector/profile smoke is recorded evidence, not a replacement for that
runtime observation.
