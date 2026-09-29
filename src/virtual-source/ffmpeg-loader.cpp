#include <windows.h>
#include <delayimp.h>
#include "ffmpeg-loader.h"

extern "C" {
#include "libswscale/version.h"
};

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)
#define SWSCALE_DLL "swscale-" TO_STRING(LIBSWSCALE_VERSION_MAJOR) ".dll"

static HMODULE load_from_private_dir(const char *dll_name)
{
	HMODULE self = nullptr;
	if (!GetModuleHandleExW(
		    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
			    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		    (LPCWSTR)&load_from_private_dir, &self))
		return nullptr;

	wchar_t path[MAX_PATH];
	DWORD len = GetModuleFileNameW(self, path, MAX_PATH);
	if (len == 0 || len >= MAX_PATH)
		return nullptr;

	wchar_t *slash = wcsrchr(path, L'\\');
	if (!slash)
		return nullptr;
	*(slash + 1) = 0;

	wchar_t name[MAX_PATH];
	if (!MultiByteToWideChar(CP_ACP, 0, dll_name, -1, name, MAX_PATH))
		return nullptr;

	if (wcscat_s(path, FFMPEG_PRIVATE_DIR L"\\") != 0 ||
	    wcscat_s(path, name) != 0)
		return nullptr;

	// Altered search path lets swscale find avutil in the same folder.
	return LoadLibraryExW(path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
}

static FARPROC WINAPI delay_load_hook(unsigned notify, PDelayLoadInfo info)
{
	if (notify != dliNotePreLoadLibrary)
		return nullptr;

	// Returning null falls back to the normal search, which still finds
	// DLLs placed next to this one by older installs.
	return (FARPROC)load_from_private_dir(info->szDll);
}

extern "C" const PfnDliHook __pfnDliNotifyHook2 = delay_load_hook;

bool ffmpeg_loaded()
{
	__try {
		return SUCCEEDED(__HrLoadAllImportsForDll(SWSCALE_DLL));
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}
