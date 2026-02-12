// ITexture.h: Abstract interface for texture/surface objects
//
// Part of DDrawEngine static library
// This interface wraps DirectDraw surfaces or equivalent in other backends
//////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _WIN32
#include <windows.h>
#endif
#include <cstdint>
#include "SpriteTypes.h"

namespace hb::shared::render {

class ITexture
{
public:
    virtual ~ITexture() = default;

    // ============== Dimensions ==============
    virtual uint16_t GetWidth() const = 0;
    virtual uint16_t GetHeight() const = 0;

    // ============== Blitting ==============
    virtual bool Blt(RECT* destRect, ITexture* src, RECT* srcRect, uint32_t flags) = 0;
    virtual bool BltFast(int x, int y, ITexture* src, RECT* srcRect, uint32_t flags) = 0;

    // ============== Native Handle ==============
    // Platform-specific handle for transition period
    // DirectDraw: returns LPDIRECTDRAWSURFACE7
    virtual void* GetNativeHandle() = 0;

    // ============== Restoration ==============
    // DirectDraw surfaces can be lost and need restoration
    virtual bool IsLost() const = 0;
    virtual bool Restore() = 0;
};

// Texture flags for blitting operations
// These map to DirectDraw DDBLT and DDBLTFAST flags
namespace TextureFlags
{
    constexpr uint32_t WAIT        = 0x00000001;  // DDBLT_WAIT / DDBLTFAST_WAIT
    constexpr uint32_t SRCCOLORKEY = 0x00000002;  // DDBLT_KEYSRC / DDBLTFAST_SRCCOLORKEY
    constexpr uint32_t DSTCOLORKEY = 0x00000004;  // DDBLT_KEYDEST / DDBLTFAST_DESTCOLORKEY
    constexpr uint32_t NOCOLORKEY  = 0x00000008;  // DDBLTFAST_NOCOLORKEY
}

} // namespace hb::shared::render
