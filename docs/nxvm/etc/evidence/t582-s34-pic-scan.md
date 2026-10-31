# T582 S34 — PIC selection scan consolidation

The current PIC has separate master and non-master priority scans. Both check
the same rotating priority, pending mask, per-line delay and in-service
priority. Only the master scan applies the special fully nested and cascade
wiring rules. A task-only candidate removes the separate non-master scan and
uses the existing master loop for both, guarding those cascade rules with the
existing `cascade_master` field. It adds no state or interface and removes 28
net source lines. Guest state, callback frequency and time are not changed by
the intended design.

The original and candidate C implementations were compiled together under
separate symbols and compared on 2,000,000 seeded PIC states on x64. The
inputs varied initialization status, rotating priority, requests, masks,
in-service bits, cascade configuration, special modes and delivery delays.
All returned status and request fields matched. Most generated states were
outside `OCW1`; this comparison does not replace the existing targeted PIC and
board regressions, does not establish behavior for all states and is not an
interactive qualification.

Configured-CMOS startup probes reached the same 10,000,000-instruction,
79,964,294-clock and `0003dbc5` checkpoint. Sequential
baseline/candidate/candidate/baseline times were 18381/17652/20619/15337 ms
on x64 and 25981/22522/24334/22451 ms on x86. The order changed the direction
on both widths; they do not demonstrate a product speedup or full-state
equivalence. An x64 same-process synthetic selector workload compiled at
`-O3` measured baseline/candidate/candidate/baseline 79/79/77/79 ms for four
million selections each. Its mostly idle input mix is illustrative, not a
PC110 workload or proof of no regression. Log SHA-256:
`A278244FCA6DEEEDFC0CEDD3B05583B5C68B23FD8FDF953340A55500671D9683`.

Admission is based on removing duplicate production logic with no demonstrated
material slowdown, not on a claim that PIC consolidation alone makes PC110
playable.

The actual Shared change is +4/-30 lines in `pic.c`, plus its manifest hash.
The retained loop's initial state checks are the same as before; non-master
controllers bypass master-only cascade filtering and return a false cascade
flag. The existing PIC contract and command tests passed 2/2 on each width.
The cached receiver then passed 712/712 unit and 48/48 default integration
tests per width; the additional 5160, 5170 and DeskPro profile integrations
passed 3/3, 5/5 and 6/6 per width. The selected-set MyRaidenII startup and
product Console checks passed on each width. The receiver restored the default
profile in each existing build tree.

All fourteen deployed EXEs were checked for PE width and are listed in the
[artifact inventory](t582-s34-pic-artifacts.json). The NXVM and MyPC110
deployments matched their current build output on each width; MyNES and
MyRaidenII deployments also matched and remained byte-identical to their
committed versions. PC profile deployment identity was exercised by the
profile integration checks. No interactive PC110 gameplay, RDP performance
or physical-device session was established by these tests. The PIC change
is a code consolidation, not a measured correction of the remaining slowdown.

SHA-256 (receiver log, external receiver log, artifact inventory, differential
fixture source):
`BCABDFB0941599F15E0FC6B53884C052DB96C7FB956A5CCD7928BB1975E4E2CC`,
`D829B6B9159B4FC435B77EA9664536C7FBF0141BD92E96C5ABA8C6E63833C585`,
`4D511D52EF935417DCAD0B394EDBB1D1580725D76FFC3720600022BACDD04522`,
`AC2E3691497CBADA86E23E676320CE15DD55FF304FA1ACFB58E83CDF1DACD1A2`.
