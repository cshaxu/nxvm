# T582 S34 — PIT deadline simplification

`x86_pit_ticks_until_output` returns `LIB_STATUS_OK` only after setting a
nonzero deadline: restart/trigger/load returns one, and a zero computed
counter value is rejected as invalid state. The board's `board_consider_pit`
therefore cannot receive a successful zero deadline from this owner. Its
clock helper returns true only for zero input; nonzero conversion either
produces a nonzero source deadline or fails without changing the minimum.

The candidate preserves the three PIT calls, order, nearest positive device
deadline and existing clock conversion. It removes the impossible immediate
result and uses zero as the unset minimum. Board `immediate_due` handling for
other devices is unchanged. There is no added interface, state or path.
Production diff: 7 additions, 15 deletions; source manifest changed one hash.

Cached x64 and x86 builds and 13 focused PIT/deadline tests per width passed.
An independent fixed-work baseline/candidate/candidate/baseline comparison
used identical private CMOS/media. Each reached 10,000,000 instructions,
79,964,294 guest ticks and PC `0003dbc5`. Times in milliseconds were
17,224/17,669/17,545/17,681 on x64 and
21,627/21,122/21,674/21,741 on x86. These noisy pairs establish neither a
reliable speedup nor full machine-state equivalence. Guest log SHA-256:
`55D8C1AE056B1D944DAF7568F47DBF9FDB5B11D3540D0F13C8C59ABFF5B25DDB`.

The cached receiver ran 712 unit tests per architecture. Default integration
was 48/48 on x86; on x64 47/48 passed initially and MyNES native-window failed
with a black or invalid sampled pixel, then passed when run individually. Its
cause was not established, so the initial failure is retained here. The
5160/5170/DeskPro profile integration runs passed 3/5/6 tests respectively on
both architectures. MyRaidenII selected-set startup and product-console
receivers passed on both architectures. PE widths were checked for all 14
artifacts; the default NXVM, PC110, MyNES and MyRaidenII deployed identities
were also compared with their build outputs. See
[artifact inventory](t582-s34-pit-deadline-artifacts.json).

These results do not establish native Window/RDP gameplay responsiveness or
PC110 interactive usability. The fixed-work comparison does not demonstrate a
user-visible performance gain.
