#pragma once
#include <array>
#include <core/valueTypes.h>

namespace dxray
{
    /// <summary>
    /// The perlin noice class, capable of generating and retrieving noise from specific locations.
    /// </summary>
    class Perlin final
    {
    public:
        Perlin();
        ~Perlin();

        fp32 Noise(const vath::Vector3f& a_point) const;
        fp32 Turbulence(const vath::Vector3f& a_point, u32 a_depth) const;

    private:
        static const i32 m_pointCount = 256;
        std::array<vath::Vector3f, m_pointCount> m_noiseMap;
        i32 m_permutationx[m_pointCount];
        i32 m_permutationy[m_pointCount];
        i32 m_permutationz[m_pointCount];

        static void Permute(i32* a_pPermutation);
        static fp32 TrilinearInterpolate(const vath::Vector3f c[2][2][2], const vath::Vector3f& a_uvw);
    };
}