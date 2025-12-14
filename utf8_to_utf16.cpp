#include "pch.h"

#include <cstdlib>
#include <string>

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "utf8_to_utf16.h"

static LPGLYPHSET renderRanges = NULL;

void utf8_clearRenderRanges() {
    if (renderRanges != NULL) {
        std::free(renderRanges);
        renderRanges = NULL;
    }
}

// By ChatGPT
std::wstring utf8_to_utf16(const std::string& utf8)
{
    if (utf8.empty())
        return std::wstring();

    // Calculate the size of the destination buffer
    int size_needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        utf8.data(), static_cast<int>(utf8.size()),
        nullptr, 0);
    if (size_needed == 0)
    {
        MessageBoxW(NULL, L"MultiByteToWideChar failed: invalid UTF-8 or system error.", L"UnitXP Service Pack 3", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
        return std::wstring();
    }

    std::wstring utf16(size_needed, 0);

    // Perform the actual conversion
    int result = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
        utf8.data(), static_cast<int>(utf8.size()),
        &utf16[0], size_needed);
    if (result == 0)
    {
        MessageBoxW(NULL, L"MultiByteToWideChar failed during conversion.", L"UnitXP Service Pack 3", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
        return std::wstring();
    }

    return utf16;
}

bool utf8_canBeRendered(const std::wstring& utf16) {
    if (renderRanges == NULL) {
        return false;
    }

    for (wchar_t ch : utf16) {
        bool flag = false;
        for (DWORD ri = 0; ri < renderRanges->cRanges; ++ri) {
            wchar_t start = renderRanges->ranges[ri].wcLow;
            uint32_t total = renderRanges->ranges[ri].cGlyphs;
            if (ch >= start && ch < start + total) {
                flag = true;
                break;
            }
        }
        if (flag == false) {
            return false;
        }
    }

    return true;
}

void utf8_reloadRenderRanges(void* d3dxFont) {
    if (d3dxFont == NULL) {
        return;
    }
    utf8_clearRenderRanges();
    
    ID3DXFont* font = reinterpret_cast<ID3DXFont*>(d3dxFont);

    DWORD size = GetFontUnicodeRanges(font->GetDC(), NULL);
    // This comparison is to hint static analyzer that there isn't buffer overrun on renderRanges
    if (size < sizeof GLYPHSET) {
        return;
    }
    renderRanges = reinterpret_cast<LPGLYPHSET>(std::malloc(size));
    if (renderRanges == NULL) {
        return;
    }
    renderRanges->cbThis = size;

    DWORD ret = GetFontUnicodeRanges(font->GetDC(), renderRanges);
    if (ret == 0) {
        utf8_clearRenderRanges();
        return;
    }
}
