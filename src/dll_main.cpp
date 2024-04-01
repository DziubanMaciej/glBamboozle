#include <Windows.h>

#include "global_state.h"

HINSTANCE glBamboozleInstance = nullptr;

BOOL WINAPI DllMain(HINSTANCE module, DWORD fdwReason, LPVOID) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        glBamboozleInstance = module;
        if (!GlobalState::instance().init()) {
            return FALSE;
        }
        return TRUE;
    case DLL_PROCESS_DETACH:
        // TODO this is too late... When should we call it? wglDeleteContext?
        // GlobalState::instance().deinit();
        return TRUE;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        return TRUE;
    }
}


