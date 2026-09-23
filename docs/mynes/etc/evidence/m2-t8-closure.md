# M2 T8 Cycle And Interrupt Refinement Closure

T8 closes the admitted ordered-transfer and cycle-sampled interrupt scope on
one Core CPU/Bus path. S2 supplies recorder fixtures for fetches, dummy reads,
RMW, branches and stack/control. S3 supplies reset, IRQ/NMI entry, polling,
priority and vector-gate fixtures. S4 adds run/debug-step checkpoint equality.

On 2026-09-21 both shared trees were rebuilt and every executable in each
`test-*` directory passed. The delivered product artifacts are:

| Architecture | Format | SHA-256 |
| --- | --- | --- |
| x64 | `pei-x86-64` | `8F60A1A2CCBAB66F1C3682A9C6D640D3B3EABF351B6E912E4445B8661D0F0674` |
| x86 | `pei-i386` | `503E9572FEAAD0012A3A8C72BBE60BEEE9D9B2EF6C9D4B58F9C7FD9A7BF52835` |

T9 receives product lifecycle, media, command, host and cooked-console
reliability. PPU/APU/controller and equal KVM gameplay remain M3 receivers.
