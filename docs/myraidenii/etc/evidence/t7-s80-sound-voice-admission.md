# T7 S80 — Sound voice admission and selected-set PCM

The existing local OKI receiver restarted an active voice when a second start
command selected it. The independently inspected MAME `mame0289` reference,
`src/devices/sound/okim6295.cpp`, instead starts only selected inactive voices.
That finite behavioral observation is L2; no reference source or table was
copied. The local receiver now leaves a busy voice untouched and still starts
other selected inactive voices. No shared component, API or event path changed.

The existing synthetic sound unit now issues two consecutive selections with
one busy and one free voice, verifies two total starts, then stops and restarts
the first voice and verifies three total starts and both status masks. With the
old receiver the extended x64 unit failed; after the local change the unit
passes on x64 and x86. This is a chip-command regression, not proof the chosen
game requests a busy voice.

The full repository unit label passed 698/698 on each host width. At that
point the sound unit and Product executable were newly linked, but five other
MyRaidenII unit executables still needed relinking against the changed static
Core. They were then relinked, and all 13 MyRaidenII unit tests passed again
on each width. The unrelated product and Shared test executables were not
rebuilt or changed.

A 60-million-instruction x64 selected-set run of the pre-change probe with
the existing vblank Coin/Start schedule completed with no reported Board/COP
provider fault. It generated 2,820,700 nonzero PCM frames and observed voice
starts on both OKI devices (5 and 193 respectively). This disproves the idea
that the whole longer run is silent; it does not establish audio fidelity,
audibility through the host, or the effect of the busy-voice repair. The probe
uses only bounded external owner data and its staging wrapper removes it.

After relinking, the x64 probe also completed the same 60-million-instruction
schedule without a provider failure. Its final frame digest and retained PCM
digest matched the pre-change run. This selected interval therefore does not
demonstrate a game-visible busy-voice correction; the synthetic regression is
the evidence for that specific behavior. The rebuilt x86 probe completed the
same schedule with the same reported frame, PCM and sound-operation aggregates.

The YM path remains the documented L1 diagnostic waveform. The actual
YM2151 music model, desktop/audio output,
gameplay qualification and suspended pause-fence issue remain open under T7.
