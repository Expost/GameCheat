// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <stdio.h>
#include <windows.h>

namespace ShooterGameCheat {
    DWORD WINAPI init(LPVOID parm);
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, ShooterGameCheat::init, NULL, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) void NullExportFunction()
{
   
}


