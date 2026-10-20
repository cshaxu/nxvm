#include "myraidenii/machine/image_hash.h"

static const lib_u32 myraidenii_machine_image_sha256_constants[64] = {
    0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
    0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
    0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
    0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
    0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
    0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
    0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
    0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u};

static lib_u32 myraidenii_machine_image_rotr(lib_u32 value, lib_u8 bits)
{ return (value >> bits) | (value << (32u - bits)); }

static void myraidenii_machine_image_sha256_block(lib_u32 state[8], const lib_u8 block[64])
{
    lib_u32 words[64];
    lib_u32 a, b, c, d, e, f, g, h;
    for (lib_size index = 0u; index < 16u; ++index)
        words[index] = ((lib_u32)block[index * 4u] << 24u) |
            ((lib_u32)block[index * 4u + 1u] << 16u) |
            ((lib_u32)block[index * 4u + 2u] << 8u) | block[index * 4u + 3u];
    for (lib_size index = 16u; index < 64u; ++index) {
        lib_u32 s0 = myraidenii_machine_image_rotr(words[index - 15u], 7u) ^
            myraidenii_machine_image_rotr(words[index - 15u], 18u) ^ (words[index - 15u] >> 3u);
        lib_u32 s1 = myraidenii_machine_image_rotr(words[index - 2u], 17u) ^
            myraidenii_machine_image_rotr(words[index - 2u], 19u) ^ (words[index - 2u] >> 10u);
        words[index] = words[index - 16u] + s0 + words[index - 7u] + s1;
    }
    a=state[0]; b=state[1]; c=state[2]; d=state[3]; e=state[4]; f=state[5]; g=state[6]; h=state[7];
    for (lib_size index = 0u; index < 64u; ++index) {
        lib_u32 s1 = myraidenii_machine_image_rotr(e,6u)^myraidenii_machine_image_rotr(e,11u)^myraidenii_machine_image_rotr(e,25u);
        lib_u32 choice = (e & f) ^ ((~e) & g);
        lib_u32 temporary_1 = h + s1 + choice + myraidenii_machine_image_sha256_constants[index] + words[index];
        lib_u32 s0 = myraidenii_machine_image_rotr(a,2u)^myraidenii_machine_image_rotr(a,13u)^myraidenii_machine_image_rotr(a,22u);
        lib_u32 majority = (a & b) ^ (a & c) ^ (b & c);
        lib_u32 temporary_2 = s0 + majority;
        h=g; g=f; f=e; e=d+temporary_1; d=c; c=b; b=a; a=temporary_1+temporary_2;
    }
    state[0]+=a; state[1]+=b; state[2]+=c; state[3]+=d; state[4]+=e; state[5]+=f; state[6]+=g; state[7]+=h;
}

void myraidenii_machine_image_sha256(const lib_u8 *bytes, lib_size byte_count,
    lib_u8 out_digest[MYRAIDENII_MACHINE_IMAGE_SHA256_BYTES])
{
    lib_u32 state[8]={0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u};
    lib_u8 block[64]={0};
    lib_size offset=0u, remainder;
    lib_u64 bit_count=(lib_u64)byte_count*8u;
    if (out_digest == LIB_NULL || (bytes == LIB_NULL && byte_count != 0u)) return;
    while (byte_count - offset >= 64u) { myraidenii_machine_image_sha256_block(state, bytes + offset); offset += 64u; }
    remainder=byte_count-offset;
    if (remainder != 0u) lib_memory_copy(block, bytes + offset, remainder);
    block[remainder++]=0x80u;
    if (remainder > 56u) { myraidenii_machine_image_sha256_block(state, block); lib_memory_set(block,0,64u); }
    for (lib_size index=0u; index<8u; ++index) block[56u+index]=(lib_u8)(bit_count >> (56u-index*8u));
    myraidenii_machine_image_sha256_block(state, block);
    for (lib_size index=0u; index<8u; ++index) { out_digest[index*4u]=(lib_u8)(state[index]>>24u); out_digest[index*4u+1u]=(lib_u8)(state[index]>>16u); out_digest[index*4u+2u]=(lib_u8)(state[index]>>8u); out_digest[index*4u+3u]=(lib_u8)state[index]; }
}
