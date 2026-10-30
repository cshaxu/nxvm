# T582 S26 — Personaware menu timing without screenshot output

An x64 task-owned product-path probe used private copies of the same PC110
disk and CMOS inputs as the earlier Game investigation. It polled the
published frame once per host second, without writing screenshots. Two pixel
values selected from the previously inspected Personaware menu frame first
matched at sample 216; the process exited successfully after 220.64 seconds,
and its final guest RTC reported 00:00:20. The log SHA-256 is
`BF354FD6EECC95910D127A403D256ECB18DA9AAD95389E2986E0243F532FBD86`.

The earlier Game probe sent navigation at fixed samples 26–30, so its total
runtime includes intentional waiting and cannot be treated as startup time.
This new observation shows that the long wait is not explained solely by
writing earlier PPM screenshots. It is not proof that the complete menu was
ready at sample 216: two pixels are only a screen feature, not a complete
frame comparison. The probe still copies frames, logs machine quanta, and
bypasses native Window and RDP. Its host/guest time ratio is approximate and
does not establish desktop responsiveness or gameplay qualification. No
production code, owner media, or published executable changed.
