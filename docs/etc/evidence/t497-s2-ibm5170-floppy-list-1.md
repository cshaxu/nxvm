# T497 S2 IBM 5170 Floppy Function And Timing List 1

`M5:T497:S2:IBM5170-FLOPPY-LIST-1:OK`

| ID | Guest-visible requirement | Primary basis | Level | Required disposition |
| --- | --- | --- | --- | --- |
| F1 | The frozen physical Drive A declaration reports high-capacity 96-TPI type `02h`. | IBM 5170 TR CMOS Diskette Drive Type Byte table. | Manual L3 | Descriptor selects `0x20`; it is not derived from mounted media. |
| F2 | Default mounted media for the selected physical drive is 1.2 MB, 80 cylinders, 2 heads, 15 sectors per track, 512-byte sectors. | IBM high-capacity media compatibility statement plus the existing project geometry definition. | Manual L3 for 1.2 MB selection; existing logical-media geometry. | Session selects the existing 1.2 MB FDD geometry. |
| F3 | A user may explicitly mount compatible 360 KB media, 40 cylinders, 2 heads, 9 sectors per track, 512-byte sectors. | IBM compatibility statement; existing project geometry definition. | Manual L3 for compatibility; existing logical-media geometry. | Session selects existing 360 KB geometry without changing CMOS. |
| F4 | 720 KB and 1.44 MB are not accepted Model-339 requests. | IBM selected 1984 drive table has no such physical configuration. | Manual L3 negative profile boundary. | Session/provider rejects the request. |
| F5 | An absent format request means native 1.2 MB, not enum-zero 1.44 MB. | F1--F2. | Manual L3. | Explicit Model-339 selection is required before FDD construction. |
| F6 | A mounted 360 KB image does not rewrite the immutable physical Drive A CMOS type. | F1 versus F3 are distinct facts. | Manual L3 ownership boundary. | Descriptor stays `0x20`; FDD alone owns current geometry. |
| F7 | The FDC, its IRQ6/DMA2 wiring, command timing and result protocol do not change with this profile/media selection. | Existing completed 8272A/AT FDC contracts; IBM drive choice does not make a second controller. | Retained L3 contract; no new timing claim. | No FDC code change. |
| F8 | Pulse-by-pulse double-step, spindle/rotation, density-sensor and ready timing are not implemented by a logical geometry request. | IBM compatibility statement does not establish a complete current physical-drive timing receiver. | Explicit non-claim. | No fabricated deadline or timing constant. |

The complete S2 universe is F1--F8. S3 maps every row to current owners and
one cohesive repair batch; it must not add a profile-side controller or
duplicate media state.
