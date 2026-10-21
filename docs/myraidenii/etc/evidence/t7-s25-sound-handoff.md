# M1 T7 S25 — Board/Sound Handoff

## Baseline and evidence grade

Baseline: `2c3ce8354`. The existing Board and Sound implementation and two
existing synthetic units remain the only production and test routes.

Reference: MAME revision `c1425a1af00b88aa07477b1b23a30ed5e5f856ff`,
`src/mame/seibu/raiden2.cpp` and `src/mame/shared/seibusound.cpp` / `.h`.
The inspected local driver SHA-256 is
`FDDE57F6E9024657F2A9830864523305139562229750B87C967E597324CDB26A`.
The finite facts below are L2 external-emulator behavior, not silicon-exact
timing. No implementation, ROM bytes or lookup table is imported.

MAME 0.289 was also queried through its public Lua address-space API. One
frame callback wrote original synthetic values and immediately read the two
CPU address spaces, without intervening guest execution. The experiment swept
all 32 main-window byte addresses and the four pending-publication aliases.
It used the previously inventoried read-only external set. Only addresses,
synthetic values and non-reversible runtime summaries were retained.

## Confirmed defects and repair

| Contract | Correct physical main addresses | Previous defect |
| --- | --- | --- |
| Command byte 0 | `700`, `702` | A1 incorrectly selected another register |
| Command byte 1 | `704`, `706` | Two-byte rather than four-byte register stride |
| Response byte 0 | `708`, `70a` | Reply read from the wrong register |
| Response byte 1 | `70c`, `70e` | Reply read from the wrong register |
| RST18 request | `710`, `712` | IRQ write decoded at the wrong address |
| Main command pending read | `714`, `716` | Pending read decoded at the wrong address |
| Publish main pending | `708`, `70a`, `718`, `71a` | Missing aliases and response-pending transition |

Odd reads return zero and odd writes do nothing; unused even reads return
`ff`. Board owns this byte-lane decoding; Sound does not reinterpret main
addresses. A sound write to `4000` clears main-command pending and sets
sound-response pending. Main publication clears response pending and sets
command pending. Sound's `4012` reads response pending, not command pending.
The private callback was renamed accordingly; the two facts are not aliases.

Sound now retains separate in-service facts for RST18 and YM/RST10. Delivery
of RST18 consumes its pending request; YM remains level-driven. Service inhibits
same-source delivery until its device EOI: `4001` or `4003` for RST18, `4002`
for RST10. RST18 retains priority when both are eligible. CPU DI/EI is not a
device EOI. Reset clears both service facts. These are device state, not a
second scheduler, interrupt queue, recovery mechanism or CPU execution path.

## Regression and similar-path review

- Board unit exercises both command mirrors, ignored odd writes with distinct
  values, all 32 read lanes, both IRQ mirrors and ignored odd IRQ writes.
- The real Board-to-Sound execution route echoes `4012` through response byte
  zero and tests every main pending-publication alias, including word access.
- Sound unit preserves its command/response test and tests reset. The existing
  YM test now holds the timer level asserted, proves service inhibition and
  EOI re-entry, then checks reset. A small original Z80 program sweeps all
  three EOI addresses for RST18, including wrong-source EOI and DI/EI.
- Coin polarity is retained: the selected driver uses
  `SEIBU_COIN_INPUTS_INVERT`, whose pinned declaration is active-low. An
  immediate field-set/read experiment did not advance input latching and is
  not treated as polarity evidence. MAME's four-frame impulse is not copied
  into the local Machine, which already receives explicit key transitions.
- Bank selection, YM synthesis, OKI samples and COPX are not modified. No
  evidence from this batch establishes them as the remaining silence cause.

## Real-set observation

The existing Machine-driver Coin/Start pulse ran once per host width with
the unchanged 20M-instruction containment budget. Both output records match:

| Observation | x64 and x86 |
| --- | --- |
| Executed / guest ticks | 20,000,000 / 324,088,505 |
| Stop / faults | Budget reached; no CPU, COP or Board-control fault |
| Frame | 320 x 240; 74,865 non-black pixels |
| Final frame digest | `ccad0e0867e39b9c` |
| Four frame digests | `d7e4cff81e7dabcd`, `e6d49832ee1363b8`, `4b92fd71b738afb4`, `ccad0e0867e39b9c` |
| Sound latches at checkpoints | `00`, `82`; interrupt not pending |
| PCM copied / nonzero / dropped | 4,096 / 0 / 968,169 |

Before repair S23 sampled command `00,00`; the new second latch proves the
corrected receiving route is exercised by the real guest. Neither this change
nor a frame checksum establishes title, credit acceptance, gameplay or sound
fidelity. PCM remains silent, requiring subsequent evidence-led investigation.

## Qualification record

The existing candidate Product target is rebuilt on both host widths. It
remains a build-tree candidate, not a newly qualified Assets release; T7's
release/configuration and gameplay receiver is still open. No other App,
Shared tree, INI, media or published EXE changes in this batch.

Both stable Ninja graphs rebuilt the affected units and Product candidate.
Complete repository-only units passed **697/697 per architecture**: x64 in
296.07 seconds and x86 in 342.30 seconds. This includes all 12 MyRaidenII
units and 45 MyNES units. Documentation governance and whitespace checks pass.
No new test target, fixture framework or Shared manifest change is needed.

The candidate PE widths are `8664` and `014c`. SHA-256 identities:

- x64: `A329AD41B8495ABF898B5011643CB6712446A57A81D3819803D42EB5B49D3560`.
- x86: `D36413DF132456BD743BF745D72C321B03B416373E977A731492427BAF667392`.

Actual production delta: **+36/-20, net +16** across Board, Sound and its
private header. Existing tests: **+176/-27, net +149** across two files.
There is one new pending Boolean and two independent device-service Booleans;
no new public interface, host state, scheduler, thread or command route.
The initial estimate was production +65/-25 and tests +140/-30; the extra test
lines cover the full byte-lane sweep and wrong-source EOI without new targets.

Actual-diff review confirms the Board owns bus decoding/latches and Sound owns
interrupt service. The tests reject the old stride, wrong pending direction
and repeated delivery; they exercise production CPU execution rather than a
test-only device entry. The broad unit runs consumed the final rebuilt Board
test after its lane sweep was added. Desktop/RDP/physical-audio and other Apps'
external-ROM integration runs are not claimed by this packet.
The owned Lua script and external staging directories have been removed after
the diagnostic processes exited; no protected payload is retained.
