# T7 S62: Selected-Set Sound Publication Audit

## Local Route

The local sound route has one ownership chain:

```text
Board transport → Sound CPU/device state → Board PCM copy
→ Machine staging/drain → Lib PCM accepted-prefix enqueue
```

Board owns the command latches and interrupt state. The Sound owner alone
executes the sound CPU, advances YM/OKI state and generates bounded PCM. The
Machine bridge alone preserves partial accepted prefixes and drains Board PCM
into the Lib stream. No alternate mixer, host-synthesis path, duplicate queue
or Product-side audio route exists.

## Evidence And Disposition

Existing Sound unit coverage exercises transport service, device-map writes,
timer interrupts, bounded PCM publication, OKI selection/stop behavior and
ADPCM sign handling. Existing Machine coverage exercises partial enqueue,
failed clear and no-host-stream disposal. The selected-set safe aggregates
from S30/S31 and S60 establish active transport/device traffic but do not
establish that the selected program has entered a state that requests a
playable voice.

Consequently, a silent bounded startup sample is not evidence that the local
transport, PCM bridge or host stream needs a repair. Rewriting any one of
those owners would be speculative and would create a second explanation for a
state-selection question the audit cannot answer.

## Source Grade

The YM and MSM device manuals remain the primary component sources recorded in
the research index. External emulator implementations remain L2 behavioral
references only. This audit imports neither code nor tables and records no
guest command, media or trace data.

## Transfer

The current high-priority Board gap is the unresolved COPX-D2 contract. It
can affect selected-program game-state progression before a sample-selection
state is reached. A successor must first establish its finite request/index/
result contract from permitted L3 or independently implemented L2 evidence;
it may not add an unused image role, generic lookup fallback or guessed audio
trigger.

## Verification

This is a static ownership audit using the existing Sound, Board and Machine
units plus the already-recorded safe selected-set aggregates. Documentation
governance and whitespace checks pass. No source, test, build, external asset
or artifact changed.
