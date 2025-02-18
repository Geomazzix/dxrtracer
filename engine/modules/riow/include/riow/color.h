#pragma once
#include <core/vath/vath.h>

namespace dxray::riow
{
    using Color = vath::Vector3f;
    constexpr fp32 GammaFactor = 2.2f;

    /// <summary>
    /// Rough transform function to transform linear color space to srgb.
    /// </summary>
    /// <param name="a_color"></param>
    /// <returns></returns>
    inline Color LinearToSrgb(const Color& a_color)
    {
        const fp32 gammaReciprocal = 1.0f / GammaFactor;
        return Color(
            std::powf(a_color.x, gammaReciprocal),
            std::powf(a_color.y, gammaReciprocal),
            std::powf(a_color.z, gammaReciprocal)
        );
    }

    /// <summary>
    /// Rough transform function to transform color textures that are stored in srgb space to linear space.
    /// </summary>
    /// <param name="a_color"></param>
    /// <returns></returns>
    inline Color SrgbToLinear(const Color& a_color)
    {
        return Color(
            std::powf(a_color.x, GammaFactor),
            std::powf(a_color.y, GammaFactor),
            std::powf(a_color.z, GammaFactor)
        );
    }
}