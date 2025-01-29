#pragma once
#include "core/valueTypes.h"
#include "riow/color.h"
#include "riow/image.h"
#include "riow/perlin.h"

namespace dxray::riow
{
    inline const Color InvalidTexture = Color(1.0f, 0.0f, 1.0f);

    /// <summary>
    /// Base class for any image that can be sampled 
    /// </summary>
    class Texture
    {
    public:
        virtual ~Texture() = default;
        virtual Color Sample(const vath::Vector2f& a_uvCoord, const vath::Vector3f& a_point) const = 0;
    };


    /// <summary>
    /// Represents a solid color that can be used for objects. This way we can assign a color or texture based on image data regardless of static type.
    /// </summary>
    class SolidColor final : public Texture
    {
    public:
        SolidColor(const Color& a_albedo) :
            m_albedo(a_albedo)
        {}

        Color Sample(const vath::Vector2f& a_uvCoord, const vath::Vector3f& a_point) const override
        {
            return m_albedo;
        }

    private:
        Color m_albedo;
    };


    /// <summary>
    /// Classic checkerboard implementation, based on even/odd floored values.
    /// </summary>
    class CheckerBoard final : public Texture
    {
    public:
        CheckerBoard(const fp32 a_scale, std::shared_ptr<Texture> a_evenTile, std::shared_ptr<Texture> a_oddTile) :
            m_scaleReciprocal(1.0f / a_scale),
            m_evenTileTexture(a_evenTile), 
            m_oddTileTexture(a_oddTile)
        { }

        CheckerBoard(const fp32 a_scale, const Color& a_evenColor, const Color& a_oddColor) :
            m_scaleReciprocal(1.0f / a_scale),
            m_evenTileTexture(std::make_shared<SolidColor>(a_evenColor)),
            m_oddTileTexture(std::make_shared<SolidColor>(a_oddColor))
        { }

        Color Sample(const vath::Vector2f& a_uvCoord, const vath::Vector3f& a_point) const override
        {
            const vath::Vector3u32 tileEdges(
                static_cast<u32>(std::floor(m_scaleReciprocal * a_point.x)),
                static_cast<u32>(std::floor(m_scaleReciprocal * a_point.y)),
                static_cast<u32>(std::floor(m_scaleReciprocal * a_point.z))
            );

            return (tileEdges.x + tileEdges.y + tileEdges.z) % 2 == 0
                ? m_evenTileTexture->Sample(a_uvCoord, a_point)
                : m_oddTileTexture->Sample(a_uvCoord, a_point);
        }

    private:
        fp32 m_scaleReciprocal;
        std::shared_ptr<Texture> m_evenTileTexture;
        std::shared_ptr<Texture> m_oddTileTexture;
    };


    /// <summary>
    /// Texture based on image data.
    /// </summary>
    class ImageTexture final : public Texture
    {
    public:
        ImageTexture(std::shared_ptr<Image> a_image) :
            m_image(a_image)
        { }

        Color Sample(const vath::Vector2f& a_uvCoord, const vath::Vector3f& a_point) const override
        {
            if (m_image == nullptr)
            {
                return InvalidTexture;
            }

            const u8* pixel = m_image->ReadPixel(vath::Vector2i32(
                static_cast<i32>(vath::Clamp<fp32>(a_uvCoord.x, 0.0f, 1.0f) * m_image->GetWidth()),
                static_cast<i32>(vath::Clamp<fp32>(a_uvCoord.y, 0.0f, 1.0f) * m_image->GetHeight())
            ));

            Color sampledColor = Color(0.0f);
            const fp32 unitColorReciprocal = 1.0f / 255.0f;
            for (u8 ci = 0; ci < m_image->GetChannelCount(); ++ci)
            {
                sampledColor[ci] = static_cast<fp32>(pixel[ci] * unitColorReciprocal);
            }

            return sampledColor;
        }

    private:
        std::shared_ptr<Image> m_image;
    };


    /// <summary>
    /// Texture able to sample perlin noise.
    /// </summary>
    class NoiseTexture final : public Texture
    {
    public:
        NoiseTexture(const fp32 a_noiseScalar, const u32 a_accumilationDepth = 7) :
            m_perlin(),
            m_noiseScalar(a_noiseScalar),
            m_accumilationDepth(a_accumilationDepth)
        {}

        ~NoiseTexture() = default;

        Color Sample(const vath::Vector2f& a_uvCoord, const vath::Vector3f& a_point) const override
        {
            //As perlin noise returns values between -1 and 1 due to directions being fully random they are mapped to 0 and 1.
            const fp32 noiseValue = m_perlin.Turbulence(a_point, m_accumilationDepth);

            //Marble like noise.
            //const fp32 noiseValue = 0.5f * (1.0f + std::sin(m_noiseScalar * a_point.x + 10.0f * m_perlin.Turbulence(a_point, m_accumilationDepth)));
            return vath::Vector3f(noiseValue);
        }

    private:
        Perlin m_perlin;
        fp32 m_noiseScalar;
        u32 m_accumilationDepth;
    };
}