#pragma once
#include <core/string.h>
#include <core/vath/vath.h>
#include "riow/color.h"

namespace dxray::riow
{
    inline Path AssetRootDirectory = Path(ASSET_ROOT_DIRECTORY);

    /// <summary>
    /// Holds byte formatted image data.
    /// #Todo: Implement HDR image data loading, i.e. an image that can hold fp32 for it's data.
    /// </summary>
    class Image
    {
    public:
        enum class EFileExtension : u8
        {
            png = 0,
            jpg,
            Invalid = 255
        };

        //#Note: Keep unscoped - I can't be asked to implement bitmask flags for scoped enums.
        enum ELoadOptions
        {
            FlipVertically = 0x1
        };

        Image(const Path& a_path, const ELoadOptions a_loadFlags, u8 a_numChannels = 3);
        ~Image();

        vath::Vector2i32 GetDimensions() const;
        u32 GetWidth() const;
        u32 GetHeight() const;
        u32 GetChannelCount() const;        
        const u8* ReadPixel(const vath::Vector2i32& a_pixelCoords) const;
        
        static std::shared_ptr<Image> LoadFromFile(const Path& a_path, const ELoadOptions a_loadFlags, u8 a_numChannels = 3);

    private:
        vath::Vector2i32 m_dimensions;
        i32 m_channelCount;
        u8* m_data;
    };

    inline vath::Vector2i32 Image::GetDimensions() const
    {
        return m_dimensions;
    }

    inline u32 Image::GetWidth() const
    {
        return m_dimensions.x;
    }

    inline u32 Image::GetHeight() const
    {
        return m_dimensions.y;
    }

    inline u32 Image::GetChannelCount() const
    {
        return m_channelCount;
    }

    //#Todo: Should be reimplemented to: SaveBufferToFile(takes in any buffer type + stride per data + datatype).
    extern i32 SaveColorBufferToFile(const String& a_fileName, const Image::EFileExtension a_fileExtension, i32 a_width, i32 a_height, i32 a_numChannels, Color* a_pColorData, bool a_bNormalizedData);
}