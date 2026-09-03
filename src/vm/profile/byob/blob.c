#include "type.h"

#include "vm/profile/byob/blob.h"

#include "core/platform/file.h"

static type_unsigned_32 vm_profile_byob_sha256_rotate_right(
    type_unsigned_32 value, type_unsigned_8 bits)
{
    return (value >> bits) | (value << (32u - bits));
}

static C_VOID vm_profile_byob_sha256_block(type_unsigned_32 state[8],
    const type_unsigned_8 block[64])
{
    static const type_unsigned_32 constants[64] = {
        0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
        0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
        0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
        0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
        0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
        0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
        0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
        0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u };
    type_unsigned_32 words[64];
    type_unsigned_32 a, b, c, d, e, f, g, h;
    STD_SIZE_T index;

    for (index = 0u; index < 16u; ++index) words[index] =
        ((type_unsigned_32)block[index * 4u] << 24u) |
        ((type_unsigned_32)block[index * 4u + 1u] << 16u) |
        ((type_unsigned_32)block[index * 4u + 2u] << 8u) | block[index * 4u + 3u];
    for (; index < 64u; ++index) {
        type_unsigned_32 s0 = vm_profile_byob_sha256_rotate_right(words[index - 15u], 7u) ^
            vm_profile_byob_sha256_rotate_right(words[index - 15u], 18u) ^ (words[index - 15u] >> 3u);
        type_unsigned_32 s1 = vm_profile_byob_sha256_rotate_right(words[index - 2u], 17u) ^
            vm_profile_byob_sha256_rotate_right(words[index - 2u], 19u) ^ (words[index - 2u] >> 10u);
        words[index] = words[index - 16u] + s0 + words[index - 7u] + s1;
    }
    a = state[0]; b = state[1]; c = state[2]; d = state[3]; e = state[4]; f = state[5]; g = state[6]; h = state[7];
    for (index = 0u; index < 64u; ++index) {
        type_unsigned_32 s1 = vm_profile_byob_sha256_rotate_right(e, 6u) ^ vm_profile_byob_sha256_rotate_right(e, 11u) ^ vm_profile_byob_sha256_rotate_right(e, 25u);
        type_unsigned_32 choice = (e & f) ^ ((~e) & g);
        type_unsigned_32 temp1 = h + s1 + choice + constants[index] + words[index];
        type_unsigned_32 s0 = vm_profile_byob_sha256_rotate_right(a, 2u) ^ vm_profile_byob_sha256_rotate_right(a, 13u) ^ vm_profile_byob_sha256_rotate_right(a, 22u);
        type_unsigned_32 majority = (a & b) ^ (a & c) ^ (b & c);
        type_unsigned_32 temp2 = s0 + majority;
        h = g; g = f; f = e; e = d + temp1; d = c; c = b; b = a; a = temp1 + temp2;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

static C_VOID vm_profile_byob_sha256(const type_unsigned_8 *bytes,
    STD_SIZE_T byte_count, type_unsigned_8 digest[32])
{
    type_unsigned_32 state[8] = {0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,
        0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u};
    type_unsigned_8 final[64] = {0};
    type_unsigned_64 bit_count = (type_unsigned_64)byte_count * 8u;
    STD_SIZE_T index;
    STD_SIZE_T final_bytes;

    for (index = 0u; index + 64u <= byte_count; index += 64u) vm_profile_byob_sha256_block(state, bytes + index);
    final_bytes = byte_count - index;
    if (final_bytes != 0u) STD_MEMCPY(final, bytes + index, final_bytes);
    final[final_bytes++] = 0x80u;
    if (final_bytes > 56u) { vm_profile_byob_sha256_block(state, final); STD_MEMSET(final, 0, sizeof(final)); }
    for (index = 0u; index < 8u; ++index) final[63u - index] = (type_unsigned_8)(bit_count >> (index * 8u));
    vm_profile_byob_sha256_block(state, final);
    for (index = 0u; index < 8u; ++index) {
        digest[index * 4u] = (type_unsigned_8)(state[index] >> 24u);
        digest[index * 4u + 1u] = (type_unsigned_8)(state[index] >> 16u);
        digest[index * 4u + 2u] = (type_unsigned_8)(state[index] >> 8u);
        digest[index * 4u + 3u] = (type_unsigned_8)state[index];
    }
}

static C_INT vm_profile_byob_sha256_matches(const type_unsigned_8 digest[32], const C_CHAR *text)
{
    STD_SIZE_T index;
    if (text == STD_NULL || STD_STRLEN(text) != 64u) return 0;
    for (index = 0u; index < 32u; ++index) {
        C_INT high = text[index * 2u] >= '0' && text[index * 2u] <= '9' ? text[index * 2u] - '0' :
            text[index * 2u] >= 'a' && text[index * 2u] <= 'f' ? text[index * 2u] - 'a' + 10 :
            text[index * 2u] >= 'A' && text[index * 2u] <= 'F' ? text[index * 2u] - 'A' + 10 : -1;
        C_INT low = text[index * 2u + 1u] >= '0' && text[index * 2u + 1u] <= '9' ? text[index * 2u + 1u] - '0' :
            text[index * 2u + 1u] >= 'a' && text[index * 2u + 1u] <= 'f' ? text[index * 2u + 1u] - 'a' + 10 :
            text[index * 2u + 1u] >= 'A' && text[index * 2u + 1u] <= 'F' ? text[index * 2u + 1u] - 'A' + 10 : -1;
        if (high < 0 || low < 0 || digest[index] != (type_unsigned_8)((high << 4u) | low)) return 0;
    }
    return 1;
}

C_INT vm_profile_byob_blob_is_valid(const vm_profile_byob_blob *blob)
{
    return blob != STD_NULL && blob->path != STD_NULL && blob->path[0] != '\0' &&
        (blob->sha256 == STD_NULL || STD_STRLEN(blob->sha256) == 64u) && blob->bytes != 0u;
}

C_INT vm_profile_byob_option_rom_is_valid(const type_unsigned_8 *bytes,
    STD_SIZE_T byte_count, STD_SIZE_T maximum_bytes)
{
    type_unsigned_8 checksum = 0u;
    STD_SIZE_T index;

    if (bytes == STD_NULL || byte_count < 3u || byte_count > maximum_bytes ||
        bytes[0u] != 0x55u || bytes[1u] != 0xaau ||
        (STD_SIZE_T)bytes[2u] * 512u != byte_count) return 0;
    for (index = 0u; index < byte_count; ++index) {
        checksum = (type_unsigned_8)(checksum + bytes[index]);
    }
    return checksum == 0u;
}

type_status vm_profile_byob_blob_load(const vm_profile_byob_blob *blob,
    type_unsigned_8 *out_bytes)
{
    C_VOID *loaded = STD_NULL;
    STD_SIZE_T count;
    type_unsigned_8 digest[32];

    if (!vm_profile_byob_blob_is_valid(blob) || out_bytes == STD_NULL ||
        core_platform_file_read_all(blob->path, blob->bytes, &loaded, &count) != TYPE_FALSE ||
        count != blob->bytes) { STD_FREE(loaded); return TYPE_STATUS_FAULT; }
    STD_MEMCPY(out_bytes, loaded, count);
    STD_FREE(loaded);
    if (blob->sha256 == STD_NULL) return TYPE_STATUS_OK;
    vm_profile_byob_sha256(out_bytes, blob->bytes, digest);
    return vm_profile_byob_sha256_matches(digest, blob->sha256) ? TYPE_STATUS_OK : TYPE_STATUS_FAULT;
}
