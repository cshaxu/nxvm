# M1 T7 S112 — Selected-Set Gameplay-Stall Investigation

## Scope

This is a read-only diagnosis of the owner-reported Window symptom: the
selected RAIDEN II set appears to remain at its startup/title presentation
instead of entering play.  It changes no production, test, Shared, ROM or
media file.

## Reproduction and route evidence

The deployed x64 product's adjacent `myraidenii.ini` resolves all twelve
required image roles to existing external files.  The existing staged Product
Console smoke completed successfully:

```text
MYRAIDENII:SELECTED-SET-PRODUCT-CONSOLE:OK
```

That smoke proves the real Product's adjacent-INI loading and its
`start → pause → resume → stop → exit` lifecycle route.  It does not inject
guest keys.

The existing selected-set probe was then run once per host width with its
ordinary Machine-driver KVM path and its existing VBlank-relative Coin 1 /
Start 1 pulse:

```text
coin-press-at=300 coin-release-at=330
start-press-at=360 start-release-at=390
coin-pressed=1 coin-released=1 start-pressed=1 start-released=1
executed=8000000 ticks=133667500 video-vblanks=463
input-player-reads=430 input-system-reads=860 input-coin-reads=1508
coin-window-observed=1 start-window-observed=1
board-provider-failure=0 cop-provider-failure=0
```

The complete x64 and x86 records were identical, including the final frame
digest and the input-read counts.  `reason=1` is the probe's normal bounded
instruction-budget stop, not a guest fault.  The wrapper staged its images
only below the approved external diagnostics root and removed every temporary
child.

## Diagnosis

There is no reproduced CPU, Board, COP, video, sound, Machine-FIFO or Product
lifecycle stall in this range.  The source route has one owner at every step:

```text
Window key record → Lib Base/KVM normalized key → Emulator Session/Machine FIFO
→ MyRaidenII Machine driver → Board active-low input → guest input registers
```

Top-row `5` and `1` are neutral ASCII keys, respectively mapped to Coin 1 and
Start 1.  The normalizer also converts a character-only `5`/`1` stream into
those same neutral keys.  Product reserves only Escape as its host pause
hotkey, so it does not intercept either game key.

The short 300/360-VBlank pulse proves only delivery and guest polling.  It is
too early to prove title acceptance.  The Board's nominal raster is 8 MHz,
512 pixels by 282 lines on a 16 MHz guest axis, or about 55.4 VBlank events
per nominal second.  Existing T7 S103 L2 evidence used a later, one-time
sequence at VBlanks 1200/1230 (Coin) and 1260/1290 (Start): it observed
`CREDIT 1`, the runway introduction, then a one-player HUD, with no Board or
COP provider failure.  This places the first demonstrated accepting input
window about 22 seconds after reset, not at the first title picture.

Accordingly, a static-looking startup/title screen by itself is not evidence
that the guest has frozen.  A valid manual sequence is: focus the Window,
allow the initial boot/attract initialization to finish, press top-row `5`,
then press top-row `1`.  Arrow keys and `Z`/`X` control Player 1 after the
game start; Escape is host pause/resume.

## Remaining boundary

This host's desktop automation inventory exposed no native Product Window, so
it cannot truthfully certify physical or RDP keyboard focus/delivery.  The
current repository deliberately has no second test-only Product input entry;
adding one merely to simulate Window messages would create the path the
architecture forbids.  The next evidence, if the symptom persists after the
timed key sequence above, must be an actual focused Window/RDP observation,
not a guessed Core, COP or input repair.

## Disposition

No source repair is justified by the evidence.  This S has ruled out the
claimed emulator deadlock for the selected set and isolated the remaining
uncertainty to host Window/RDP focus and the timing of manual Coin/Start
input.  This is L2 runtime evidence; it is not a claim of full gameplay,
audio, stage, boss or release qualification.
