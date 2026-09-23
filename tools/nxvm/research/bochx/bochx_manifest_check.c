#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum manifest_field {
    FIELD_SCHEMA,
    FIELD_BOCHS_ROOT,
    FIELD_BOCHS_HASH,
    FIELD_PATCH_PATH,
    FIELD_PATCH_HASH,
    FIELD_NXVM_COMMIT,
    FIELD_PROBE_ID,
    FIELD_CPU_MODE,
    FIELD_START_PC,
    FIELD_INSTRUCTION_BUDGET,
    FIELD_WALLCLOCK_MS,
    FIELD_NO_PROGRESS_LIMIT,
    FIELD_TRACE_RING_EVENTS,
    FIELD_COMPARISON_MASK,
    FIELD_CLEANUP_OWNER,
    FIELD_COUNT
};

typedef struct manifest {
    char values[FIELD_COUNT][512];
    unsigned present[FIELD_COUNT];
} manifest;

static const char *const field_names[FIELD_COUNT] = {
    "schema",
    "bochs_source_root",
    "bochs_archive_sha256",
    "bochx_patch_path",
    "bochx_patch_sha256",
    "nxvm_commit",
    "probe_id",
    "cpu_mode",
    "start_linear_pc",
    "instruction_budget",
    "wallclock_ms",
    "no_progress_limit",
    "trace_ring_events",
    "comparison_mask",
    "cleanup_owner"
};

static int field_index(const char *key)
{
    int index;

    for (index = 0; index < FIELD_COUNT; ++index) {
        if (strcmp(key, field_names[index]) == 0) {
            return index;
        }
    }
    return -1;
}

static void trim(char *text)
{
    char *start = text;
    char *end;

    while (*start != '\0' && isspace((unsigned char)*start)) {
        ++start;
    }
    if (start != text) {
        memmove(text, start, strlen(start) + 1u);
    }
    end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) {
        --end;
    }
    *end = '\0';
}

static int is_hex(const char *value, size_t expected_length)
{
    size_t index;

    if (strlen(value) != expected_length) {
        return 0;
    }
    for (index = 0u; index < expected_length; ++index) {
        if (!isxdigit((unsigned char)value[index])) {
            return 0;
        }
    }
    return 1;
}

static int is_positive_decimal(const char *value)
{
    char *end = NULL;
    unsigned long parsed;

    if (*value == '\0') {
        return 0;
    }
    parsed = strtoul(value, &end, 10);
    return parsed > 0ul && end != value && *end == '\0';
}

static int file_contains(const char *path, const char *needle)
{
    char buffer[1024];
    FILE *file = fopen(path, "rb");
    size_t count;

    if (file == NULL) {
        return 0;
    }
    count = fread(buffer, 1u, sizeof(buffer) - 1u, file);
    fclose(file);
    buffer[count] = '\0';
    return strstr(buffer, needle) != NULL;
}

static int validate(const manifest *input)
{
    char version_path[1024];
    int index;

    for (index = 0; index < FIELD_COUNT; ++index) {
        if (!input->present[index] || input->values[index][0] == '\0') {
            fprintf(stderr, "missing required field: %s\n", field_names[index]);
            return 0;
        }
    }
    if (strcmp(input->values[FIELD_SCHEMA],
               "ntvdm64.bochx-experiment.v1") != 0 ||
        strcmp(input->values[FIELD_CPU_MODE], "real_mode") != 0 ||
        !is_hex(input->values[FIELD_BOCHS_HASH], 64u) ||
        !is_hex(input->values[FIELD_PATCH_HASH], 64u) ||
        !is_hex(input->values[FIELD_NXVM_COMMIT], 40u) ||
        !is_positive_decimal(input->values[FIELD_INSTRUCTION_BUDGET]) ||
        !is_positive_decimal(input->values[FIELD_WALLCLOCK_MS]) ||
        !is_positive_decimal(input->values[FIELD_NO_PROGRESS_LIMIT]) ||
        !is_positive_decimal(input->values[FIELD_TRACE_RING_EVENTS])) {
        fprintf(stderr, "invalid manifest identity or budget field\n");
        return 0;
    }
    if (snprintf(version_path, sizeof(version_path), "%s/bxversion.h",
                 input->values[FIELD_BOCHS_ROOT]) >= (int)sizeof(version_path) ||
        !file_contains(version_path, "#define VER_STRING \"2.6\"") ||
        !file_contains(input->values[FIELD_PATCH_PATH], "PK")) {
        fprintf(stderr, "local Bochs 2.6 root or Bochx patch is unavailable\n");
        return 0;
    }
    return 1;
}

int main(int argc, char **argv)
{
    char line[1024];
    manifest input = { { { 0 } }, { 0u } };
    FILE *file;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <local-manifest>\n", argv[0]);
        return 2;
    }
    file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "cannot read local manifest\n");
        return 2;
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        char *equals;
        int index;

        trim(line);
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }
        equals = strchr(line, '=');
        if (equals == NULL) {
            fclose(file);
            fprintf(stderr, "invalid manifest line\n");
            return 2;
        }
        *equals = '\0';
        trim(line);
        trim(equals + 1);
        index = field_index(line);
        if (index < 0 || input.present[index] ||
            strlen(equals + 1) >= sizeof(input.values[index])) {
            fclose(file);
            fprintf(stderr, "invalid or duplicate manifest field\n");
            return 2;
        }
        strcpy(input.values[index], equals + 1);
        input.present[index] = 1u;
    }
    fclose(file);
    if (!validate(&input)) {
        return 1;
    }
    puts("M5:T1:S2:BOCHX-MANIFEST:OK");
    return 0;
}
