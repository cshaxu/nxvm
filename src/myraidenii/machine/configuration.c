#include "myraidenii/machine/configuration_interface.h"

#include "lib/storage/file_interface.h"
#include "myraidenii/core/board_interface.h"
#include "myraidenii/core/sound.h"
#include "myraidenii/machine/image_hash.h"

typedef struct myraidenii_machine_image_identity {
    lib_size bytes;
    const char *sha256;
} myraidenii_machine_image_identity;

static const myraidenii_machine_image_identity myraidenii_machine_image_identities[] = {
    {524288u, "90bb05525c1529b97c359deb47305e8e25ee9de8d7c951b0be0b5587c89354ec"},
    {524288u, "c0a6e9a660843851f1cfbcfa84977f82f69a2fd7a4b28b7561c692389d4577b7"},
    {65536u, "53d1070d7de94bee906ec9ded0a5a2417cab881f802ea5bb718133276fbbfd67"},
    {131072u, "3d749b4b6d67134c2181a4ce28d6d53b5ac8ca8ba7b69e1b14688107726a6fc8"},
    {2097152u, "fc5f161091571a3237511205f5e10a3260a028e45ec68bb95363c0d1f67d372f"},
    {2097152u, "aa08fb017daa6d5cead69fecb4f63c981d9a539e07d295bddac23298e4d90042"},
    {2097152u, "34db21461927d33e85b5bdd309ed52624b32e003c4f2edf99f090508c4e11ed4"},
    {2097152u, "bf95ed2e9e7c6875d91f708bb8c417c3a28ed0f41cf86a86d3d010da0ccc469c"},
    {2097152u, "865a3cd7b63ad743a1e475b388c0cd528539bdffb986c0534b8d7be0f45ef550"},
    {2097152u, "ba3e2a38886caab102f4d93c690fcb3c3ac4fab2e6dce238a5261aca29a3d51a"},
    {262144u, "19dff9d74f907836238d0765cc5f45d77c915a99698e857426e8518953452aa4"},
    {262144u, "fcf0b840286763ca2be60bf5231b3d5dc03a569c41bd0ac82d257bb582995c88"}
};

struct myraidenii_machine_images {
    void *bytes[MYRAIDENII_MACHINE_IMAGE_COUNT];
    myraidenii_core_board_graphics_images graphics;
};

static lib_bool myraidenii_machine_path_absolute(const char *path)
{
    return path[0] == '/' || path[0] == '\\' || (path[0] != '\0' && path[1] == ':');
}

static lib_status myraidenii_machine_image_path(const char *base, const char *path,
    char output[1024])
{
    lib_size base_length, path_length;
    if (base == LIB_NULL || path == LIB_NULL || path[0] == '\0')
        return LIB_STATUS_INVALID_ARGUMENT;
    if (myraidenii_machine_path_absolute(path)) {
        if (lib_text_length(path) >= 1024u) return LIB_STATUS_LIMIT_EXCEEDED;
        lib_memory_copy(output, path, lib_text_length(path) + 1u);
        return LIB_STATUS_OK;
    }
    base_length=lib_text_length(base); path_length=lib_text_length(path);
    if (base_length == 0u || base_length + path_length + 2u > 1024u)
        return LIB_STATUS_LIMIT_EXCEEDED;
    lib_memory_copy(output, base, base_length);
    if (output[base_length - 1u] != '/' && output[base_length - 1u] != '\\')
        output[base_length++] = '\\';
    lib_memory_copy(output + base_length, path, path_length + 1u);
    return LIB_STATUS_OK;
}

static lib_bool myraidenii_machine_image_digest_matches(const lib_u8 digest[32], const char *hex)
{
    static const char digits[] = "0123456789abcdef";
    for (lib_size index = 0u; index < 32u; ++index) {
        if (hex[index * 2u] != digits[digest[index] >> 4u] ||
            hex[index * 2u + 1u] != digits[digest[index] & 15u])
            return LIB_FALSE;
    }
    return hex[64u] == '\0';
}

void myraidenii_machine_images_destroy(myraidenii_machine_images *images)
{
    if (images == LIB_NULL) return;
    for (lib_size index = 0u; index < MYRAIDENII_MACHINE_IMAGE_COUNT; ++index)
        lib_release(images->bytes[index]);
    lib_release(images);
}

lib_status myraidenii_machine_images_load(const char *base_directory,
    const char *const image_paths[MYRAIDENII_MACHINE_IMAGE_COUNT], myraidenii_machine_images **out_images)
{
    myraidenii_machine_images *images;
    if (out_images == LIB_NULL || base_directory == LIB_NULL || image_paths == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_images = LIB_NULL;
    images = lib_allocate_zero(1u, sizeof(*images));
    if (images == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    for (lib_size index = 0u; index < MYRAIDENII_MACHINE_IMAGE_COUNT; ++index) {
        char path[1024];
        lib_size count = 0u;
        lib_u8 digest[32];
        lib_status status = myraidenii_machine_image_path(base_directory,
            image_paths[index], path);
        if (status == LIB_STATUS_OK)
            status = lib_storage_file_read_owned(path,
                myraidenii_machine_image_identities[index].bytes,
                &images->bytes[index], &count);
        if (status != LIB_STATUS_OK ||
            count != myraidenii_machine_image_identities[index].bytes) {
            myraidenii_machine_images_destroy(images);
            return status == LIB_STATUS_OK ? LIB_STATUS_INVALID_ARGUMENT : status;
        }
        myraidenii_machine_image_sha256(images->bytes[index], count, digest);
        if (!myraidenii_machine_image_digest_matches(digest,
                myraidenii_machine_image_identities[index].sha256)) {
            myraidenii_machine_images_destroy(images);
            return LIB_STATUS_INVALID_ARGUMENT;
        }
    }
    images->graphics = (myraidenii_core_board_graphics_images){
        images->bytes[MYRAIDENII_MACHINE_IMAGE_CHARACTERS],
        images->bytes[MYRAIDENII_MACHINE_IMAGE_BACKGROUND_1],
        images->bytes[MYRAIDENII_MACHINE_IMAGE_BACKGROUND_2],
        images->bytes[MYRAIDENII_MACHINE_IMAGE_SPRITES_1],
        images->bytes[MYRAIDENII_MACHINE_IMAGE_SPRITES_2],
        images->bytes[MYRAIDENII_MACHINE_IMAGE_SPRITES_3],
        images->bytes[MYRAIDENII_MACHINE_IMAGE_SPRITES_4]};
    *out_images = images;
    return LIB_STATUS_OK;
}

lib_status myraidenii_machine_images_board_config(const myraidenii_machine_images *images,
    myraidenii_core_board_config *out_config)
{
    if (images == LIB_NULL || out_config == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_config = (myraidenii_core_board_config){
        .program_even = images->bytes[MYRAIDENII_MACHINE_IMAGE_PROGRAM_EVEN],
        .program_odd = images->bytes[MYRAIDENII_MACHINE_IMAGE_PROGRAM_ODD],
        .program_lane_bytes = MYRAIDENII_CORE_PROGRAM_LANE_BYTES,
        .graphics_images = &images->graphics,
        .sound_program = images->bytes[MYRAIDENII_MACHINE_IMAGE_SOUND_PROGRAM],
        .sound_program_bytes = MYRAIDENII_CORE_SOUND_PROGRAM_BYTES,
        .sound_samples_1 = images->bytes[MYRAIDENII_MACHINE_IMAGE_SAMPLES_1],
        .sound_samples_1_bytes = MYRAIDENII_CORE_SOUND_SAMPLE_BYTES,
        .sound_samples_2 = images->bytes[MYRAIDENII_MACHINE_IMAGE_SAMPLES_2],
        .sound_samples_2_bytes = MYRAIDENII_CORE_SOUND_SAMPLE_BYTES};
    return LIB_STATUS_OK;
}
