#pragma once
#include "core/valueTypes.h"
#include "riow/color.h"

namespace dxray::riow
{
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
}