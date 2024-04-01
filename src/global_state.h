#pragma once

#include <Windows.h>
#include "real_entrypoints.h"
#include "bamboozler.h"
#include "extensions.h"


class GlobalState {
public:
    inline static GlobalState &instance() {
        static GlobalState state{};
        return state;
    }

    bool init() {
        if (!loadRealOpenglLoader()) {
            return false;
        }
        realEntrypoints.load(realLoader);
        
        if (!bamboozler.init()) {
            return false;
        }
        return true;
    }

    void deinit() {
        FreeLibrary(realLoader);
        bamboozler.deinit();
    }

    auto &getRealEntrypoints() { return realEntrypoints; }
    auto &getExtensions() { return extensions; }
    auto &getBamboozler() { return bamboozler; }

private:
    GlobalState::GlobalState() : bamboozler(realEntrypoints, extensions) {}

    bool GlobalState::loadRealOpenglLoader() {
        realLoader = LoadLibraryA("C:\\Windows\\System32\\opengl32.dll");
        return realLoader != NULL;
    }

    HMODULE realLoader = {};
    RealEntryPoints realEntrypoints = {};
    Extensions extensions = {};
    Bamboozler bamboozler;
};
