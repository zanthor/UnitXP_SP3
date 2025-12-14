#pragma once

#include <string>

#include <Windows.h>
#include <d3dx9.h>

#include "performanceProfiling.h"

namespace worldText {
    extern const int animationFPS;
    extern double nameplateHeight;

    enum FLOATING_DIRECTION { up, down, arc };

    class Floating {
    public:
        Floating(std::string text, uint64_t stickToGUID, int r, int g, int b, int a, FLOATING_DIRECTION direction, ID3DXFont* font, bool serif, LPDIRECT3DDEVICE9 device);

        // Return 1 = draw; 0 = invisible; -1 = end
        // As device might be lost during animation, we would update it
        int update(ID3DXFont* font, LPDIRECT3DDEVICE9 device);
        void draw();
        void fastForward(int ffDistance);

        RECT m_rect;
        bool m_serif;
    private:
        std::string m_text;
        uint64_t m_stickToGUID;
        uint64_t m_playerGUID;
        int m_r;
        int m_g;
        int m_b;
        int m_a;
        LPDIRECT3DDEVICE9 m_device;
        ID3DXFont* m_font;
        LARGE_INTEGER m_startTime;
        int m_width;
        int m_height;
        LARGE_INTEGER m_timingPrecision;
        double m_totalTime;
        double m_fadeOutTime;
        int m_floatingDistance;
        int m_shadowWeight;
        int m_offsetY;
        int m_offsetX;
        double m_arcRadius;
        int m_arcTowardsRight;
        double m_nameplatesOffset;
        FLOATING_DIRECTION m_direction;

        // The default copy constructor won't increase this, but we don't need it anyway.
        static uint64_t m_instanceCount;

        // Additional alpha blending
        double m_alpha;
    };

    class Crit {
    public:
        Crit(std::string text, uint64_t stickToGUID, int r, int g, int b, int a, ID3DXFont* fontNormal, ID3DXFont* fontBig, ID3DXFont* fontHuge, bool serif, LPDIRECT3DDEVICE9 device);

        // Return 1 = draw; 0 = invisible; -1 = end
        // As device might be lost during animation, we would update it
        int update(ID3DXFont* fontNormal, ID3DXFont* fontBig, ID3DXFont* fontHuge, LPDIRECT3DDEVICE9 device);

        void draw();

        RECT m_rect;
        bool m_serif;
    private:
        std::string m_text;
        uint64_t m_stickToGUID;
        uint64_t m_playerGUID;
        int m_r;
        int m_g;
        int m_b;
        int m_a;

        // Additional alpha blending
        double m_alpha;
        double m_alpha_forHugeFont;
        ID3DXFont* m_fontNormal;
        ID3DXFont* m_fontBig;
        ID3DXFont* m_fontHuge;
        ID3DXFont* m_fontDraw;
        LPDIRECT3DDEVICE9 m_device;
        LARGE_INTEGER m_startTime;
        LARGE_INTEGER m_timingPrecision;
        double m_totalTime;
        double m_fadeOutTime;
        double m_nameplatesOffset;
        int m_shadowWeight;
        int m_floatingDistance;
        int m_width;
        int m_height;
    };
}
