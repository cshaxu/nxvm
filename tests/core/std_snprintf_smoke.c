#include "type.h"

C_INT main(C_VOID)
{
    C_CHAR exact[5] = {0};
    C_CHAR truncated[4] = {0};
    C_CHAR failed[4] = {'x', 'x', 'x', '\0'};
    C_CHAR appended[5] = {0};
    C_CHAR *cursor = appended;
    STD_SIZE_T remaining = sizeof(appended);
    C_INT result;

    result = STD_SNPRINTF(exact, sizeof(exact), "%s", "abcd");
    if (result != 4 || STD_STRCMP(exact, "abcd") != 0) return 1;

    result = STD_SNPRINTF(truncated, sizeof(truncated), "%s", "abcd");
    if (result != 4 || STD_STRCMP(truncated, "abc") != 0) return 1;

    result = STD_SNPRINTF(STD_NULL, 0u, "%s", "abcd");
    if (result != 4) return 1;

    result = STD_SNPRINTF(failed, sizeof(failed), STD_NULL);
    if (result >= 0 || failed[0] != '\0') return 1;

    result = STD_SNPRINTF_APPEND(&cursor, &remaining, "%s", "ab");
    if (result != 2 || cursor != appended + 2 || remaining != 3u) return 1;

    result = STD_SNPRINTF_APPEND(&cursor, &remaining, "%s", "cde");
    if (result != 3 || cursor != appended + 2 || remaining != 3u ||
        STD_STRCMP(appended, "abcd") != 0) return 1;

    puts("M5:T279:S3:FORMAT:OK");
    return 0;
}
