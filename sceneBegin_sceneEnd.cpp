#include "pch.h"

// This file requires to be saved in UTF-8 BOM, as it includes Chinese strings

#pragma comment(lib, "libMinHook.x86.lib")

#include <sstream>

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "MinHook.h"

#include "sceneBegin_sceneEnd.h"
#include "utf8_to_utf16.h"
#include "Vanilla1121_functions.h"

static ID3DXFont* scene_fallback = NULL;
static ID3DXFont* scene_fallbackBIG = NULL;
static ID3DXFont* scene_fallbackSmall = NULL;
static ID3DXFont* scene_fallbackHUGE = NULL;
static ID3DXFont* scene_selected = NULL;
static ID3DXFont* scene_selectedBIG = NULL;
static ID3DXFont* scene_selectedSmall = NULL;
static ID3DXFont* scene_selectedHUGE = NULL;
static std::list<worldText::Floating> floatingTexts{};
static std::unordered_map<uint64_t, worldText::Crit> critTexts{};
static std::list<worldText::Floating> smallFloatingTexts{};
static bool scene_checkIfD3D = false;
static std::string scene_disableReason{};

LPDIRECT3DDEVICE9 scene_lastDXdevice = NULL;
bool scene_needReloadFont = false;
bool scene_fontsOnLost = false;
bool scene_isEnabled = false;
bool scene_hideEXPtext = false;
int scene_fontSize = 36;
// ChatGPT: Microsoft YaHei is an Unicode font and it exists even on English Windows Vista
const static std::string scene_fallbackFontName{ u8"Microsoft YaHei" };
std::string scene_userSelectedFontName{ scene_fallbackFontName };

LPD3DXCREATEFFONTW p_D3DXCreateFontW = NULL;
void scene_init() {
    HMODULE hDLL = NULL;
    for (int ver = 43; ver >= 24; --ver) {
        std::stringstream ss{};
        ss << "d3dx9_" << ver << ".dll";

        hDLL = LoadLibraryA(ss.str().c_str());
        if (hDLL != NULL) {
            break;
        }
    }
    if (hDLL == NULL) {
        scene_disableReason = u8"UnitXP_SP3 requires d3dx9_43.dll to support anti-aliased floating combat text. Please try installing DirectX End-User Runtimes.";
        return;
    }

    p_D3DXCreateFontW = reinterpret_cast<LPD3DXCREATEFFONTW>(GetProcAddress(hDLL, "D3DXCreateFontW"));

    if (p_D3DXCreateFontW == NULL) {
        FreeLibrary(hDLL);
        scene_disableReason = u8"It seems d3dx9 is broken. Please try reinstalling DirectX End-User Runtimes.";
        return;
    }

    scene_isEnabled = true;
}

static bool scene_fontIsOutlineCapable(ID3DXFont* font) {
    return GetOutlineTextMetricsW(font->GetDC(), 0, NULL) != 0;
}

static void scene_fontPreload(ID3DXFont* font) {
    // Visiable ASCII by ChatGPT
    font->PreloadCharacters(0x20, 0x7e);

    // Chinese
    // ChatGPT says: Calling ID3DXFont::PreloadText on a string that contains characters the font cannot render is safe in the sense that the API itself does not document (and there are no common reports of) crashes or undefined memory corruption when characters are unsupported.
    font->PreloadTextW(utf8_to_utf16(u8"经验值：").c_str(), 4);
    font->PreloadTextW(utf8_to_utf16(u8"脱离进入战斗").c_str(), 6);
    font->PreloadTextW(utf8_to_utf16(u8"非荣誉击杀").c_str(), 5);
    font->PreloadTextW(utf8_to_utf16(u8"未命中").c_str(), 3);
    font->PreloadTextW(utf8_to_utf16(u8"闪避").c_str(), 2);
    font->PreloadTextW(utf8_to_utf16(u8"招架").c_str(), 2);
    font->PreloadTextW(utf8_to_utf16(u8"格挡").c_str(), 2);
    font->PreloadTextW(utf8_to_utf16(u8"吸收").c_str(), 2);
    font->PreloadTextW(utf8_to_utf16(u8"免疫").c_str(), 2);
}

static void scene_fontsOnLostDevice() {
    if (scene_fallback != NULL) {
        scene_fallback->OnLostDevice();
    }
    if (scene_fallbackBIG != NULL) {
        scene_fallbackBIG->OnLostDevice();
    }
    if (scene_fallbackSmall != NULL) {
        scene_fallbackSmall->OnLostDevice();
    }
    if (scene_fallbackHUGE != NULL) {
        scene_fallbackHUGE->OnLostDevice();
    }
    if (scene_selected != NULL) {
        scene_selected->OnLostDevice();
    }
    if (scene_selectedBIG != NULL) {
        scene_selectedBIG->OnLostDevice();
    }
    if (scene_selectedSmall != NULL) {
        scene_selectedSmall->OnLostDevice();
    }
    if (scene_selectedHUGE != NULL) {
        scene_selectedHUGE->OnLostDevice();
    }
    scene_fontsOnLost = true;
}

static bool scene_fontsOnResetDevice() {
    if (scene_fallback != NULL) {
        if (false == SUCCEEDED(scene_fallback->OnResetDevice())) {
            return false;
        }
    }
    if (scene_fallbackBIG != NULL) {
        if (false == SUCCEEDED(scene_fallbackBIG->OnResetDevice())) {
            return false;
        }
    }
    if (scene_fallbackSmall != NULL) {
        if (false == SUCCEEDED(scene_fallbackSmall->OnResetDevice())) {
            return false;
        }
    }
    if (scene_fallbackHUGE != NULL) {
        if (false == SUCCEEDED(scene_fallbackHUGE->OnResetDevice())) {
            return false;
        }
    }
    if (scene_selected != NULL) {
        if (false == SUCCEEDED(scene_selected->OnResetDevice())) {
            return false;
        }
    }
    if (scene_selectedBIG != NULL) {
        if (false == SUCCEEDED(scene_selectedBIG->OnResetDevice())) {
            return false;
        }
    }
    if (scene_selectedSmall != NULL) {
        if (false == SUCCEEDED(scene_selectedSmall->OnResetDevice())) {
            return false;
        }
    }
    if (scene_selectedHUGE != NULL) {
        if (false == SUCCEEDED(scene_selectedHUGE->OnResetDevice())) {
            return false;
        }
    }
    scene_fontsOnLost = false;
    return true;
}

static void scene_unloadFonts() {
    if (scene_fallback != NULL) {
        scene_fallback->Release();
        scene_fallback = NULL;
    }
    if (scene_fallbackBIG != NULL) {
        scene_fallbackBIG->Release();
        scene_fallbackBIG = NULL;
    }
    if (scene_fallbackSmall != NULL) {
        scene_fallbackSmall->Release();
        scene_fallbackSmall = NULL;
    }
    if (scene_fallbackHUGE != NULL) {
        scene_fallbackHUGE->Release();
        scene_fallbackHUGE = NULL;
    }
    if (scene_selected != NULL) {
        scene_selected->Release();
        scene_selected = NULL;
    }
    if (scene_selectedBIG != NULL) {
        scene_selectedBIG->Release();
        scene_selectedBIG = NULL;
    }
    if (scene_selectedSmall != NULL) {
        scene_selectedSmall->Release();
        scene_selectedSmall = NULL;
    }
    if (scene_selectedHUGE != NULL) {
        scene_selectedHUGE->Release();
        scene_selectedHUGE = NULL;
    }
}

void scene_end() {
    scene_unloadFonts();
    scene_isEnabled = false;
}

bool scene_reloadFont() {
    if (scene_isEnabled == false) {
        return false;
    }
    else {
        scene_unloadFonts();
    }

    {
        if (scene_fallback == NULL) {
            if (false == SUCCEEDED(p_D3DXCreateFontW(scene_lastDXdevice, scene_fontSize, 0, FW_SEMIBOLD, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_DONTCARE, utf8_to_utf16(scene_fallbackFontName).c_str(), &scene_fallback))) {
                scene_fallback = NULL;
                scene_isEnabled = false;
                scene_disableReason = u8"Failed to load the fallback font. This is usually due to an imcomplete installation of the Windows operating system.";
                return false;
            }
            if (false == scene_fontIsOutlineCapable(scene_fallback)) {
                scene_unloadFonts();
                scene_isEnabled = false;
                scene_disableReason = u8"The fallback font is not outline-capable. This is usually due to an imcomplete installation of the Windows operating system.";
                return false;
            }
            scene_fontPreload(scene_fallback);
        }

        if (scene_fallbackBIG == NULL) {
            if (false == SUCCEEDED(p_D3DXCreateFontW(scene_lastDXdevice, scene_fontSize + 15, 0, FW_BOLD, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_DONTCARE, utf8_to_utf16(scene_fallbackFontName).c_str(), &scene_fallbackBIG))) {
                scene_fallbackBIG = NULL;
                return false;
            }
            scene_fontPreload(scene_fallbackBIG);
        }

        if (scene_fallbackSmall == NULL) {
            if (false == SUCCEEDED(p_D3DXCreateFontW(scene_lastDXdevice, scene_fontSize - 4, 0, FW_SEMIBOLD, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_DONTCARE, utf8_to_utf16(scene_fallbackFontName).c_str(), &scene_fallbackSmall))) {
                scene_fallbackSmall = NULL;
                return false;
            }
            scene_fontPreload(scene_fallbackSmall);
        }

        if (scene_fallbackHUGE == NULL) {
            if (false == SUCCEEDED(p_D3DXCreateFontW(scene_lastDXdevice, scene_fontSize + 15, 0, FW_HEAVY, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_DONTCARE, utf8_to_utf16(scene_fallbackFontName).c_str(), &scene_fallbackHUGE))) {
                scene_fallbackHUGE = NULL;
                return false;
            }
            scene_fontPreload(scene_fallbackHUGE);
        }
    }

    {
        if (scene_selected == NULL) {
            if (false == SUCCEEDED(p_D3DXCreateFontW(scene_lastDXdevice, scene_fontSize, 0, FW_SEMIBOLD, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_DONTCARE, utf8_to_utf16(scene_userSelectedFontName).c_str(), &scene_selected))) {
                scene_userSelectedFontName = scene_fallbackFontName;
                if (false == SUCCEEDED(p_D3DXCreateFontW(scene_lastDXdevice, scene_fontSize, 0, FW_SEMIBOLD, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_DONTCARE, utf8_to_utf16(scene_userSelectedFontName).c_str(), &scene_selected))) {
                    scene_selected = NULL;
                    return false;
                }
            }
            if (false == scene_fontIsOutlineCapable(scene_selected)) {
                scene_selected->Release();
                scene_selected = NULL;
                scene_userSelectedFontName = scene_fallbackFontName;
                if (false == SUCCEEDED(p_D3DXCreateFontW(scene_lastDXdevice, scene_fontSize, 0, FW_SEMIBOLD, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_DONTCARE, utf8_to_utf16(scene_userSelectedFontName).c_str(), &scene_selected))) {
                    scene_selected = NULL;
                    return false;
                }
            }
            scene_fontPreload(scene_selected);
            utf8_reloadRenderRanges(reinterpret_cast<void*>(scene_selected));
        }

        if (scene_selectedBIG == NULL) {
            if (false == SUCCEEDED(p_D3DXCreateFontW(scene_lastDXdevice, scene_fontSize + 15, 0, FW_BOLD, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_DONTCARE, utf8_to_utf16(scene_userSelectedFontName).c_str(), &scene_selectedBIG))) {
                scene_selectedBIG = NULL;
                return false;
            }
            scene_fontPreload(scene_selectedBIG);
        }

        if (scene_selectedSmall == NULL) {
            if (false == SUCCEEDED(p_D3DXCreateFontW(scene_lastDXdevice, scene_fontSize - 4, 0, FW_SEMIBOLD, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_DONTCARE, utf8_to_utf16(scene_userSelectedFontName).c_str(), &scene_selectedSmall))) {
                scene_selectedSmall = NULL;
                return false;
            }
            scene_fontPreload(scene_selectedSmall);
        }

        if (scene_selectedHUGE == NULL) {
            if (false == SUCCEEDED(p_D3DXCreateFontW(scene_lastDXdevice, scene_fontSize + 15, 0, FW_HEAVY, 1, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH | FF_DONTCARE, utf8_to_utf16(scene_userSelectedFontName).c_str(), &scene_selectedHUGE))) {
                scene_selectedHUGE = NULL;
                return false;
            }
            scene_fontPreload(scene_selectedHUGE);
        }
    }

    scene_needReloadFont = false;
    return true;
}

ISCENEBEGIN p_sceneBegin = reinterpret_cast<ISCENEBEGIN>(0x5a1680);
ISCENEBEGIN p_original_sceneBegin = NULL;
void __fastcall detoured_sceneBegin(uint32_t CGxDevice, void* ignored, uint32_t unknown) {
    if (scene_isEnabled == false) {
        p_original_sceneBegin(CGxDevice, unknown);
        return;
    }

    if (scene_checkIfD3D == false) {
        if (vanilla1121_getCVar("gxApi") != "direct3d") {
            scene_isEnabled = false;
            p_original_sceneBegin(CGxDevice, unknown);
            scene_disableReason = u8"UnitXP_SP3 only supports the Direct3D renderer. OpenGL is not supported.";
            return;
        }
        else {
            scene_checkIfD3D = true;
        }
    }

    LPDIRECT3DDEVICE9 dxDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(vanilla1121_d3dDevice(CGxDevice));
    if (dxDevice == NULL || (reinterpret_cast<uintptr_t>(dxDevice) & 1) != 0) {
        p_original_sceneBegin(CGxDevice, unknown);
        return;
    }

    HRESULT test = dxDevice->TestCooperativeLevel();
    if (D3DERR_DEVICELOST == test) {
        if (scene_fontsOnLost == false) {
            scene_fontsOnLostDevice();
        }
    }

    if (D3D_OK == test || D3DERR_DEVICENOTRESET == test) {
        if (scene_fontsOnLost == true) {
            scene_fontsOnResetDevice();
        }
    }

    p_original_sceneBegin(CGxDevice, unknown);
}

ISCENEEND p_sceneEnd = reinterpret_cast<ISCENEEND>(0x5a17a0);
ISCENEEND p_original_sceneEnd = NULL;
void __fastcall detoured_sceneEnd(uint32_t CGxDevice, void* ignored) {
    if (*reinterpret_cast<uint32_t*>(CGxDevice + 0x3a38) != NULL
        && scene_isEnabled == true
        && scene_fontsOnLost == false && scene_needReloadFont == false) {
        LPDIRECT3DDEVICE9 dxDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(vanilla1121_d3dDevice(CGxDevice));
        if (dxDevice == NULL || (reinterpret_cast<uintptr_t>(dxDevice) & 1) != 0) {
            p_original_sceneEnd(CGxDevice);
            return;
        }
        if (dxDevice != scene_lastDXdevice) {
            scene_lastDXdevice = dxDevice;
            scene_needReloadFont = true;
            p_original_sceneEnd(CGxDevice);
            return;
        }

        if (scene_fallback == NULL || scene_fallbackBIG == NULL || scene_fallbackSmall == NULL || scene_fallbackHUGE == NULL
            || scene_selected == NULL || scene_selectedBIG == NULL || scene_selectedSmall == NULL || scene_selectedHUGE == NULL) {
            scene_needReloadFont = true;
            p_original_sceneEnd(CGxDevice);
            return;
        }

        // First iteration does not draw but only update RECT for overlapping test
        for (auto it = critTexts.begin(); it != critTexts.end();) {
            int update = -1;
            if (it->second.m_serif) {
                update = it->second.update(scene_selected, scene_selectedBIG, scene_selectedHUGE, scene_lastDXdevice);
            }
            else {
                update = it->second.update(scene_fallback, scene_fallbackBIG, scene_fallbackHUGE, scene_lastDXdevice);
            }

            if (update == -1) {
                it = critTexts.erase(it);
                continue;
            }
            it++;
        }

        for (auto it = smallFloatingTexts.begin(); it != smallFloatingTexts.end();) {
            int update = -1;
            if (it->m_serif) {
                update = it->update(scene_selectedSmall, scene_lastDXdevice);
            }
            else {
                update = it->update(scene_fallbackSmall, scene_lastDXdevice);
            }

            if (update == -1) {
                it = smallFloatingTexts.erase(it);
                continue;
            }

            if (update > 0) {
                for (auto jt = critTexts.begin(); jt != critTexts.end(); jt++) {
                    RECT r = {};
                    if (IntersectRect(&r, &(jt->second.m_rect), &(it->m_rect)) != 0) {
                        update = 0;
                        break;
                    }
                }
            }

            if (update > 0) {
                it->draw();
            }
            it++;
        }


        for (auto it = floatingTexts.begin(); it != floatingTexts.end();) {
            int update = -1;
            if (it->m_serif) {
                update = it->update(scene_selected, scene_lastDXdevice);
            }
            else {
                update = it->update(scene_fallback, scene_lastDXdevice);
            }

            if (update == -1) {
                it = floatingTexts.erase(it);
                continue;
            }

            if (update > 0) {
                for (auto jt = critTexts.begin(); jt != critTexts.end(); jt++) {
                    RECT r = {};
                    if (IntersectRect(&r, &(jt->second.m_rect), &(it->m_rect)) != 0) {
                        update = 0;
                        break;
                    }
                }
            }

            if (update > 0) {
                it->draw();
            }
            it++;
        }


        for (auto it = critTexts.begin(); it != critTexts.end();) {
            int update = -1;
            if (it->second.m_serif) {
                update = it->second.update(scene_selected, scene_selectedBIG, scene_selectedHUGE, scene_lastDXdevice);
            }
            else {
                update = it->second.update(scene_fallback, scene_fallbackBIG, scene_fallbackHUGE, scene_lastDXdevice);
            }

            if (update == -1) {
                it = critTexts.erase(it);
                continue;
            }
            if (update > 0) {
                it->second.draw();
            }
            it++;
        }

    }

    p_original_sceneEnd(CGxDevice);
}

static void sortAddNewFloatingText(worldText::Floating& newText, std::list<worldText::Floating>& list, std::list<worldText::Floating>& secondListToCheck) {
    int maxOverlapHeight = -1;
    for (auto it = list.begin(); it != list.end(); ++it) {
        RECT r = {};
        if (IntersectRect(&r, &newText.m_rect, &it->m_rect)) {
            int intersectHeight = r.bottom - r.top;
            if (intersectHeight > maxOverlapHeight) {
                maxOverlapHeight = intersectHeight;
            }
        }
    }

    if (maxOverlapHeight > 0) {
        for (auto it = list.begin(); it != list.end(); ++it) {
            it->fastForward(maxOverlapHeight);
        }
    }

    list.push_back(newText);

    maxOverlapHeight = -1;
    for (auto it = secondListToCheck.begin(); it != secondListToCheck.end(); ++it) {
        RECT r = {};
        if (IntersectRect(&r, &newText.m_rect, &it->m_rect)) {
            int intersectHeight = r.bottom - r.top;
            if (intersectHeight > maxOverlapHeight) {
                maxOverlapHeight = intersectHeight;
            }
        }
    }

    if (maxOverlapHeight > 0) {
        for (auto it = secondListToCheck.begin(); it != secondListToCheck.end(); ++it) {
            it->fastForward(maxOverlapHeight);
        }
    }
}

static std::unordered_map<int, std::string> worldTextHistory{};
CREATEWORLDTEXT p_createWorldText = reinterpret_cast<CREATEWORLDTEXT>(0x6c73f0);
CREATEWORLDTEXT p_original_createWorldText = NULL;
bool scene_useXP3combatText = false;
void __fastcall detoured_createWorldText(uint32_t self, void* ignored, int type, char const* text, uint32_t color, uint32_t unknown) {
    if (scene_hideEXPtext && type == 4) {
        return;
    }

    if (false == scene_useXP3combatText) {
        p_original_createWorldText(self, type, text, color, unknown);
        return;
    }

    if (false == scene_isEnabled) {
        p_original_createWorldText(self, type, text, color, unknown);
        return;
    }

    if (scene_lastDXdevice == NULL || scene_fallback == NULL || scene_fallbackBIG == NULL || scene_fallbackHUGE == NULL || scene_fallbackSmall == NULL
        || scene_selected == NULL || scene_selectedBIG == NULL || scene_selectedHUGE == NULL || scene_selectedSmall == NULL) {
        p_original_createWorldText(self, type, text, color, unknown);
        return;
    }

    uint64_t stickToGUID = *reinterpret_cast<uint64_t*>(self + 0x10);
    if (stickToGUID == 0) {
        stickToGUID = UnitGUID("player");
    }

    ID3DXFont* font = scene_fallback;
    ID3DXFont* fontBIG = scene_fallbackBIG;
    ID3DXFont* fontHUGE = scene_fallbackHUGE;
    bool serif = false;
    if (utf8_canBeRendered(utf8_to_utf16(text))) {
        font = scene_selected;
        fontBIG = scene_selectedBIG;
        fontHUGE = scene_selectedHUGE;
        serif = true;
    }

    int r = 255;
    int g = 255;
    int b = 255;
    if (color != 0 && (color & 1) == 0) {
        r = (color >> 8) & 0xff;
        g = (color >> 16) & 0xff;
        b = (color >> 24) & 0xff;
    }

    // type:
    // 0 attack/heal
    // 1 absorb
    // 2 crit
    // 3 dodge
    // 4 exp
    // 5 honor
    switch (type) {
    case 0:
    case 1:
    case 3:
    case 4:
    case 5:
    {
        if (type == 4) {
            r = 0xff;
            g = 0x33;
            b = 0xcc;
        }
        else if (type == 5) {
            r = 239;
            g = 191;
            b = 4;
        }

        worldText::Floating newText(text, stickToGUID, r, g, b, 255, worldText::up, font, serif, scene_lastDXdevice);
        sortAddNewFloatingText(newText, floatingTexts, smallFloatingTexts);
        return;
    }
    case 2:
    {
        worldText::Crit newCrit(text, stickToGUID, r, g, b, 255, font, fontBIG, fontHUGE, serif, scene_lastDXdevice);

        auto it = critTexts.find(stickToGUID);
        if (it != critTexts.end()) {
            critTexts.erase(it);
        }

        critTexts.insert({ stickToGUID, newCrit });
        return;
    }
    default:
    {
        std::stringstream ss{};
        ss << type << " " << std::string(text);

        if (color != 0 && (color & 1) == 0) {
            uint32_t c = *reinterpret_cast<uint32_t*>(color);
            ss << " (" << c << ")";
        }
        uint64_t testGUID = *reinterpret_cast<uint64_t*>(self + 0x10);
        ss << "0x" << testGUID << " ";
        worldTextHistory.insert({ type, ss.str() });
        p_original_createWorldText(self, type, text, color, unknown);
        return;
    }
    }
}

void scene_addSmallFloatingText(std::string text, int r, int g, int b, int a, worldText::FLOATING_DIRECTION direction) {
    if (scene_isEnabled == false) {
        return;
    }
    if (scene_lastDXdevice == NULL || scene_fallback == NULL || scene_fallbackBIG == NULL || scene_fallbackHUGE == NULL || scene_fallbackSmall == NULL
        || scene_selected == NULL || scene_selectedBIG == NULL || scene_selectedHUGE == NULL || scene_selectedSmall == NULL) {
        return;
    }

    ID3DXFont* font = scene_fallbackSmall;
    bool serif = false;
    if (utf8_canBeRendered(utf8_to_utf16(text))) {
        font = scene_selectedSmall;
        serif = true;
    }

    worldText::Floating newText(text, UnitGUID("player"), r, g, b, a, direction, font, serif, scene_lastDXdevice);
    sortAddNewFloatingText(newText, smallFloatingTexts, floatingTexts);
}

void scene_addCritText(std::string text, int r, int g, int b, int a) {
    if (scene_isEnabled == false) {
        return;
    }
    if (scene_lastDXdevice == NULL || scene_fallback == NULL || scene_fallbackBIG == NULL || scene_fallbackHUGE == NULL || scene_fallbackSmall == NULL
        || scene_selected == NULL || scene_selectedBIG == NULL || scene_selectedHUGE == NULL || scene_selectedSmall == NULL) {
        return;
    }

    ID3DXFont* font = scene_fallback;
    ID3DXFont* fontBIG = scene_fallbackBIG;
    ID3DXFont* fontHUGE = scene_fallbackHUGE;
    bool serif = false;
    if (utf8_canBeRendered(utf8_to_utf16(text))) {
        font = scene_selected;
        fontBIG = scene_selectedBIG;
        fontHUGE = scene_selectedHUGE;
        serif = true;
    }

    uint64_t player = UnitGUID("player");

    D3DCOLOR color = D3DCOLOR_ARGB(a, r, g, b);
    worldText::Crit newCrit(text, player, r, g, b, a, font, fontBIG, fontHUGE, serif, scene_lastDXdevice);

    auto it = critTexts.find(player);
    if (it != critTexts.end()) {
        critTexts.erase(it);
    }

    critTexts.insert({ player, newCrit });
}

std::string scene_debugText() {
    std::stringstream ss{};

    if (worldTextHistory.size() > 0) {
        ss << "Unimplemented world text history:";
        for (auto& i : worldTextHistory) {
            ss << std::endl << i.second;
        }
    }

    if (scene_disableReason.length() > 0) {
        ss << scene_disableReason << std::endl;
    }

    return ss.str();
}
