// SFMLSprite.cpp: SFML implementation of ISprite interface
//
// Part of SFMLEngine static library
//////////////////////////////////////////////////////////////////////

#include "SFMLSprite.h"
#include "SFMLRenderer.h"
#include "CommonTypes.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTexture.hpp>


SFMLSprite::SFMLSprite(SFMLRenderer* pRenderer, const std::string& pakFilePath, int spriteIndex, bool alphaEffect)
    : m_pRenderer(pRenderer)
    , m_pakFilePath(pakFilePath)
    , m_spriteIndex(spriteIndex)
    , m_bitmapWidth(0)
    , m_bitmapHeight(0)
    , m_textureLoaded(false)
    , m_alphaEffect(alphaEffect)
    , m_ambient_light_level(1)
    , m_inUse(false)
    , m_lastAccessTime(0)
{
    // Load sprite data from PAK file
    PAKLib::sprite spriteData = PAKLib::get_sprite_fast(pakFilePath, spriteIndex);
    InitFromSpriteData(spriteData);
}

SFMLSprite::SFMLSprite(SFMLRenderer* pRenderer, const PAKLib::sprite& spriteData, bool alphaEffect)
    : m_pRenderer(pRenderer)
    , m_spriteIndex(0)
    , m_bitmapWidth(0)
    , m_bitmapHeight(0)
    , m_textureLoaded(false)
    , m_alphaEffect(alphaEffect)
    , m_ambient_light_level(1)
    , m_inUse(false)
    , m_lastAccessTime(0)
{
    InitFromSpriteData(spriteData);
}

SFMLSprite::~SFMLSprite()
{
    Unload();
}

void SFMLSprite::InitFromSpriteData(const PAKLib::sprite& spriteData)
{
    // Copy frame data
    m_frames.clear();
    m_frames.reserve(spriteData.sprite_rectangles.size());
    for (const auto& rect : spriteData.sprite_rectangles)
    {
        m_frames.push_back(rect);
    }

    // Copy image data (PNG format) - will be cleared after texture creation
    m_imageData = spriteData.image_data;

    // Don't create texture yet - lazy load on first draw to save memory
    // CreateTexture();
}

bool SFMLSprite::CreateTexture()
{
    if (m_textureLoaded)
        return true;

    if (m_imageData.empty())
        return false;

    // Load image data - SFML handles PNG natively with alpha channel support
    if (!m_collisionImage.loadFromMemory(m_imageData.data(), m_imageData.size()))
        return false;

    // Get dimensions from loaded image
    sf::Vector2u size = m_collisionImage.getSize();
    m_bitmapWidth = static_cast<uint16_t>(size.x);
    m_bitmapHeight = static_cast<uint16_t>(size.y);

    // Create texture from image
    if (!m_texture.loadFromImage(m_collisionImage))
        return false;

    // Disable smooth filtering to prevent edge artifacts and ensure crisp pixel art
    m_texture.setSmooth(false);

    // Clear PNG data to free memory - we have the texture and collision image now
    m_imageData.clear();
    m_imageData.shrink_to_fit();

    m_textureLoaded = true;
    return true;
}

void SFMLSprite::Draw(int x, int y, int frame, const hb::shared::sprite::DrawParams& params)
{
    // Lazy load texture on first draw
    if (!m_textureLoaded)
        CreateTexture();

    if (!m_textureLoaded || !m_pRenderer || frame < 0 || frame >= static_cast<int>(m_frames.size()))
        return;

    // Sync alpha degree with global (day/night mode)
    if (m_alphaEffect && m_ambient_light_level != m_pRenderer->GetAmbientLightLevel())
    {
        m_ambient_light_level = m_pRenderer->GetAmbientLightLevel();
    }

    m_inUse = true;
    m_lastAccessTime = GameClock::GetTimeMS();

    DrawInternal(m_pRenderer->GetBackBuffer(), x, y, frame, params);

    m_inUse = false;
}

void SFMLSprite::DrawToSurface(void* destSurface, int x, int y, int frame, const hb::shared::sprite::DrawParams& params)
{
    // Lazy load texture on first draw
    if (!m_textureLoaded)
        CreateTexture();

    if (!destSurface || !m_textureLoaded || frame < 0 || frame >= static_cast<int>(m_frames.size()))
        return;

    // Sync alpha degree with global (day/night mode)
    if (m_alphaEffect && m_ambient_light_level != m_pRenderer->GetAmbientLightLevel())
    {
        m_ambient_light_level = m_pRenderer->GetAmbientLightLevel();
    }

    m_inUse = true;
    m_lastAccessTime = GameClock::GetTimeMS();

    sf::RenderTexture* target = static_cast<sf::RenderTexture*>(destSurface);
    DrawInternal(target, x, y, frame, params);

    m_inUse = false;
}

void SFMLSprite::DrawInternal(sf::RenderTexture* target, int x, int y, int frame, const hb::shared::sprite::DrawParams& params)
{
    if (!target)
        return;

    const PAKLib::sprite_rect& frameRect = m_frames[frame];

    // Shadow rendering - replicate DDraw's skewed shadow effect
    if (params.isShadow)
    {
        // DDraw shadow algorithm:
        // - Reads from bottom of sprite going up
        // - Every 3 source rows -> 1 shadow row (1/3 height)
        // - Skews left by 1 pixel per row going UP
        // - Shadow base (bottom) is anchored at sprite's feet

        int drawX = x + frameRect.pivotX;
        int drawY = y + frameRect.pivotY;

        // Create sprite for this frame
        sf::IntRect srcRect(
            {static_cast<int>(frameRect.x), static_cast<int>(frameRect.y)},
            {static_cast<int>(frameRect.width), static_cast<int>(frameRect.height)}
        );

        sf::Sprite sprite(m_texture, srcRect);

        // Semi-transparent black for shadow
        // At night (m_pRenderer->GetAmbientLightLevel() == 2), reduce shadow opacity by 50%
        uint8_t shadowAlpha = (m_pRenderer->GetAmbientLightLevel() == 2) ? 70 : 140;
        sprite.setColor(sf::Color(0, 0, 0, shadowAlpha));

        // Set origin to bottom-left of sprite so transforms anchor there (at feet)
        sprite.setOrigin({0.0f, static_cast<float>(frameRect.height)});

        // Shadow dimensions
        float scaleY = 1.0f / 3.0f;
        float shadowHeight = frameRect.height * scaleY;

        // Position: shadow base at sprite's feet (bottom of sprite)
        float baseX = static_cast<float>(drawX);
        float baseY = static_cast<float>(drawY + frameRect.height);

        // Build transform:
        // 1. Translate to feet position
        // 2. Apply shear (skew left going up) and vertical scale
        //
        // The shear matrix skews X based on Y. Since origin is at bottom,
        // and Y goes negative (upward), we want positive shear to skew left.

        sf::Transform transform;
        transform.translate({baseX, baseY});

        // Shear and scale matrix (origin at bottom-left):
        // x' = x + shearX * y
        // y' = scaleY * y
        // Since y is negative (going up from origin), positive shearX moves left
        sf::Transform skewScale(
            1.0f, 1.0f, 0.0f,   // x' = x + 1*y (shear)
            0.0f, scaleY, 0.0f, // y' = scaleY * y
            0.0f, 0.0f, 1.0f
        );
        transform.combine(skewScale);

        sf::RenderStates states;
        states.transform = transform;
        states.blendMode = sf::BlendAlpha;

        target->draw(sprite, states);

        // Update bounds (approximate)
        m_boundRect.left = drawX;
        m_boundRect.top = static_cast<int>(baseY - shadowHeight);
        m_boundRect.right = drawX + frameRect.width;
        m_boundRect.bottom = static_cast<int>(baseY);

        return;
    }

    // Calculate draw position (apply pivot) - must ADD pivot like DDraw does
    int drawX = x + frameRect.pivotX;
    int drawY = y + frameRect.pivotY;

    // Update bounds
    m_boundRect.left = drawX;
    m_boundRect.top = drawY;
    m_boundRect.right = drawX + frameRect.width;
    m_boundRect.bottom = drawY + frameRect.height;

    // Clip to render target bounds - SFML doesn't auto-clip like DDraw
    // Get actual target size
    sf::Vector2u targetSize = target->getSize();
    int targetWidth = static_cast<int>(targetSize.x);
    int targetHeight = static_cast<int>(targetSize.y);

    int srcX = frameRect.x;
    int srcY = frameRect.y;
    int srcW = frameRect.width;
    int srcH = frameRect.height;

    // Clip left edge
    if (drawX < 0)
    {
        srcX -= drawX;
        srcW += drawX;
        drawX = 0;
    }
    // Clip top edge
    if (drawY < 0)
    {
        srcY -= drawY;
        srcH += drawY;
        drawY = 0;
    }
    // Clip right edge
    if (drawX + srcW > targetWidth)
    {
        srcW = targetWidth - drawX;
    }
    // Clip bottom edge
    if (drawY + srcH > targetHeight)
    {
        srcH = targetHeight - drawY;
    }

    // Skip if completely clipped
    if (srcW <= 0 || srcH <= 0)
        return;

    // Create sprite for this frame with clipped source rect
    sf::IntRect srcRect(
        {srcX, srcY},
        {srcW, srcH}
    );

    sf::Sprite sprite(m_texture, srcRect);
    sprite.setPosition({static_cast<float>(drawX), static_cast<float>(drawY)});

    // Check if we need any color modifications
    bool needsColorChange = (params.alpha < 1.0f) ||
                            (params.tintR != 0 || params.tintG != 0 || params.tintB != 0) ||
                            params.isShadow || params.isFade ||
                            (m_ambient_light_level == 2 && m_alphaEffect);

    if (needsColorChange)
    {
        sf::Color color = sf::Color::White;

        // Apply alpha
        if (params.alpha < 1.0f)
        {
            color.a = static_cast<uint8_t>(params.alpha * 255.0f);
        }

        // Apply tint: SFML multiplies sprite pixels by color/255
        // (255,255,255) = no change, (0,0,0) = black, (255,200,0) = gold
        if (params.tintR != 0 || params.tintG != 0 || params.tintB != 0)
        {
            int r = params.tintR;
            int g = params.tintG;
            int b = params.tintB;

            // Clamp to valid range
            color.r = static_cast<uint8_t>(r < 0 ? 0 : (r > 255 ? 255 : r));
            color.g = static_cast<uint8_t>(g < 0 ? 0 : (g > 255 ? 255 : g));
            color.b = static_cast<uint8_t>(b < 0 ? 0 : (b > 255 ? 255 : b));
        }

        // Apply shadow effect (darken)
        // Use semi-transparent black with alpha blending - the sprite's own alpha
        // channel will mask where the shadow appears (only on non-transparent pixels)
        if (params.isShadow)
        {
            color.r = 0;
            color.g = 0;
            color.b = 0;
            color.a = 64;  // Semi-transparent black for shadow darkening
        }

        // Apply fade effect - darkens destination like DDraw's DrawFadeInternal
        // At night (m_pRenderer->GetAmbientLightLevel() == 2), reduce fade intensity by 50%
        // Day: 75% alpha -> 25% brightness | Night: 50% alpha -> 50% brightness
        if (params.isFade)
        {
            color.r = 0;
            color.g = 0;
            color.b = 0;
            color.a = (m_pRenderer->GetAmbientLightLevel() == 2) ? 127 : 191;  // Less darkening at night
        }

        // Apply alpha degree darkening (night mode)
        if (m_ambient_light_level == 2 && m_alphaEffect)
        {
            color.r = static_cast<uint8_t>(color.r * 0.7f);
            color.g = static_cast<uint8_t>(color.g * 0.7f);
            color.b = static_cast<uint8_t>(color.b * 0.7f);
        }

        sprite.setColor(color);
    }

    // Set up render states based on blend mode
    sf::RenderStates states;
    bool wasSmooth = m_texture.isSmooth();

    if (params.blendMode == hb::shared::sprite::BlendMode::Additive) {
        states.blendMode = sf::BlendAdd;
        // Enable smooth filtering for additive blending (light effects)
        // This produces smooth gradients like DDraw's per-pixel blending
        if (!wasSmooth) {
            m_texture.setSmooth(true);
        }
    } else if (params.blendMode == hb::shared::sprite::BlendMode::Average) {
        // 50/50 averaging: result = (src + dst) / 2
        // Achieved with alpha blending at 50% opacity
        states.blendMode = sf::BlendAlpha;
        sf::Color color = sprite.getColor();
        color.a = 127;  // 50% alpha for averaging blend
        sprite.setColor(color);
    } else {
        states.blendMode = sf::BlendAlpha;
    }

    // Draw the sprite
    target->draw(sprite, states);

    // Restore smooth setting if we changed it
    if (params.blendMode == hb::shared::sprite::BlendMode::Additive && !wasSmooth) {
        m_texture.setSmooth(false);
    }
}

void SFMLSprite::DrawWidth(int x, int y, int frame, int width, bool vertical)
{
    // Lazy load texture on first draw
    if (!m_textureLoaded)
        CreateTexture();

    if (!m_textureLoaded || !m_pRenderer || frame < 0 || frame >= static_cast<int>(m_frames.size()))
        return;

    // Sync alpha degree with global (day/night mode)
    if (m_alphaEffect && m_ambient_light_level != m_pRenderer->GetAmbientLightLevel())
    {
        m_ambient_light_level = m_pRenderer->GetAmbientLightLevel();
    }

    m_inUse = true;
    m_lastAccessTime = GameClock::GetTimeMS();

    const PAKLib::sprite_rect& frameRect = m_frames[frame];

    int drawX = x + frameRect.pivotX;
    int drawY = y + frameRect.pivotY;

    // Clamp width
    int maxSize = vertical ? frameRect.height : frameRect.width;
    if (width > maxSize) width = maxSize;
    if (width < 0) width = 0;

    // Create partial source rect
    sf::IntRect srcRect;
    if (vertical)
    {
        srcRect = sf::IntRect(
            {static_cast<int>(frameRect.x), static_cast<int>(frameRect.y)},
            {static_cast<int>(frameRect.width), width}
        );
    }
    else
    {
        srcRect = sf::IntRect(
            {static_cast<int>(frameRect.x), static_cast<int>(frameRect.y)},
            {width, static_cast<int>(frameRect.height)}
        );
    }

    sf::Sprite sprite(m_texture, srcRect);
    sprite.setPosition({static_cast<float>(drawX), static_cast<float>(drawY)});

    // Use explicit alpha blending to respect PNG transparency
    m_pRenderer->GetBackBuffer()->draw(sprite, sf::RenderStates(sf::BlendAlpha));

    // Update bounds
    m_boundRect.left = drawX;
    m_boundRect.top = drawY;
    m_boundRect.right = drawX + (vertical ? frameRect.width : width);
    m_boundRect.bottom = drawY + (vertical ? width : frameRect.height);

    m_inUse = false;
}

void SFMLSprite::DrawShifted(int x, int y, int shiftX, int shiftY, int frame, const hb::shared::sprite::DrawParams& params)
{
    // DrawShifted draws a 128x128 subregion of the sprite starting at (shiftX, shiftY)
    // This is used for the guide map to show a viewport window into a large map sprite
    // Matches DDraw's CSprite::PutShiftSpriteFast / PutShiftTransSprite2

    // Lazy load texture on first draw (same as Draw())
    if (!m_textureLoaded)
        CreateTexture();

    if (!m_textureLoaded || !m_pRenderer || frame < 0 || frame >= static_cast<int>(m_frames.size()))
        return;

    // Sync alpha degree with global (day/night mode)
    if (m_alphaEffect && m_ambient_light_level != m_pRenderer->GetAmbientLightLevel())
    {
        m_ambient_light_level = m_pRenderer->GetAmbientLightLevel();
    }

    sf::RenderTexture* target = m_pRenderer->GetBackBuffer();
    if (!target)
        return;

    m_inUse = true;

    const PAKLib::sprite_rect& frameRect = m_frames[frame];

    // Source position within the sprite texture - start at frame origin and add shift
    int srcX = frameRect.x + shiftX;
    int srcY = frameRect.y + shiftY;
    int srcW = 128;  // Guide map viewport is always 128x128
    int srcH = 128;

    // Destination position with pivot offsets (matching DDraw implementation)
    int drawX = x + frameRect.pivotX;
    int drawY = y + frameRect.pivotY;

    // Get render target size for clipping
    sf::Vector2u targetSize = target->getSize();
    int targetWidth = static_cast<int>(targetSize.x);
    int targetHeight = static_cast<int>(targetSize.y);

    // Clip to screen boundaries (matching DDraw's clipping logic)
    if (drawX < 0)
    {
        int diff = -drawX;
        srcX += diff;
        srcW -= diff;
        if (srcW <= 0) { m_inUse = false; return; }
        drawX = 0;
    }
    else if (drawX + srcW > targetWidth)
    {
        srcW -= (drawX + srcW) - targetWidth;
        if (srcW <= 0) { m_inUse = false; return; }
    }

    if (drawY < 0)
    {
        int diff = -drawY;
        srcY += diff;
        srcH -= diff;
        if (srcH <= 0) { m_inUse = false; return; }
        drawY = 0;
    }
    else if (drawY + srcH > targetHeight)
    {
        srcH -= (drawY + srcH) - targetHeight;
        if (srcH <= 0) { m_inUse = false; return; }
    }

    // Update bounds
    m_boundRect.left = drawX;
    m_boundRect.top = drawY;
    m_boundRect.right = drawX + srcW;
    m_boundRect.bottom = drawY + srcH;

    // Create sprite with the shifted source rectangle
    sf::IntRect srcRect(
        {srcX, srcY},
        {srcW, srcH}
    );
    sf::Sprite sprite(m_texture, srcRect);

    // Apply alpha if specified
    if (params.alpha < 1.0f)
    {
        std::uint8_t alphaVal = static_cast<std::uint8_t>(params.alpha * 255.0f);
        sprite.setColor(sf::Color(255, 255, 255, alphaVal));
    }

    // Position at destination
    sprite.setPosition({static_cast<float>(drawX), static_cast<float>(drawY)});

    // Draw to target
    sf::RenderStates states;
    states.blendMode = params.alpha < 1.0f ? sf::BlendAlpha : sf::BlendNone;
    target->draw(sprite, states);

    m_inUse = false;
}

int SFMLSprite::GetFrameCount() const
{
    return static_cast<int>(m_frames.size());
}

hb::shared::sprite::SpriteRect SFMLSprite::GetFrameRect(int frame) const
{
    if (frame < 0 || frame >= static_cast<int>(m_frames.size()))
    {
        return hb::shared::sprite::SpriteRect{0, 0, 0, 0, 0, 0};
    }

    const PAKLib::sprite_rect& src = m_frames[frame];
    hb::shared::sprite::SpriteRect rect;
    rect.x = src.x;
    rect.y = src.y;
    rect.width = src.width;
    rect.height = src.height;
    rect.pivotX = src.pivotX;
    rect.pivotY = src.pivotY;
    return rect;
}

void SFMLSprite::GetBoundingRect(int x, int y, int frame, int& left, int& top, int& right, int& bottom)
{
    if (frame < 0 || frame >= static_cast<int>(m_frames.size()))
    {
        left = top = right = bottom = 0;
        return;
    }

    const PAKLib::sprite_rect& frameRect = m_frames[frame];

    left = x + frameRect.pivotX;
    top = y + frameRect.pivotY;
    right = left + frameRect.width;
    bottom = top + frameRect.height;
}

void SFMLSprite::CalculateBounds(int x, int y, int frame)
{
    if (frame < 0 || frame >= static_cast<int>(m_frames.size()))
    {
        m_boundRect = hb::shared::sprite::BoundRect{};
        return;
    }

    const PAKLib::sprite_rect& frameRect = m_frames[frame];

    m_boundRect.left = x + frameRect.pivotX;
    m_boundRect.top = y + frameRect.pivotY;
    m_boundRect.right = m_boundRect.left + frameRect.width;
    m_boundRect.bottom = m_boundRect.top + frameRect.height;
}

bool SFMLSprite::GetLastDrawBounds(int& left, int& top, int& right, int& bottom) const
{
    if (!m_boundRect.IsValid())
        return false;

    left = m_boundRect.left;
    top = m_boundRect.top;
    right = m_boundRect.right;
    bottom = m_boundRect.bottom;
    return true;
}

hb::shared::sprite::BoundRect SFMLSprite::GetBoundRect() const
{
    return m_boundRect;
}

bool SFMLSprite::CheckCollision(int spriteX, int spriteY, int frame, int pointX, int pointY)
{
    if (frame < 0 || frame >= static_cast<int>(m_frames.size()))
        return false;

    const PAKLib::sprite_rect& frameRect = m_frames[frame];

    int left = spriteX + frameRect.pivotX;
    int top = spriteY + frameRect.pivotY;
    int right = left + frameRect.width;
    int bottom = top + frameRect.height;

    // Bounding box check
    if (pointX < left || pointX >= right || pointY < top || pointY >= bottom)
        return false;

    // Pixel-perfect check using cached image alpha channel
    if (m_collisionImage.getSize().x == 0)
        return true; // Fallback to bounding box if image not available

    int localX = pointX - left;
    int localY = pointY - top;
    sf::Color pixel = m_collisionImage.getPixel({
        static_cast<unsigned int>(frameRect.x + localX),
        static_cast<unsigned int>(frameRect.y + localY)
    });
    return pixel.a > 0;
}

void SFMLSprite::Preload()
{
    if (!m_textureLoaded && !m_imageData.empty())
    {
        CreateTexture();
    }
}

void SFMLSprite::Unload()
{
    // SFML textures don't need explicit unloading
    // But we can clear the image cache to save memory
    // Note: Keep m_imageData for potential texture recreation
}

bool SFMLSprite::IsLoaded() const
{
    return m_textureLoaded;
}

void SFMLSprite::Restore()
{
    // SFML textures don't need restoration like DirectDraw surfaces
    // If needed, recreate from image data
    if (!m_textureLoaded && !m_imageData.empty())
    {
        CreateTexture();
    }
}

bool SFMLSprite::IsInUse() const
{
    return m_inUse;
}

uint32_t SFMLSprite::GetLastAccessTime() const
{
    return m_lastAccessTime;
}

void SFMLSprite::SetAmbientLightLevel(char level)
{
    if (m_ambient_light_level != level && m_alphaEffect)
    {
        m_ambient_light_level = level;
        ApplyAmbientLightLevel();
    }
}

void SFMLSprite::ApplyAmbientLightLevel()
{
    // Alpha degree is now applied at draw time via sprite color
    // No need to modify the texture
}
