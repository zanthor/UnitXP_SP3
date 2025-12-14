#pragma once

#include <string>

std::wstring utf8_to_utf16(const std::string& utf8);

// This function use void* so that we could include <d3d9x.h> less
void utf8_reloadRenderRanges(void* d3dxFont);
bool utf8_canBeRendered(const std::wstring& utf16);

void utf8_clearRenderRanges();
