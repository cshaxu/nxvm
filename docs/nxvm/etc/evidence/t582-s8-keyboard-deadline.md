# T582 S8: keyboard idle repeat deadline

The Board keyboard-input callback retains its existing signal snapshot and
only asks ticks_until_repeat when repeat_bytes is nonzero. Keyboard signals
set repeat_bytes to zero exactly when scanning is disabled or repeat_count is
zero, the same conditions for which ticks_until_repeat returns invalid state.
The zero-initialized pending/ticks fields remain unchanged in that case; active
repeat queries and BAT, scanning, FIFO and event timing retain their owners.
Production diff: +3/-2 in board-at/kbc.c; no interface or state change.
Existing keyboard, controller, serial cadence and lifecycle tests retain repeat,
FIFO-full and release coverage. Source manifest verifies.

The S7 fixed-work single pairs executed 10,000,000 instructions at identical
79,964,294 guest ticks and PC 0003dbc5: x64 baseline/candidate 21,678/21,084
ms; x86 25,258/21,019 ms. These are early-startup samples, not variance or
interactive realtime qualification.

Cached dual-width full qualification completed: per width 712 unit and 48
integration tests, plus XT 3, AT 5 and DeskPro 6 profile integrations. All
14 EXEs were built and PE-checked; MyNES bytes were unchanged. MyRaidenII
selected-set startup and deployed Console checks passed on both widths. No
gameplay, native RDP or PC110 realtime qualification is asserted.

| Artifact | SHA-256 |
| --- | --- |
| `assets/nxvm/nxvm_0_5_0582_x64.exe` | `94D19BF794293EBAAB81BB89F268CD1F1FEB7DE1571C24C84036E000BA654005` |
| `assets/my5160/my5160_0_5_0582_x64.exe` | `3D1BE46A8B706BC7D840FA978797871C86A9243AD174DF07C94943BFC2D13B5F` |
| `assets/my5170/my5170_0_5_0582_x64.exe` | `5C65907990E993351E4F5BBE340AC662D011F5F9D30608A27E77BD633AA5A5B7` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe` | `4FF1D92B02360A53E525E8F0A24BF61A880530B02435854830F6273274D5953F` |
| `assets/mypc110/mypc110_x64.exe` | `1D90A39CA32FDD62598E86B86019DF144A654194B4640F7622FEDBC14F3E63B2` |
| `assets/myraidenii/myraidenii_0_0_0007_x64.exe` | `00BA0E428EBD6069573E35B9BB5E80C0710B28B073C0719457BC5C92F14717B3` |
| `assets/mynes/mynes_0_0_0044_x64.exe` | `B499CD2583ECDDD1AE01EED882E21D49D20187558E2138858E5A27117CF74FC8` |
| `assets/nxvm/nxvm_0_5_0582_x86.exe` | `6662F1A7F5DC12BCDD2732D0578D0B1EB91158743234816BEEE2E75E7581D14B` |
| `assets/my5160/my5160_0_5_0582_x86.exe` | `CBD8406EA0EBE59ADF339960903B092CA8D5FA42E53F8914FA3DE9E9BE0B511B` |
| `assets/my5170/my5170_0_5_0582_x86.exe` | `54097DAD7488F033EC2B8C9B9798AB886C0B005DB9ED626DF78593F6C1055E5A` |
| `assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe` | `F2B8A5A0C2D09A0F12DAC99AD9908781D0023B396363B40DD146033469C702AB` |
| `assets/mypc110/mypc110_x86.exe` | `4CD2D4DA8A4AB53A591C4224237E5BA70CC2F86E11B0F22BF1E4BB6F074BA2B9` |
| `assets/myraidenii/myraidenii_0_0_0007_x86.exe` | `5261CE97ECD9E4EE2087B4C041E0A2B04CF10E9B2A80F35B1362AC0EA6BF8B38` |
| `assets/mynes/mynes_0_0_0044_x86.exe` | `718617AF2FBF6D49C36AE3BB10043DC3B2383E35440E7A371E503B8BD4B4EA16` |

Qualification log SHA-256: `E14C889A34B9D870713B73BB48F871B310F8206FB1C2DFF0E9F1503FCDF9505D`.

External receiver log SHA-256: `D829B6B9159B4FC435B77EA9664536C7FBF0141BD92E96C5ABA8C6E63833C585`.
