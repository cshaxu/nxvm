#ifndef LIB_TYPES_WIN32_SCALAR_H
#define LIB_TYPES_WIN32_SCALAR_H

#include <windows.h>

typedef HANDLE lib_win32_handle;
typedef BOOL lib_win32_bool;
typedef DWORD lib_win32_dword;
typedef DWORD_PTR lib_win32_dword_ptr;
typedef DWORD_PTR *lib_win32_pdword_ptr;
typedef SIZE_T lib_win32_size_t;
typedef LPVOID lib_win32_lpvoid;
typedef LPCVOID lib_win32_lpcvoid;
typedef LPCSTR lib_win32_lpcstr;
typedef LPCWSTR lib_win32_lpcwstr;
typedef LPDWORD lib_win32_lpdword;
typedef LONG lib_win32_long;
typedef LONGLONG lib_win32_longlong;
typedef LONG_PTR lib_win32_long_ptr;
typedef UINT lib_win32_uint;
typedef UINT_PTR lib_win32_uint_ptr;
typedef WORD lib_win32_word;
typedef SHORT lib_win32_short;
typedef WCHAR lib_win32_wchar;
typedef SECURITY_ATTRIBUTES lib_win32_security_attributes;
typedef LPSECURITY_ATTRIBUTES lib_win32_lpsecurity_attributes;
typedef LPTHREAD_START_ROUTINE lib_win32_thread_start_routine;
typedef LPOVERLAPPED lib_win32_lpoverlapped;
typedef SYSTEM_INFO lib_win32_system_info;
typedef INPUT_RECORD *lib_win32_pinput_record;
typedef HINSTANCE lib_win32_hinstance;
typedef TIMERPROC lib_win32_timer_proc;

#endif
