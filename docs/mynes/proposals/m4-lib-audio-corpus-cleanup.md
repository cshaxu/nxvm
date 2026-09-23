# M4 Lib Audio Corpus Cleanup

## Outcome

Make Lib Audio a clean shared corpus: remove its unused platform query endpoint,
hide FIFO and native-playback batch sizing from the public ABI, and define the
fate of accepted PCM smaller than an internal playback batch.

## Contract

`lib_audio_stream_enqueue` accepts ordered PCM into Lib's private FIFO. Full
internal batches are submitted asynchronously. A partial tail is submitted when
the caller explicitly flushes the stream or deactivates it; those operations do
not return until the accepted tail has either reached the native backend or the
stream reports its terminal error. Destruction also submits a sole queued tail,
but cancels an outstanding native wait for a full batch and discards the
remaining FIFO so teardown cannot hang or write after native resources begin
closing. Consumers that require delivery of every accepted finite sound call
`flush` before destruction. `clear` remains the explicit active discard
operation. Batch and capacity values are private Audio implementation policy,
never public consumer ABI.

## Boundaries

Only `src/lib/audio`, its Lib build/dependency declarations, and `test/lib`
change. Lib keeps its Types-and-Base-only dependency boundary. Core/App/Common
and SoftPC source remain unchanged; MyNes only rebuilds and validates their
consumer linkage.

## Acceptance

No production declaration or caller of `audio_stream_platform_query` remains.
Public Audio headers expose neither queue capacity nor playback batch. Tests
prove an accepted sub-batch tail is submitted in order during explicit flush,
deactivation and sole-tail destruction, while clear and native-wait destruction
discard as documented. x64/x86 full regression, component dependency checks,
manifests and rebuilt MyNes artifacts pass.
