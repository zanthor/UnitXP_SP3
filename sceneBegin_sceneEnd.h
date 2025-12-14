#pragma once

#include <unordered_map>
#include <list>
#include <string>

#include <d3d9.h>

#include "worldText.h"

typedef HRESULT(WINAPI* LPD3DXCREATEFFONTW)(
    LPDIRECT3DDEVICE9       pDevice,
    INT                     Height,
    UINT                    Width,
    UINT                    Weight,
    UINT                    MipLevels,
    BOOL                    Italic,
    DWORD                   CharSet,
    DWORD                   OutputPrecision,
    DWORD                   Quality,
    DWORD                   PitchAndFamily,
    LPCWSTR                 pFaceName,
    LPD3DXFONT* ppFont);
extern LPD3DXCREATEFFONTW p_D3DXCreateFontW;
extern bool scene_isEnabled;
extern bool scene_useXP3combatText;
extern bool scene_hideEXPtext;
extern int scene_fontSize;
extern LPDIRECT3DDEVICE9 scene_lastDXdevice;
extern std::string scene_userSelectedFontName;
// It seems d3d9 would crash if call reloadFont() between BeginScene() and EndScene(), so here is a bool switch to delay reloadFont() till Present() is done.
extern bool scene_needReloadFont;
extern bool scene_fontsOnLost;
void scene_init();
void scene_end();
bool scene_reloadFont();
void scene_addSmallFloatingText(std::string text, int r, int g, int b, int a, worldText::FLOATING_DIRECTION direction);
void scene_addCritText(std::string text, int r, int g, int b, int a);
std::string scene_debugText();

// The technique of hooking __thiscall function is from: https://tresp4sser.wordpress.com/2012/10/06/how-to-hook-thiscall-functions/
// -- Pointer is __thiscall with 1st param being THIS
// -- The detoured function is __fastcall with 1st param being THIS, and 2nd param being IGNORED

typedef void(__thiscall* ISCENEBEGIN)(uint32_t, uint32_t);
extern ISCENEBEGIN p_sceneBegin;
extern ISCENEBEGIN p_original_sceneBegin;
void __fastcall detoured_sceneBegin(uint32_t CGxDevice, void* ignored, uint32_t unknown);

typedef void(__thiscall* ISCENEEND)(uint32_t);
extern ISCENEEND p_sceneEnd;
extern ISCENEEND p_original_sceneEnd;
void __fastcall detoured_sceneEnd(uint32_t CGxDevice, void* ignored);

typedef void(__thiscall* CREATEWORLDTEXT)(uint32_t, int, char const*, uint32_t, uint32_t);
extern CREATEWORLDTEXT p_createWorldText;
extern CREATEWORLDTEXT p_original_createWorldText;
void __fastcall detoured_createWorldText(uint32_t self, void* ignored, int type, char const* text, uint32_t color, uint32_t unknown);
