# T498 S1 Supported DOS Media Matrix

`M5:T498:S1:MEDIA-MATRIX:OK`

| Profile | CPU | Format | Current disposition / S2 input need |
| --- | --- | --- | --- |
| IBM 5160 | 8088 | 360 KB | Existing T496 DOS terminal evidence; replay with its lawful XT BYOB firmware pair. |
| IBM 5170 Model 339 | 80286 | 360 KB | DOS 5 installer-ready accepted in T497 S5. |
| IBM 5170 Model 339 | 80286 | 1.2 MB | Native geometry accepted; requires a 286-compatible 1.2-MB boot image. Local DOS 6.22 is 386-only. |
| DeskPro 386 Model 40 | 80386 | 1.2 MB | Requires its lawful external BYOB firmware pair and boot image; no inherited 5170 result. |
| default-at | 8086 | 360 KB | DOS 5 installer-ready observed. |
| default-at | 80186 | 720 KB | DOS 3.21 date-input observed. |
| default-at | 80286 | 360 KB | DOS 5 installer-ready observed. |
| default-at | 80386 | 1.2 MB | DOS 6.22 prompt observed. |
| default-at | 80386 | 1.44 MB | DOS 6.0 prompt observed. |

All other profile/format combinations are rejected by the selected physical
profile contract, not treated as missing tests. The five CPU families are
represented by the accepted rows above. No image path or bytes are retained.
