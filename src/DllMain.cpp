#include <windows.h>
#include "DllMain.h"
#include "MargretePlugin.h"
#include "Ware.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


DLLEXPORT void WINAPI MargretePluginGetInfo(MP_PLUGININFO* info) {
	if (!info)
		return;

	info->sdkVersion = MP_SDK_VERSION;
	if (info->nameBuffer)
		wcsncpy_s(info->nameBuffer, info->nameBufferLength, L"Margrete-WARE", info->nameBufferLength);
	if (info->descBuffer)
		wcsncpy_s(info->descBuffer, info->descBufferLength, L"Margrete-WARE", info->descBufferLength);
	if (info->developerBuffer)
		wcsncpy_s(info->developerBuffer, info->developerBufferLength, L"125iinog", info->developerBufferLength);
}

DLLEXPORT MpBoolean WINAPI MargretePluginCommandCreate(IMargretePluginCommand** ppobj) {
	*ppobj = new Ware();
	(*ppobj)->addRef();
	return MP_TRUE;
}