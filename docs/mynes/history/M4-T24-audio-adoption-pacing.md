# M4 T24: Audio Adoption And Pacing

T24 closes the shared-audio prerequisite without changing SoftPC. MyNes now
owns a neutral Lib copied-PCM stream for 48 kHz mono signed-16 samples. Its
bounded public queue has activation, flush and teardown semantics independent
of NES hardware; the Win32 backend uses four persistent asynchronous waveOut
buffers. The component is self-contained so later upstream recovery can take
the same interface without a cross-repository dependency.

Core Driver creates the stream, drains its existing APU FIFO only into available
Lib capacity, activates it with the Common heartbeat and flushes it on pause or
stop. Driver pacing compares guest cycles against the RP2A03 clock and uses a
bounded host wait; waiting never advances or drops guest work. The APU remains
the owner of generated PCM and all emulation time.

The independent stream contract, Lib manifest/DAG checks, Core/App composition
and focused M4 suite pass on x64 and x86. Native Window and text-only Console
smoke paths pass using project-authored fixtures. A 20,000-slice observation of
each owner-local named mapper input showed zero dropped PCM samples after the
asynchronous backend change. T24 proves PCM delivery and bounded pacing; it
does not claim that a human heard audio or that either named game reached a
controllable title/gameplay state. Those claims transfer to T25.
