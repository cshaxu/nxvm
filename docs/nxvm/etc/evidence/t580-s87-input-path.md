# T580 S87 — Product guest-input boundary

The real released TFT x64 product reached PC DOS driver output under the
actual RDP Window in S86. The previous machine-only diagnostic also reached
Personaware after keyboard input and four inking-pad events; its captured
post-key 640x475 frame asks for the upper-left pad corner.

An initial scratch-instrumented product run found the same recorded `INT 16h`
exception at 1475:0735 after 230,193,750 instructions and sent a Window Enter.
This is the last recorded exception, not proof that the live instruction was
still polling when the event arrived. The final PC sampling remained 0012F63E
while instructions continued. The final debug collection was interrupted by
the probe's monitor-input timing and is not accepted. Probe and runner logs
are SHA-256 `BDE358B0F947643EE570F861F1A0F84E35D20A44E86078FED234D8503169EFB3`
and `27123D172D1433ED1F41518791394D8DF6B726AFDD0241BB1EE8923FE39989D5`.

A second scratch product run enqueued four relative mouse press/release pairs
through `emulator_machine_enqueue_input`; all eight calls returned success.
However the Window frame hash remained
`8571B4E7F63BF5CB7BCBDC03802F53175E14B53AC0AB1DEBECD5890E58BB0B70`.
The experiment did not check whether inking was enabled before injection, and
its colour-only Setup detector incorrectly sent the second Enter while the
first ERROR page was still visible. A subsequent actual date confirmation and
HLT-observed F12 were sent normally. This run proves queue acceptance only;
it cannot prove input consumption or a product defect. It does not exercise
native Window mouse capture. Event log SHA-256 is
`AC64A9812A8ACEDE10CA616455CA6989B205197F33C384FB55EF8CDB39F7B17B`.
The isolated master/slave input image hashes remained unchanged.

The next bounded observation must distinguish the two Setup screens, wake only
a confirmed second HLT, and check the existing inking control register before
each input, then observe the resulting Guest frame. No production or Shared
code is modified to obtain the evidence.
