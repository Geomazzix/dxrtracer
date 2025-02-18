#include "riow/perlin.h"
#include <core/vath/vathUtility.h>

namespace dxray
{
    Perlin::Perlin()
    {
        //The random noise is calculated here, the moment the noise is sampled is when it's taking the values from 
        //this existing buffer and using them to interpolate them.
        for (i32 i = 0; i < m_pointCount; i++)
        {
            m_noiseMap[i] = vath::Normalize(vath::Vector3f(vath::RandomNumber<fp32>(-1.0f, 1.0f)));
        }

        for (i32 i = 0; i < m_pointCount; i++)
        {
            m_permutationx[i] = i;
            m_permutationy[i] = i;
            m_permutationz[i] = i;
        }

        Permute(m_permutationx);
        Permute(m_permutationy);
        Permute(m_permutationz);
    }

    Perlin::~Perlin()
    {
        //#Note: stack memory is running low in riow.cpp, if stackoverflowing consider moving this to the heap.
    }

    void Perlin::Permute(i32* a_pPermutation)
    {
        for (i32 i = m_pointCount - 1; i > 0; i--)
        {
            const i32 target = vath::RandomNumber<i32>(0, i);
            std::swap(a_pPermutation[i], a_pPermutation[target]);
        }
    }

    fp32 Perlin::Noise(const vath::Vector3f& a_point) const
    {
        //Lattice points, interpolation will be based on this.
        const vath::Vector3i32 latticePoint(
            static_cast<i32>(vath::Floor<fp32>(a_point.x)),
            static_cast<i32>(vath::Floor<fp32>(a_point.y)),
            static_cast<i32>(vath::Floor<fp32>(a_point.z))
        );

        //2 * 2 * 2 = 8 vectors for all of the corners of the 3d grid, to be stored in this 3d array.
        vath::Vector3f c[2][2][2];
        for (i32 x = 0; x < 2; x++)
        {
            for (i32 y = 0; y < 2; y++)
            {
                for (i32 z = 0; z < 2; z++)
                {
                    c[x][y][z] = m_noiseMap[
                        m_permutationx[(latticePoint.x + x) & 255] ^ 
                        m_permutationy[(latticePoint.y + y) & 255] ^ 
                        m_permutationz[(latticePoint.z + z) & 255]
                    ];
                }
            }
        }

        //Lattice point delta to the next point.
        const vath::Vector3f latticePointDelta(
            a_point.x - vath::Floor<fp32>(a_point.x),
            a_point.y - vath::Floor<fp32>(a_point.y),
            a_point.z - vath::Floor<fp32>(a_point.z)
        );

        return TrilinearInterpolate(c, latticePointDelta);
    }

    fp32 Perlin::Turbulence(const vath::Vector3f& a_point, u32 a_depth) const
    {
        fp32 accumulated = 0.0f;
        vath::Vector3f frequency = a_point;
        fp32 weight = 1.0f;

        //The concept behind turbulence is to take the sum of multiple summed frequencies.
        //Hence multiple noise samples are taken and averaged.
        for (u32 i = 0; i < a_depth; i++)
        {
            accumulated += weight * Noise(frequency);
            weight *= 0.5f;
            frequency *= 2.0f;
        }

        //Take the abs of the value as noise samples are taken from -1 and 1 pointing directions.
        return vath::Abs<fp32>(accumulated);
    }

    fp32 Perlin::TrilinearInterpolate(const vath::Vector3f c[2][2][2], const vath::Vector3f& a_uvw)
    {
        //Hermitian smoothing to get rid of Mach-bands - an common artifact from the result of color lerping.
        vath::Vector3 smoothedUvw(
            a_uvw.x * a_uvw.x * (3.0f - 2.0f * a_uvw.x),
            a_uvw.y * a_uvw.y * (3.0f - 2.0f * a_uvw.y),
            a_uvw.z * a_uvw.z * (3.0f - 2.0f * a_uvw.z)
        );

        //Tri-linear interpolate the values.
        fp32 accumulated = 0.0f;
        for (i32 i = 0; i < 2; i++)
        {
            for (i32 j = 0; j < 2; j++)
            {
                for (i32 k = 0; k < 2; k++)
                {
                    const vath::Vector3f weight(a_uvw.u - i, a_uvw.v - j, a_uvw.w - k);
                    accumulated += (
                        (i * smoothedUvw.u + (1.0f - i) * (1.0f - smoothedUvw.u)) *
                        (j * smoothedUvw.v + (1.0f - j) * (1.0f - smoothedUvw.v)) *
                        (k * smoothedUvw.w + (1.0f - k) * (1.0f - smoothedUvw.w)) *
                        vath::Dot(c[i][j][k], weight));
                }
            }
        }

        return accumulated;
    }
}