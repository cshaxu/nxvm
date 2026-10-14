# T556 S10 RDP Controller Investigation

## Baseline And Boundaries

Production baseline: `7bfefb2fc`, unchanged by this investigation. The owner
reports Window gameplay using a mobile RDP software keyboard: Escape works,
while game controls do not. MyNES registers Escape, not the PC pause chord.

The configured owner-managed TMNT3 cartridge is a local read-only input. It
is not copied, modified or redistributed. No snapshot or INI is modified.
No Shared or MyNES production code has changed. S10 remains open.

## Reproduction

`test/app-mynes/diagnostic/controller_input_probe.c` is an explicitly invoked
owner-ROM probe, not an automatic CTest or a new production path. Its optional
build target is `mynes-controller-input-probe`. Pass the local cartridge path
as its sole argument. The probe runs two fresh machines:

- Hold the existing Enter binding during guest frames 180 through 189.
- At frame 180, pass one carriage-return character through the production KVM
  normalizer and MyNES input adapter. This produces an immediate make/break.

Both executions stop at frame 200 or ten million step calls, with at most
twelve latch records and twenty-four post-poll zero-page writes per execution.
There is no physical audio, host pacing, native UI or unbounded recorder.
The Core trace observes existing bus accesses; no production instrumentation
or ROM-specific branch is introduced. This is not a capture of the phone's
actual native event stream, nor a test of the whole Session/Window route.

Both existing x64/x86 Ninja/ccache trees build and run the probe successfully.
Results agree on both architectures:

- The game latches the controller twice each frame, approximately 387 CPU
  cycles apart within each pair.
- Held Enter gives `08, 08` on the first pair. The game publishes Start in
  its input bytes (`f8` and `54` receive `10`, in its own bit ordering).
- The character pulse gives `08, 00`. The game instead takes its clearing
  path and writes zero to those input bytes.
- Across the twenty measured frames, held input appears in twenty of forty
  latches; the pulse appears in only one. Both runs have twenty close pairs.

Ignored local logs: `build/t556-s10-controller-input-x64.log` and the matching
`x86.log`. These bounded observations contain no cartridge instruction bytes.

## Proven Defect And Remaining Uncertainty

`core_controller_write_strobe()` clears the transient buttons on the first
falling strobe edge. That preserves one read, but not the game's repeated-read
consistency check. This is a demonstrated MyNES short-input compatibility
defect, not merely a missing long-press feature or an absent key mapping.

Escape does not use this path: Session dispatches its hotkey directly to the
lifecycle provider. It need not survive a guest controller sampling cycle.

The test establishes the defect for a character pulse that reaches the adapter.
It does not establish which exact native messages the mobile client sends or
exclude a separate native-input issue. Shift delivery, repeated typing and
actual user gameplay still need validation. The configured cartridge is the
tested case; the owner has not separately confirmed the game's filename.

## Repair Direction For Review

Stop consuming a short press as a side effect of a hardware latch. Give the
MyNES adapter an explicit bounded guest-time pulse lifetime, long enough to
span a complete guest frame; release it through the existing controller input
operations, independently of how many times a game polls. Real held keys retain
their make/break semantics. Pause/reset/source cleanup must not leave a pulse
held. Do not add host sleeps, a ROM-specific poll count or a permanent key latch.

The precise expiration boundary and snapshot/reset interaction must be designed
before implementing this visible input-policy change. Keep snapshot encoding
unchanged and do not add a Lib or Emulator timer/API. Required regression proof
includes repeated polls, eventual release, held-key overlap, frame-boundary
injection and cleanup. Then repeat the owner-ROM comparison and test the actual
Window character path. Continuous mobile movement cannot be promised if the
client does not supply held-key transitions.

## Current Delivery State

Only the diagnostic source/build registration and task documentation changed.
No product EXE requires rebuilding for this investigation-only state. The owner
approved publishing this investigation after moving the probe into Diagnostic.
Its explicit target belongs to `mynes-diagnostic-binaries`, not the integration
aggregate or ordinary CTest. Strict warnings apply as for the existing probe.
This is a bounded investigation delivery, not a production fix or S10 closure.

After relocation, both existing Ninja graphs build the diagnostic with strict
warnings. Both diagnostic executions reproduce the same held/pulse results;
MyNES unit suites pass 45/45 on x64 and 45/45 on x86. Logs are
`build/t556-s10-diagnostic-{x64,x86}-{build,run,unit}.log`.
No desktop/RDP validation was performed by this relocation. No production,
Shared, executable or owner configuration file changed.
