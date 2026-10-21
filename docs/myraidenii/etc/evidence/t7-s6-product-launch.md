# M1 T7 S6 — External Product Launch Diagnostic

## Scope and grade

This packet exercises the real MyRaidenII Product executable with the
owner-supplied selected set.  It is **L2** evidence: its runtime observations
are not an inspected primary-manual PDF formula, constant or rule.  It makes no
claim about title, gameplay, protection, sound fidelity, guest input response
or release fitness.

## Reproducible route

`Invoke-SelectedSetStartupProbe.ps1` now has two exclusive modes.  Its existing
Probe mode is unchanged.  Product mode reuses the same archive verifier and
the same twelve Machine-owned runtime roles, then creates one fresh directory
below `O:\assets\raidenii\myraidenii-diagnostics\` containing:

- a copy of the Product executable;
- a lowercase adjacent `myraidenii.ini` with `display=window`,
  `console_control=0`, and only same-directory image filenames; and
- the twelve verified files consumed by the existing Machine transaction.

It launches only that copied executable, requires it to remain live for the
bounded observation, terminates that owned PID, and removes the fresh stage in
`finally`.  It retains no protected input bytes, configuration or executable.
It neither changes the working directory rule nor invents another runtime
loader: Product still resolves the adjacent INI and Machine still resolves all
paths relative to that INI directory.

## Results

The already configured Ninja product targets required no rebuild.  Product
mode reported the following once for each host width:

```text
MYRAIDENII:PRODUCT-LAUNCH:OK pid=<owned pid> seconds=10
```

The x64 and x86 process each survived the full ten-second observation and were
then terminated by the script.  The diagnostic root contained no child after
each run.  The unchanged Probe mode was also rerun on x64 with held Coin 1 and
Start 1; it retained its normal two-million-instruction budget result with no
CPU, COP or control terminal fault.

The Product launch proves only that the adjacent-INI path, image transaction,
Product composition and optional host-audio policy did not immediately exit in
these host conditions.  The available desktop automation inventory did not
expose the native Product window, so no visual Window, KVM host-key delivery,
guest coin/start, title, attract or gameplay assertion was executed.  Those
remain separate external checks.

## Verification

- `ctest -L myraidenii --output-on-failure`: 12/12 x64 and 12/12 x86.
- Existing x64 Probe mode with `--coin-start-held`: normal bounded result.
- Complete repository-only `unit` selection: 652/652 x64 and 652/652 x86.
  The `repository.build-cleanup` row requires a real temporary Windows junction;
  it is denied by the restricted sandbox but passes under the same host's
  authorized build-test execution.  This is a test-host capability distinction,
  not a waived assertion.
- Documentation governance and whitespace/diff checks pass at packet closure.

