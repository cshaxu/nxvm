# M5 T565 S5 — PC110 configuration-route binding

## Scope

`app-mypc110` now owns the first concrete PC110 board adapter. It installs
only the source-qualified SCAMP (`74h`/`76h`) and EC/ED (`ECh`/`EDh`) index/data
routes and delegates all register protocol state to
`x86-core/chips/vl82c420`. The adapter owns neither a Core executor nor a
clock, board profile, controller mirror or firmware behavior.

Block2 (`24h`/`25h`) is intentionally absent. Its S3 access gate is known, but
the guest-visible route that produces its four enabling reads is not yet
qualified. Installing a permanently inaccessible pair would make BIOS behavior
look supported while preventing it; waiting preserves one truthful owner.

## Contract

- `mypc110_config_install_routes()` publishes all four claimed routes
  atomically through the existing Core port-route operation.
- The VL82C420 chip is the route owner token. The App retains it until route
  removal or Core destruction; no second state object is introduced.
- SCAMP uses S3's documented seven-bit selection; EC/ED remains its distinct
  bank. Index ports are write-only because no source-backed read contract is
  claimed.
- The App boundary gate rejects peer-App includes and the excluded PC/AT
  preparation helper. The common test-support gate now recognizes the new App
  name, without granting it peer support access.

## Direct verification

`mypc110.unit/config-route` creates the normal neutral Core machine, installs
the App route adapter, freezes/resets the machine, then proves:

1. SCAMP `80h` and `00h` select the same register;
2. EC/ED `0Ch` reads back its own value;
3. a duplicate publication is rejected rather than replacing a route;
4. Block2 remains unclaimed; and
5. removing the chip-owned routes returns the published SCAMP data port to
   unsupported status.

Both `mingw-gcc-x64-release-ccache` and
`mingw-gcc-x86-release-ccache` configured, built and passed that test. The
existing configure-time App-boundary checks also pass on both widths. This is a
direct construction proof only: it does not prove reset defaults, real firmware
POST, desktop behavior, external TFT/DSTN assets or a bootable product.
