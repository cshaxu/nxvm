#include "type.h"



#include "core/platform/presentation_mailbox_interface.h"

C_INT main(C_VOID)
{
    core_platform_presentation_mailbox first;
    core_platform_presentation_mailbox second;
    core_platform_display_frame published = {0};
    core_platform_display_frame captured = {0};

    core_platform_presentation_mailbox_initialize(&first);
    core_platform_presentation_mailbox_initialize(&second);
    published.columns = 80u;
    published.rows = 25u;
    published.characters[0] = 'A';
    published.generation = 1u;
    if (core_platform_presentation_mailbox_publish(&first, &published) !=
        TYPE_STATUS_OK) return 1;
    published.characters[0] = 'B';
    published.generation = 2u;
    if (core_platform_presentation_mailbox_publish(&second, &published) !=
        TYPE_STATUS_OK || core_platform_presentation_mailbox_capture(&first,
        &captured) != TYPE_STATUS_OK) return 1;
    if (captured.characters[0] != 'A' || captured.generation != 1u) return 1;
    if (core_platform_presentation_mailbox_capture(&second, &captured) !=
        TYPE_STATUS_OK) return 1;
    if (captured.characters[0] != 'B' || captured.generation != 2u) return 1;
    core_platform_presentation_mailbox_finalize(&first);
    captured.characters[0] = 'C';
    captured.generation = 3u;
    if (core_platform_presentation_mailbox_capture(&first, &captured) !=
        TYPE_STATUS_INVALID_STATE || core_platform_presentation_mailbox_publish(
        &first, &published) != TYPE_STATUS_INVALID_STATE ||
        captured.characters[0] != 'C' || captured.generation != 3u) return 1;
    core_platform_presentation_mailbox_finalize(&second);
    puts("M5:T250:S2:CORE-PLATFORM-PRESENTATION:OK");
    puts("M5:T443:S1:MAILBOX-INITIALIZATION-AND-OUTCOMES:OK");
    return 0;
}
