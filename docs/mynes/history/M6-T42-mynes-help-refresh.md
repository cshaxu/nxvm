# M6 T42 MyNES Help Refresh

## Result

The cooked monitor now consistently identifies the product as `MyNES`, makes
Start and Select separate controls, and describes Escape as `Pause or Resume`.
The owner-cleaned command table formatting is retained; no command grammar or
lifecycle behavior changed.

## Delivery

- Shared configuration advances the two MyNES current-build presets to 0042.
- MyNES publishes `mynes_0_0_0042_x64.exe` and
  `mynes_0_0_0042_x86.exe` in `assets/mynes/`.
- [S1 evidence](../etc/evidence/m6-t42-s1-help-refresh.md) records the
  architecture and full-suite verification.

## Closure

The change set is limited to the owner-cleaned help surface, its assertions,
the MyNES 0042 product declaration and delivery pair. The one required root
preset update is an isolated Shared configuration commit; it changes only the
MyNES target name. No Shared component, NXVM source, external asset, parser or
lifecycle behavior changed.
