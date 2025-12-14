#include "pch.h"

#pragma comment(lib, "Winmm.lib")

#include <string>

using namespace std;

#include "Windows.h"
#include "winuser.h"
#include "processthreadsapi.h"
#include "Mmsystem.h"

#include "utf8_to_utf16.h"
#include "Vanilla1121_functions.h"
#include "notifyOS.h"

void flashTaskbarIcon() {
    if (vanilla1121_gameInForeground() == true) {
        return;
    }

    FLASHWINFO param = {};
    param.cbSize = sizeof(FLASHWINFO);
    param.hwnd = vanilla1121_gameWindow();
    param.dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG;
    param.uCount = 0u;
    param.dwTimeout = 0u;

    FlashWindowEx(&param);
}

// Play a system sound
bool playSystemSound(const string soundName) {
    if (vanilla1121_gameInForeground() == true) {
        return false;
    }

    if (soundName == u8"SystemAsterisk"
        || soundName == u8"SystemDefault"
        || soundName == u8"SystemExclamation"
        || soundName == u8"SystemExit"
        || soundName == u8"SystemHand"
        || soundName == u8"SystemQuestion"
        || soundName == u8"SystemStart"
        || soundName == u8"SystemWelcome") {

        return PlaySoundW(utf8_to_utf16(soundName).data(), NULL, SND_ALIAS | SND_ASYNC | SND_SENTRY);
    }
    return false;
}
