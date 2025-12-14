#include "pch.h"

#include "gameQuit.h"
#include "timer.h"
#include "LuaDebug.h"
#include "sceneBegin_sceneEnd.h"
#include "utf8_to_utf16.h"

FUNCTION_GAMEQUIT_0x41f9b0 p_gameQuit_0x41f9b0 = reinterpret_cast<FUNCTION_GAMEQUIT_0x41f9b0>(0x41f9b0);
FUNCTION_GAMEQUIT_0x41f9b0 p_original_gameQuit_0x41f9b0 = NULL;


void __fastcall detoured_gameQuit_0x41f9b0(uint32_t unknown) {
    scene_end();

    gTimer.end();

    LuaDebug_end();

    utf8_clearRenderRanges();

    return p_original_gameQuit_0x41f9b0(unknown);
}
