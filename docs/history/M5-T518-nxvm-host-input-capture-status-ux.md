# M5 T518: NXVM Host-Input, Capture And Status UX

The [active proposal](../proposals/m5-nxvm-host-input-capture-status-ux.md)
defines the owner-approved UX contract. This record is updated only with
accepted subtask outcomes and final closure evidence.

## Task Record

| Subtask | Scope | State |
| --- | --- | --- |
| S1 | Converge one Console lease, console text-Window-text transitions, multi-Window lifecycle and Window-close pause. | Accepted: P1 `8be4223c` |
| S2 | Converge host input and product action classification. | Planned |
| S3 | Publish one lifecycle observation to platform presentation. | Planned |
| S4 | Implement native Window capture, cursor and title behavior. | Planned |
| S5 | Run full unit closure, manual host proof and developer artifact delivery. | Planned |

## Accepted Outcomes

### S1

- The Console backend remains the sole holder of the process Console lease.
  Its graphics transition creates the shared per-session Window presenter and
  does not join, finalize or restart the Console runner. The reverse transition
  removes that presenter and restores the same Console route.
- Fixed `window` sessions and Console-session graphics use the one native
  Window presenter. Closing a Window reports the existing bounded pause event;
  it neither resets Core nor affects another session.
- The obsolete presentation-transition runner yield and every `auto` display
  spelling were removed. Session listing now reports the immutable selected
  policy rather than a temporary graphics presenter.
- Repository-only focused platform proof and the complete unit suite pass:
  302/302 in 19.13 seconds with `ctest --test-dir build/t518-s1 -L unit -j 4
  --output-on-failure`. Documentation governance and actual-diff review pass.
