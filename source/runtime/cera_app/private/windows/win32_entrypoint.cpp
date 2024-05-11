#include "launch.h"

#include "windows/win32_min.h"
#include "windows/win32_system_includes.h"

#include "generic_entrypoint.h"

#include <Shlwapi.h>

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nShowCmd)
{
    // Cache hinstance
    g_instance_handle = hInstance;

    // Set the working directory to the path of the executable.
    WCHAR path[MAX_PATH];
    HMODULE h_module = GetModuleHandleW(NULL);
    if (GetModuleFileNameW(h_module, path, MAX_PATH) > 0)
    {
        PathRemoveFileSpecW(path);
        SetCurrentDirectoryW(path);
    }

    return cera::launch(cera::entry());
}

int main()
{
    // You may need to obtain the required parameters for wWinMain
    HINSTANCE histance = GetModuleHandle(NULL);
    HINSTANCE hprevinstance = NULL;
    PWSTR pcmdline = GetCommandLineW();
    s32 ncmdshow = SW_SHOWDEFAULT; // or any other appropriate value

    return wWinMain(histance, hprevinstance, pcmdline, ncmdshow);
}