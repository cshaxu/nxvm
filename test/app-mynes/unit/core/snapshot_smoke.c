#include <assert.h>

#include "core/machine_interface.h"
#include "core/snapshot_interface.h"

enum { SNAPSHOT_TEST_RAM_BYTES = 8192u };

typedef struct snapshot_bytes { lib_u8 bytes[262144]; lib_size size, cursor; } snapshot_bytes;
static lib_status put(void *opaque, const lib_u8 *bytes, lib_size count)
{ snapshot_bytes *s=opaque; if (count > sizeof(s->bytes)-s->size) return LIB_STATUS_LIMIT_EXCEEDED; lib_memory_copy(s->bytes+s->size,bytes,count); s->size+=count; return LIB_STATUS_OK; }
static lib_status get(void *opaque, lib_u8 *bytes, lib_size count)
{ snapshot_bytes *s=opaque; if (count > s->size-s->cursor) return LIB_STATUS_IO_ERROR; lib_memory_copy(bytes,s->bytes+s->cursor,count); s->cursor+=count; return LIB_STATUS_OK; }
static lib_u32 snapshot_hash(const snapshot_bytes *snapshot)
{ lib_u32 hash = 2166136261u, index; for (index=0u; index<snapshot->size; ++index) hash=(hash^snapshot->bytes[index])*16777619u; return hash; }
static void fixture(lib_u8 *rom)
{ lib_memory_set(rom,0,16400u); rom[0]='N';rom[1]='E';rom[2]='S';rom[3]=0x1au;rom[4]=1u; rom[16]=0xa2u;rom[17]=0u;rom[18]=0xe8u;rom[19]=0x4cu;rom[20]=2u;rom[21]=0x80u;rom[16u+0x3ffcu]=0u;rom[16u+0x3ffdu]=0x80u; }
int main(void)
{ lib_u8 rom[16400], other_rom[16400]; core_machine *m=0, *other=0; snapshot_bytes s={0}; core_observation before, after, unchanged; core_run_result run;
  fixture(rom); assert(core_machine_create(&m,rom,sizeof(rom),&(core_machine_options){0})==LIB_STATUS_OK);
  assert(core_machine_run(m,10u,100u,&run)==LIB_STATUS_OK); assert(core_machine_observe(m,&before)==LIB_STATUS_OK);
  assert(core_snapshot_write(m,put,&s)==LIB_STATUS_OK); assert(core_machine_run(m,10u,100u,&run)==LIB_STATUS_OK);
  assert(s.size > 8u && s.bytes[4] == 2u && s.bytes[5] == 0u &&
      s.bytes[6] == 0u && s.bytes[7] == 0u);
  assert(s.size == 136006u && snapshot_hash(&s) == 0x5e1fb997u);
  s.cursor=0u; assert(core_snapshot_read(m,get,&s)==LIB_STATUS_OK); assert(core_machine_observe(m,&after)==LIB_STATUS_OK);
  assert(after.pc==before.pc && after.x==before.x && after.cycles==before.cycles);
  assert(core_machine_run(m,1u,10u,&run)==LIB_STATUS_OK); assert(core_machine_observe(m,&unchanged)==LIB_STATUS_OK);
  s.bytes[0]='X'; s.cursor=0u; assert(core_snapshot_read(m,get,&s)!=LIB_STATUS_OK); assert(core_machine_observe(m,&after)==LIB_STATUS_OK);
  assert(after.pc==unchanged.pc && after.cycles==unchanged.cycles); s.bytes[0]='M';
  --s.size; s.cursor=0u; assert(core_snapshot_read(m,get,&s)!=LIB_STATUS_OK);
  assert(core_machine_observe(m,&after)==LIB_STATUS_OK);
  assert(after.pc==unchanged.pc && after.cycles==unchanged.cycles); ++s.size;
  s.bytes[4]=1u; s.cursor=0u; assert(core_snapshot_read(m,get,&s)!=LIB_STATUS_OK);
  assert(core_machine_observe(m,&after)==LIB_STATUS_OK);
  assert(after.pc==unchanged.pc && after.cycles==unchanged.cycles); s.bytes[4]=2u;
  s.bytes[s.size - (SNAPSHOT_TEST_RAM_BYTES + 1u)]=2u; s.cursor=0u; assert(core_snapshot_read(m,get,&s)!=LIB_STATUS_OK);
  assert(core_machine_observe(m,&after)==LIB_STATUS_OK);
  assert(after.pc==unchanged.pc && after.cycles==unchanged.cycles); s.bytes[s.size - (SNAPSHOT_TEST_RAM_BYTES + 1u)]=1u;
  fixture(other_rom); other_rom[16u + 1u] = 0xeau;
  assert(core_machine_create(&other,other_rom,sizeof(other_rom),&(core_machine_options){0})==LIB_STATUS_OK);
  s.cursor=0u; assert(core_snapshot_read(other,get,&s)!=LIB_STATUS_OK);
  core_machine_destroy(other); core_machine_destroy(m); return 0; }
