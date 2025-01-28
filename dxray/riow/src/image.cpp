#include "riow/image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stbImageLoad.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stbImageWrite.h>

namespace dxray::riow
{
    static u8 CyanPixelColor[3] = { 0, 255, 255 };

    static_assert(sizeof(u8) == sizeof(stbi_uc));
    static_assert(sizeof(u16) == sizeof(stbi_us));

    inline String GetFileExtension(const Image::EFileExtension a_fileExtension)
    {
        switch (a_fileExtension)
        {
        case Image::EFileExtension::png:
            return String(".png");
        case Image::EFileExtension::jpg:
            return String(".jpg");
        case Image::EFileExtension::Invalid:
        default:
            DXRAY_ASSERT_WITH_MSG(true, "Invalid image file format chosen");
        }

        return String("");
    }

    Image::Image(const Path& a_path, const ELoadOptions a_loadFlags, u8 a_numChannels /*= 3*/) :
        m_dimensions(0, 0),
        m_channelCount(0),
        m_data(nullptr)
    {
        if (a_loadFlags & ELoadOptions::FlipVertically)
        {
            stbi_set_flip_vertically_on_load(true);
        }

        m_data = stbi_load(a_path.string().c_str(), &m_dimensions.x, &m_dimensions.y, &m_channelCount, a_numChannels);
        DXRAY_ASSERT(m_data != nullptr);
        if (m_channelCount != a_numChannels)
        {
            DXRAY_WARN("Converted {}'s channels from {} to {}.", a_path.string().c_str(), a_numChannels, m_channelCount);
        }

        DXRAY_INFO("Successfully loaded: {}", a_path.string().c_str());
    }

    Image::~Image()
    {
        DXRAY_ASSERT(m_data != nullptr);
        stbi_image_free(m_data);
        m_dimensions = vath::Vector2i32(0, 0);
        m_data = nullptr;
        m_channelCount = 0;
    }

    std::shared_ptr<Image> Image::LoadFromFile(const Path& a_path, const ELoadOptions a_loadFlags, u8 a_numChannels /*= 3*/)
    {
        return std::make_shared<Image>(a_path, a_loadFlags, a_numChannels);
    }

    const u8* Image::ReadPixel(const vath::Vector2i32& a_pixelCoords) const
    {
        if (m_data == nullptr)
        {
            return CyanPixelColor;
        }

        const vath::Vector2i32 clampedCoords(vath::Clamp<i32>(a_pixelCoords.x, 0, m_dimensions.x), vath::Clamp<i32>(a_pixelCoords.y, 0, m_dimensions.y));
        const i32 pixelIndex = vath::Clamp<i32>(clampedCoords.x * m_channelCount + m_dimensions.x * m_channelCount * clampedCoords.y, 0, m_dimensions.x * m_dimensions.y * m_channelCount - 1);
        return &m_data[pixelIndex];
    }

    i32 SaveColorBufferToFile(const String& a_fileName, const Image::EFileExtension a_fileExtension, i32 a_width, i32 a_height, i32 a_numChannels, Color* a_pColorData, bool a_bNormalizedData)
    {
        const i32 pixelCount = a_width * a_height;
        u8* const pixels = new u8[pixelCount * a_numChannels];

        for (i32 pixelIndex = 0; pixelIndex < pixelCount * a_numChannels; pixelIndex)
        {
            for (i32 channelIndex = 0; channelIndex < a_numChannels; channelIndex++)
            {
                if (a_bNormalizedData)
                {
                    fp32 channelValue = vath::Clamp<fp32>(a_pColorData[pixelIndex / a_numChannels][channelIndex], 0.0f, 1.0f);
                    pixels[pixelIndex + channelIndex] = static_cast<u8>(channelValue * 255.0f);
                }
                else
                {
                    fp32 channelValue = vath::Clamp<fp32>(a_pColorData[pixelIndex / a_numChannels][channelIndex], 0.0f, 255.0f);
                    pixels[pixelIndex + channelIndex] = static_cast<u8>(channelValue);
                }
            }

            pixelIndex += a_numChannels;
        }

        const i32 result = stbi_write_png(String(a_fileName + GetFileExtension(a_fileExtension)).c_str(), a_width, a_height, a_numChannels, pixels, a_width * a_numChannels);
        DXRAY_ASSERT(result > 0); //0 - failure.
        if (pixels != nullptr)
        {
            delete[] pixels;
        }

        return result;
    }
}