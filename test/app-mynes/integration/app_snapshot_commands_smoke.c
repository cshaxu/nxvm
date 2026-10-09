#include <assert.h>

#include "product/command.h"
#include "emulator/machine/machine_interface.h"
#include "core/driver_interface.h"
#include "lib/base/sync_interface.h"
#include "lib/storage/file_interface.h"

static void wait_state(emulator_machine *machine, emulator_machine_state state)
{ lib_u32 i; for (i=0u;i<1000u;++i) { if (emulator_machine_state_get(machine)==state) return; base_sync_sleep_milliseconds(1u); } assert(0); }
static void write_rom(void)
{ lib_u8 b[16400]={0}; lib_storage_file_writer*w=0; b[0]='N';b[1]='E';b[2]='S';b[3]=0x1a;b[4]=1;b[16]=0xea;b[16396]=0;b[16397]=0x80; assert(lib_storage_file_writer_open("mynes-app-snapshot.nes",LIB_STORAGE_FILE_WRITER_TRUNCATE,&w)==LIB_STATUS_OK);assert(lib_storage_file_writer_write(w,b,sizeof(b))==LIB_STATUS_OK);assert(lib_storage_file_writer_close(w)==LIB_STATUS_OK); }
int main(void)
{ core_driver*d=0;emulator_machine*m=0;emulator_machine_driver cd;app_command_context app;emulator_session_command_result result;
 write_rom();assert(core_driver_create(&d,&(core_driver_options){0})==LIB_STATUS_OK);assert(core_driver_make_driver(d,&cd)==LIB_STATUS_OK);assert(emulator_machine_create(&m,&cd)==LIB_STATUS_OK);assert(emulator_machine_set_removable_media(m,"mynes-app-snapshot.nes",LIB_STORAGE_MEDIUM_READONLY));assert(emulator_machine_reset(m));wait_state(m,EMULATOR_MACHINE_PAUSED);app_command_initialize(&app,m,LIB_TRUE,EMULATOR_SESSION_DISPLAY_WINDOW);
 app_command_submit_line(&app,EMULATOR_SESSION_MACHINE_PAUSED,"save mynes-app-snapshot.mns",&result);assert(lib_text_compare(result.text,"Machine saved and paused.\n\n")==0);wait_state(m,EMULATOR_MACHINE_PAUSED);assert(emulator_machine_resume(m));wait_state(m,EMULATOR_MACHINE_RUNNING);
 app_command_submit_line(&app,EMULATOR_SESSION_MACHINE_RUNNING,"save mynes-app-snapshot.mns",&result);assert(result.text[0]=='\0' && !result.arm_prompt);wait_state(m,EMULATOR_MACHINE_PAUSED);app_command_note_runtime(&app,EMULATOR_SESSION_MACHINE_RUNNING,EMULATOR_SESSION_MACHINE_PAUSED,&result);assert(lib_text_compare(result.text,"Machine saved and paused.\n\n")==0);assert(emulator_machine_stop(m));wait_state(m,EMULATOR_MACHINE_STOPPED);
 app_command_submit_line(&app,EMULATOR_SESSION_MACHINE_STOPPED,"load mynes-app-snapshot.mns",&result);assert(result.text[0]=='\0' && !result.arm_prompt);wait_state(m,EMULATOR_MACHINE_PAUSED);app_command_note_runtime(&app,EMULATOR_SESSION_MACHINE_STOPPED,EMULATOR_SESSION_MACHINE_PAUSED,&result);assert(lib_text_compare(result.text,"Machine loaded and paused.\n\n")==0);assert(emulator_machine_stop(m));wait_state(m,EMULATOR_MACHINE_STOPPED);
 app_command_submit_line(&app,EMULATOR_SESSION_MACHINE_STOPPED,"load missing.mns",&result);assert(lib_text_compare(result.text,"Cannot load machine state.\n\n")==0);assert(emulator_machine_state_get(m)==EMULATOR_MACHINE_STOPPED);assert(emulator_machine_shutdown(m)==LIB_STATUS_OK);assert(emulator_machine_destroy(m)==LIB_STATUS_OK);assert(core_driver_destroy(d)==LIB_STATUS_OK);return 0; }
