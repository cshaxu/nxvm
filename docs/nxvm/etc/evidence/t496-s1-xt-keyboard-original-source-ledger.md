# T496 S1 IBM 5160 XT Keyboard Original-Source Ledger

`M5:T496:S1:XT-KEYBOARD-SOURCE:OK`

## Method

The locally retained IBM *PC/XT 5155/5160 Technical Reference*, March 1986,
was rendered and visually checked. OCR was used only to locate pages; the
conclusions below were taken from rendered pages 4-22 through 4-33. The older
April 1983 reference was also visually checked for its original 83-key board
description. These sources are scans, so OCR never defines a requirement.

## Normative Findings

| ID | Rendered IBM source finding | Classification |
| --- | --- | --- |
| S1 | The 101/102-key keyboard has a 16-byte FIFO; response codes do not occupy it. | IBM March 1986 rendered 4-24 | Manual L3 functional relation |
| S2 | The keyboard sends 9 serial bits, controls its clock/data open-collector lines, and has specified RTS/CTS and bit timing. | IBM March 1986 rendered 4-32--4-33 | Manual L3 protocol/timing relation |
| S3 | A reset is a minimum 12.5 ms host clock-low action; it causes keyboard reset, BAT and an appropriate result. | IBM March 1986 rendered 4-26 | Manual L3 reset relation |
| S4 | BAT is 300--500 ms after reset acknowledgement; successful completion is AA and failure is FC. | IBM March 1986 rendered 4-25--4-26 | Manual L3 interval/result relation |
| S5 | The selected 101/102-key keyboard scan mapping, make/break and typematic behavior are enumerated by the manual. | IBM March 1986 rendered 4-22--4-31 | Manual L3 functional relation |
| S6 | The board PPI path remains the port/latch/IRQ receiver; the older IBM source supplies the original XT physical board context. | IBM April 1983 rendered 1-5, 1-67; T491 | Manual L3 board relation |

## Explicit Source Gaps

The manual provides intervals, not one exact reset-completion instant. A Core
implementation may therefore retain the full source interval as Manual L3 but
must record its deterministic in-range scheduling choice as macro/L2. The
manual does not require an additional reset-acknowledgement byte; adding one
would be invented behavior.

## Bounded External Cross-Check

Read-only inspection finds a separate XT keyboard device in 86Box and a
model-specific keyboard producer in PCjs. Both corroborate separating keyboard
input from an AT 8042 path, but neither supplies normative timing, BAT bytes or
a replacement source for the gaps above. No external code or data is imported.

## Result

The next lists map the complete documented keyboard relation to one Core owner.
