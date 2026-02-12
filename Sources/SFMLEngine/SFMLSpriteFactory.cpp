// SFMLSpriteFactory.cpp: SFML implementation of ISpriteFactory interface
//
// Part of SFMLEngine static library
//////////////////////////////////////////////////////////////////////

#include "SFMLSpriteFactory.h"
#include "SFMLSprite.h"
#include "SFMLRenderer.h"
#ifndef _WIN32
#include <filesystem>
#include <algorithm>
#endif

SFMLSpriteFactory::SFMLSpriteFactory(SFMLRenderer* pRenderer)
    : m_pRenderer(pRenderer)
    , m_spritePath("sprites")
    , m_ambient_light_level(1)
{
}

SFMLSpriteFactory::~SFMLSpriteFactory()
{
    // Note: Factory does not own created sprites
    // Caller is responsible for destroying sprites via DestroySprite
}

hb::shared::sprite::ISprite* SFMLSpriteFactory::CreateSprite(const std::string& pakName, int spriteIndex, bool alphaEffect)
{
    std::string fullPath = BuildPakPath(pakName);

    SFMLSprite* sprite = new SFMLSprite(m_pRenderer, fullPath, spriteIndex, alphaEffect);

    // Apply global alpha degree
    if (alphaEffect && m_ambient_light_level != 1)
    {
        sprite->SetAmbientLightLevel(static_cast<char>(m_ambient_light_level));
    }

    return sprite;
}

hb::shared::sprite::ISprite* SFMLSpriteFactory::CreateSpriteFromData(const PAKLib::sprite& spriteData, bool alphaEffect)
{
    SFMLSprite* sprite = new SFMLSprite(m_pRenderer, spriteData, alphaEffect);

    // Apply global alpha degree
    if (alphaEffect && m_ambient_light_level != 1)
    {
        sprite->SetAmbientLightLevel(static_cast<char>(m_ambient_light_level));
    }

    return sprite;
}

void SFMLSpriteFactory::DestroySprite(hb::shared::sprite::ISprite* sprite)
{
    delete sprite;
}

void SFMLSpriteFactory::SetAmbientLightLevel(int level)
{
    m_ambient_light_level = level;

    // Update renderer's sprite alpha degree
    if (m_pRenderer)
    {
        m_pRenderer->SetAmbientLightLevel(static_cast<char>(level));
    }
}

int SFMLSpriteFactory::GetAmbientLightLevel() const
{
    return m_ambient_light_level;
}

int SFMLSpriteFactory::GetSpriteCount(const std::string& pakName) const
{
    std::string fullPath = BuildPakPath(pakName);

    try
    {
        PAKLib::pak pakFile = PAKLib::loadpak_fast(fullPath);
        return static_cast<int>(pakFile.sprite_count);
    }
    catch (...)
    {
        return 0;
    }
}

std::string SFMLSpriteFactory::BuildPakPath(const std::string& pakName) const
{
    // If pakName already has a path or extension, use as-is
    if (pakName.find('/') != std::string::npos ||
        pakName.find('\\') != std::string::npos ||
        pakName.find('.') != std::string::npos)
    {
        return pakName;
    }

    // Build path: spritePath/pakName.pak
    std::string path = m_spritePath;
    if (!path.empty() && path.back() != '/' && path.back() != '\\')
    {
        path += '/';
    }
    path += pakName;
    path += ".pak";

#ifndef _WIN32
    // Linux is case-sensitive; resolve actual filename from directory
    namespace fs = std::filesystem;
    fs::path p(path);
    fs::path dir = p.parent_path();
    std::string target = p.filename().string();
    std::string targetLower = target;
    std::transform(targetLower.begin(), targetLower.end(), targetLower.begin(), ::tolower);

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        std::string name = entry.path().filename().string();
        std::string nameLower = name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
        if (nameLower == targetLower) {
            return (dir / name).string();
        }
    }
#endif

    return path;
}
