# T580 S86 — PC110 product and Setup observation

The accepted S85 Window correction is present in the deployed TFT x64 EXE.
S57 and S77 established that two actual firmware-generated Machine frames
contain `BE9E96` as their dominant non-black RGB value. These frames are not
native Window captures and do not close the reported Setup colour discrepancy.

In an isolated no-media RDP run of the released TFT x64 product, the monitor
was paused before the existing F12 binding was sent. Debug showed
`3800:1333 HLT` before input. The normal monitor resumed, F12 down/up messages
were posted, and 120 seconds later the product was paused again. The Debug
console contained a later `3000:8000 JMP 3800:5B00` sample. This supports
guest progression after the posted input but does not independently attribute
every intermediate event to F12. Both Window capture methods produced the
same 464x290 image, entirely black except for 15 white pixels, and the probe
returned zero. The guest did not reach a visible Setup or Error page in this
observation. The log SHA-256 is
`0BD4B890A476CE6210D5B98F604015FC42F8CE8E75AFA9ECE62905EC7393D408`;
the capture SHA-256 is
`7B25CB91A4B3923D3E58CF651F2831332CB2EE2B6AA40BB4CEE585C2126CD415`.

The low-level power diagnostic uses a 65536-instruction Core quantum, whereas
the actual product's Turbo runner uses 4096 instructions with no tick budget
and publishes at its existing host-time cadence. This is a factual difference,
not proof that either cadence causes the observed product-path gap. A further
run with task-owned master/slave overlays recorded six periodic native Window
captures after the same confirmed suspend/F12 sequence. All six had the same
black client image hash above; the probe returned zero. The task-owned master
and slave media hashes were unchanged. This does not establish that the guest
had reached Setup or that the media had been read.
No source or user configuration/media was modified.

Source-path review rules out one proposed stale-generation mechanism: the
PC110 profile selects `DISPLAY_TOPOLOGY_NONE`, so Board does not create a
VADP. Its VADP observation of a null adapter requests capture and does not
claim a reliable generation; the Machine therefore cannot suppress the
F65535 provider based on a stale default VADP generation. The diagnostic's
frame capture does force publication before the same driver `copy_frame`
operation, while the product runs its regular cadence. This is a meaningful
path distinction, not evidence that normal publication is broken.

A subsequent scratch-only instrumented product run records retired instruction
counts and successful frame publications without changing repository source.
It reached the normal HLT at physical 00039333h after 35,586,944 instructions;
the counter then remained fixed until the posted F12. After F12 it resumed
executing, so the earlier black image is not evidence of a permanent HLT. The
prior actual-firmware Setup diagnostic reached its Enter checkpoint after about
130 million retired instructions, whereas the shorter product observations
did not establish that much progress. These are different paths and durations,
not proof that the product eventually reaches Setup.

The longer isolated TFT x64 observation used a scratch-instrumented product
linked from the same production objects with observation-only runner and frame
logging. Its real guest left the first HLT after the ordinary posted F12, and
a 640x480 graphics frame was published at approximately 100 million retired
instructions. The actual RDP Window capture showed the firmware ERROR page
(errors 00173 and 00163); its large background pixel was RGB `BE9E96`, matching
the earlier guest-frame measurement rather than blue. The native client was
464x348, retaining the 4:3 raster ratio. The capture SHA-256 is
`0B814746D453778DC3478972F37B00722A2825389CEEE44268863D972E87DC72`.

An Enter make/break was posted to the owned LibKvmWindow by its process ID.
The next captured Window showed the date/time editor, also pink; its SHA-256
is `002667383427375B066CAC81F5A4EFCB0B1D8350A9E073E659456335F18CACE1`.
A second Enter was posted; the guest subsequently republished startup frames.
This is not yet an independent released-binary TFT reproduction, a full guest
boot, or physical panel evidence. Both task-owned media image hashes remained
unchanged: master `3CD463CC5D05D17F4709DA98F9E18F457BF7CCE279A78BDACC12ABC2DA49598A`
and slave `5149DB391D13CFEAB330016FCF0EDBE6B0D379CBB66A3AED91DBF7684142D52E`.
The separate derived-DSTN x64 observation used a task-owned copy of the
released EXE. The actual Window displayed the same ERROR screenshot hash as
the TFT observation, then normal Enter reached its date/time editor
(`4809E07625482E9A3BFC6E8F6D5A0ABCC7CD585BDFEF1720EBC0DAB3F6CA73DD`).
This is a firmware derived from the reviewed TFT image, not an independently
sourced pristine original BIOS. The observation was deliberately ended after
the required screenshots; its probe is not reported as a completed test.
The task-owned disk image hashes remained equal to those listed above.

These observations establish the actual displayed colour in the connected
RDP session and demonstrate F12/Enter progression; they do not establish a
complete guest boot, physical LCD output, or colour fidelity on a different
RDP client. Startup takes several minutes on this host. No production or
Shared code was changed in S86; the evidence does not support introducing a
Lib colour patch.

The TFT observation log SHA-256 is
`522C976C44C7353AC5D0B22A36973D796CACC91ACBA5FE6AD6E48EB490101893`;
the deliberately stopped derived-DSTN observation log SHA-256 is
`F3B545B67EA50B8D8DB33D80E41637F254EF77A13D923BFA7981F4FBCD48B02E`.
The NXVM documentation governance script passed directly. The CMake target
invocation remained silent and was interrupted; it is not reported as passed.

## Released TFT product through guest boot

A second task-owned x64 run used the actual deployed TFT executable, isolated
master/slave overlay inputs, and ordinary native Window messages. Its first
F12 was sent after the same Debug-confirmed firmware HLT. The product displayed
the same pink ERROR page and date/time editor; two ordinary Enter pairs were
posted after observing the relevant pink pages. After the ensuing POST, a
second F12 was posted. The Window later displayed `Starting PC DOS...`, then
Guest driver output. An additional Enter was posted once while the startup
text remained displayed; successful native posting alone is not proof of Guest
consumption. The screen remained visually unchanged from frame 38 through the
end of the observation; normal pause/Debug reported `0048:023E`, without
establishing the reason for the Guest's current execution.

The observer exited zero and all 60 timed Window captures succeeded; this
means the observation completed, not that the Guest reached its desktop.
The task-owned source images retained their original hashes. Representative
SHA-256 values: ERROR frame `0B814746D453778DC3478972F37B00722A2825389CEEE44268863D972E87DC72`;
PC DOS initial boot frame `B48B0BB7DC56F1AAB73D9EB45223D3432FA313F1819EBB9C7F1C010483177E1E`;
final driver frame `8571B4E7F63BF5CB7BCBDC03802F53175E14B53AC0AB1DEBECD5890E58BB0B70`;
observer log
`BC7C692DFA4CB8C1E6E018E3C003CAFF2EAEF81571072183FBAC86288419CB2E`.
These observations establish a real released-product Guest boot handoff and
DOS driver startup, not a working shell, calibration or Personaware desktop.
The Guest input wait and live product driver path remain to be investigated.
