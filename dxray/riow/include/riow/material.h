#pragma once
#include "riow/traceable/raytraceable.h"
#include "riow/color.h"

namespace dxray::riow
{
    //--- Material helper functions ---

    inline bool IsVectorNearZero(const vath::Vector3f& a_vector)
    {
        const fp32 eps = 1e-8f;
        return (std::fabsf(a_vector.x) < eps && std::fabsf(a_vector.y) < eps && std::fabsf(a_vector.z) < eps);
    }

    inline vath::Vector3f RandomUnitVector()
    {
        //Generate a random unit vector.
        while (true)
        {
            const vath::Vector3 randomDirection(
                vath::RandomNumber<fp32>(-1.0f, 1.0f),
                vath::RandomNumber<fp32>(-1.0f, 1.0f),
                vath::RandomNumber<fp32>(-1.0f, 1.0f)
            );

            const fp32 directionSqrMagnitude = vath::SqrMagnitude(randomDirection);
            if (1e-160 < directionSqrMagnitude && directionSqrMagnitude <= 1.0f)
            {
                return randomDirection / std::sqrt(directionSqrMagnitude);
            }
        }
    }

    inline vath::Vector3f RandomHemisphereReflection(const vath::Vector3f a_normal)
    {
        vath::Vector3 randomUnitVector = RandomUnitVector();

        //Invert the unit direction if it's pointing towards the correct hemisphere, based on the normal.
        return vath::Dot(randomUnitVector, a_normal) > 0.0f
            ? randomUnitVector
            : -randomUnitVector;
    }

    inline vath::Vector3f Reflect(const vath::Vector3f& a_vector, const vath::Vector3f a_normal)
    {
        return a_vector - 2.0f * vath::Dot(a_vector, a_normal) * a_normal;
    }


    //--- Material definitions ---

    /// <summary>
    /// Contains all info for the renderer to render the images.
    /// #Todo: Potentially find a way to transform this into a concept to get rid of the inheritance. The issue now becomes:
    /// how do we store/keep track of the materials - we could store them in 3 separate arrays, though there is no need to loop over them
    /// making the approach ineffective.
    /// </summary>
    class Material
    {
    public:
        virtual ~Material() = default;

        virtual bool Scatter(const Ray& ray, const IntersectionInfo& a_hitInfo, Color& a_attenuation, Ray& a_scatteredRay) const
        {
            return false;
        }
    };

    /// <summary>
    /// Material that shades using a lambertian diffuse reflection: meaning fully matte.
    /// </summary>
    class Lambertian final : public Material
    {
    public:
        Lambertian(const Color& a_albedoColor) :
            m_albedo(a_albedoColor)
        {}

        bool Scatter(const Ray& a_ray, const IntersectionInfo& a_hitInfo, Color& a_attenuation, Ray& a_scatteredRay) const override
        {
            const vath::Vector3f scatterDirection = IsVectorNearZero(scatterDirection)
                ? a_hitInfo.Normal 
                : a_hitInfo.Normal + RandomUnitVector();

            a_scatteredRay = Ray(a_hitInfo.Point, scatterDirection);
            a_attenuation = m_albedo;
            return true;
        }

    private:
        Color m_albedo;
    };


    /// <summary>
    /// Material that representing glossy material scattering.
    /// </summary>
    class Metalic final : public Material
    {
    public:
        Metalic(const Color& a_albedoColor, const fp32 a_glossynessFactor) :
            m_albedo(a_albedoColor),
            m_glossyness(vath::Min<fp32>(a_glossynessFactor, 1.0f))
        {}

        bool Scatter(const Ray& a_ray, const IntersectionInfo& a_hitInfo, Color& a_attenuation, Ray& a_scatteredRay) const override
        {
            vath::Vector3f reflected = Reflect(a_ray.GetDirection(), a_hitInfo.Normal);
            reflected = Normalize(reflected) + m_glossyness * RandomUnitVector();

            a_scatteredRay = Ray(a_hitInfo.Point, reflected);
            a_attenuation = m_albedo;
            
            return (vath::Dot(reflected, a_hitInfo.Normal) > 0.0f);
        }

    private:
        Color m_albedo;
        fp32 m_glossyness;
    };
}