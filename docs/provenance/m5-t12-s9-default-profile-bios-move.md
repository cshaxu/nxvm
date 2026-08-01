# M5 T12 S9 Default-Profile BIOS Move Provenance

- Source foundation: NXVM commit
  `6d6b7d70ab6ed83ab973d27aeea6db88f4e87e4f`, under the root MIT authorization
  in `docs/source-policy.md`.
- Original M1 paths: `src/device/vbios.c` and `src/device/vbios.h`.
- Current VM default-profile firmware destinations:
  `src/vm/profile/default_profile/firmware/vbios.c`
  `7F2C1D324C04707F3B789347E779F2C833BE4E7B5BCD7B8216C7F388794A4634` and
  `src/vm/profile/default_profile/firmware/vbios.h`
  `5832124856F8A2B3A87A97A619DD92ACA08B1BE9842985A4A46995E224C20A01`.
- Changes: preserved BIOS implementation and copyright notices; adjusted only
  direct header paths and CMake source ownership. Existing BIOS calls from
  legacy shared-chip code remain unchanged for later interface decoupling.
- Verification: `docs/verification/m5-t12-s9-default-profile-bios-move.md`.
