#ifndef LIB_TYPES_WIN32_AUDIO_H
#define LIB_TYPES_WIN32_AUDIO_H

#include "lib/types/win32/scalar.h"

#include <mmsystem.h>

typedef HWAVEOUT lib_win32_wave_output;
typedef MMRESULT lib_win32_mmresult;
typedef WAVEFORMATEX lib_win32_wave_format;
typedef WAVEHDR lib_win32_wave_header;
typedef void (CALLBACK *lib_win32_wave_callback)(lib_win32_wave_output output,
    lib_win32_uint message, lib_win32_dword_ptr instance,
    lib_win32_dword_ptr parameter_one, lib_win32_dword_ptr parameter_two);

#define lib_win32_wave_out_close waveOutClose
#define lib_win32_wave_out_open waveOutOpen
#define lib_win32_wave_out_prepare_header waveOutPrepareHeader
#define lib_win32_wave_out_reset waveOutReset
#define lib_win32_wave_out_unprepare_header waveOutUnprepareHeader
#define lib_win32_wave_out_write waveOutWrite
#define LIB_WIN32_CALLBACK_FUNCTION CALLBACK_FUNCTION
#define LIB_WIN32_MMSYSERR_NOERROR MMSYSERR_NOERROR
#define LIB_WIN32_WOM_DONE WOM_DONE
#define LIB_WIN32_WAVE_FORMAT_PCM WAVE_FORMAT_PCM
#define LIB_WIN32_WAVE_HEADER_DONE WHDR_DONE
#define LIB_WIN32_WAVE_MAPPER WAVE_MAPPER

#endif
