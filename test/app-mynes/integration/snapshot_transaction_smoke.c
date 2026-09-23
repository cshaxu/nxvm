#include <assert.h>
#include <string.h>

#include "common/machine/machine_interface.h"
#include "core/driver_interface.h"
#include "lib/base/sync_interface.h"
#include "lib/storage/file_interface.h"

typedef struct snapshot_bytes { lib_u8 bytes[262144]; lib_size size, cursor; } snapshot_bytes;
static lib_status put(void *o,const lib_u8 *b,lib_size n){snapshot_bytes*s=o;if(n>sizeof(s->bytes)-s->size)return LIB_STATUS_LIMIT_EXCEEDED;memcpy(s->bytes+s->size,b,n);s->size+=n;return LIB_STATUS_OK;}
static lib_status get(void *o,lib_u8*b,lib_size n){snapshot_bytes*s=o;if(n>s->size-s->cursor)return LIB_STATUS_IO_ERROR;memcpy(b,s->bytes+s->cursor,n);s->cursor+=n;return LIB_STATUS_OK;}
static void wait_state(common_machine*m,common_machine_state want){lib_u32 i;for(i=0;i<1000u;++i){if(common_machine_state_get(m)==want)return;base_sync_sleep_milliseconds(1u);}assert(0);}
static void fixture(void){lib_u8 b[16400]={0};lib_storage_file_writer*w=0;b[0]='N';b[1]='E';b[2]='S';b[3]=0x1a;b[4]=1;b[16]=0xa2;b[17]=0;b[18]=0xe8;b[19]=0x4c;b[20]=2;b[21]=0x80;b[16400-4]=0;b[16400-3]=0x80;assert(lib_storage_file_writer_open("mynes-snapshot.nes",LIB_STORAGE_FILE_WRITER_TRUNCATE,&w)==LIB_STATUS_OK);assert(lib_storage_file_writer_write(w,b,sizeof(b))==LIB_STATUS_OK);assert(lib_storage_file_writer_close(w)==LIB_STATUS_OK);}
static lib_u16 pc(common_machine*m){common_machine_debug_lease l;lib_u8 q[8]={1,0,1},r[64];lib_size n=0;assert(common_machine_debug_acquire(m,&l)==LIB_STATUS_OK);assert(common_machine_debug_execute_with_lease(m,&l,q,sizeof(q),r,sizeof(r),&n)==LIB_STATUS_OK);return (lib_u16)r[18]|((lib_u16)r[19]<<8u);}
int main(void){core_driver*d=0;common_machine*m=0;common_machine_driver cd;snapshot_bytes s={0};lib_u16 saved;
 fixture();assert(core_driver_create(&d,&(core_driver_options){0})==LIB_STATUS_OK);assert(core_driver_make_driver(d,&cd)==LIB_STATUS_OK);assert(common_machine_create(&m,&cd)==LIB_STATUS_OK);assert(common_machine_set_removable_media(m,"mynes-snapshot.nes",LIB_STORAGE_MEDIUM_READONLY));assert(common_machine_reset(m));wait_state(m,COMMON_MACHINE_PAUSED);assert(common_machine_resume(m));wait_state(m,COMMON_MACHINE_RUNNING);base_sync_sleep_milliseconds(10u);assert(common_machine_pause(m));wait_state(m,COMMON_MACHINE_PAUSED);saved=pc(m);
 assert(common_machine_read_state(m,&(common_machine_state_writer){put,&s})==LIB_STATUS_OK);wait_state(m,COMMON_MACHINE_PAUSED);assert(common_machine_resume(m));wait_state(m,COMMON_MACHINE_RUNNING);base_sync_sleep_milliseconds(10u);assert(common_machine_stop(m));wait_state(m,COMMON_MACHINE_STOPPED);s.cursor=0;assert(common_machine_write_state(m,&(common_machine_state_reader){get,&s})==LIB_STATUS_OK);wait_state(m,COMMON_MACHINE_PAUSED);assert(pc(m)==saved);
 assert(common_machine_stop(m));wait_state(m,COMMON_MACHINE_STOPPED);s.bytes[0]='X';s.cursor=0;assert(common_machine_write_state(m,&(common_machine_state_reader){get,&s})!=LIB_STATUS_OK);assert(common_machine_state_get(m)==COMMON_MACHINE_STOPPED);assert(common_machine_shutdown(m)==LIB_STATUS_OK);assert(common_machine_destroy(m)==LIB_STATUS_OK);assert(core_driver_destroy(d)==LIB_STATUS_OK);return 0;}
