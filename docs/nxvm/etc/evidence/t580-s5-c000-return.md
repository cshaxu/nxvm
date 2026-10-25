# T580 S5 — TFT C000 return classification

The bounded real-TFT diagnostic now follows the selected C&T video bank until
the first retirement outside C000 after the S4 `C000:6000` checkpoint.  On both
host widths it records the same return:

```text
after-c000=f000:00003f6c bytes=8b c5 5d 89
executed=10999289 resets=1 exceptions=0 c000=0/828865
```

The run subsequently reaches the already-known port-61 wait boundary without
a first Core fault or a new CPU-visible device access class.  It is therefore
not evidence for a new video, storage, panel or Core receiver.  The diagnostic
is a finite return classification only; it does not qualify POST, Setup,
display or guest boot.
