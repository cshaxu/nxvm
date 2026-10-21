# M1 T7 S38 — Sprite-Protection Reachability

S38 adds only a resettable, payload-free Board observation for the already
implemented source trigger: completed services, accepted descriptors and
rejected descriptors.  It neither exposes descriptor data/addresses nor
changes trigger, DMA, Video or sprite behavior.

The existing Board smoke proves the observation follows one accepted and one
rejected mapped request.  The final bounded owner-ROM vblank Coin/Start run
then reports the same result on both widths:

```text
instruction budget: 20,000,000
sprite-protection-services=38400
sprite-protection-accepted=37597
sprite-protection-rejected=803
frame-digest=5ca84b4d9994fb4c nonblack=74865
```

The path is therefore active in the selected set, not a disconnected S37
model.  This L2 observation does not prove that every object, blend/shadow
condition, COPX behavior, sprite decode, title screen or gameplay is faithful.
The next receiver must address one finite output-semantic gap rather than
revisiting trigger reachability.  External staging was empty after both runs.
