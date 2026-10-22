# T7 S84 — YM Timer A CSM Key Source

The existing YM receiver did not produce any key event when Timer A elapsed
with CSM enabled. The independently inspected L2 reference is 86Box commit
`4fef696a4eead1d55a28d6ac0e5bd2864e5454da`, files
`src/sound/ymfm/ymfm_opm.h` and `src/sound/ymfm/ymfm_fm.ipp`: register 14h
bit 7 enables CSM; Timer A expiration triggers all four operators of all
channels; CSM is a temporary key source combined with direct key-on. No source
or table was imported. These are behavior facts from an emulator, not a claim
of original-chip accuracy.

The local implementation reuses the existing timer, direct-key transition and
PCM mixer. It records pending and active CSM state locally, combines the CSM
and direct keys, and does not release a directly held operator when the CSM
source expires. Reset clears both fields through the existing YM reset.
The pending event is consumed at PCM sample boundaries; rapid timer expirations
can coalesce and do not model OPM internal clock duration. This timing and the
underlying waveform/envelope remain L1 approximations. A bounded earlier real
game run observed no CSM enable; that does not prove the full game never uses it.

The existing Sound unit now covers CSM across all eight channels, disabled CSM,
disabled Timer A, and overlap with a directly held operator. On both x64 and
x86 all 13 MyRaidenII unit tests pass after rebuilding Sound and Product. This
does not establish desktop playback, complete musical fidelity, or gameplay.
The existing key-off test was also extended to stop CSM Timer A and verify the
resulting sound decays; it passes on both host widths. Reset uses the same
whole-YM clearing, and the CSM unit now resets an active CSM setup and checks
that the replayed PCM matches the initial run on every channel, on both widths.
Suppressing the Timer A CSM trigger in a temporary x64 mutation makes the Sound
unit fail; the mutation was removed, affected targets were rebuilt, and the
unit passed again. No mutation or generated test media is retained.

Both rebuilt selected-set probes completed 20 million instructions on the
existing real-ROM route with identical frame digest `ac25ef796f90ea94` and PCM
digest `1067e196557f7455`, and reported no Board/COP provider failure or OKI
rejection. This is a bounded regression only; the selected trace is not a CSM
or audible output qualification. Temporary ROM staging was removed by the
existing wrapper. No Shared source, test or API was changed.
