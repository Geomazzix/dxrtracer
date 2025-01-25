#pragma once
#include "riow/traceable/raytraceable.h"
#include "riow/color.h"

namespace dxray::riow
{
    //--- Material helper functions ---

    inline bool IsVectorNearZero(const vath::Vector3f& a_vector)
    {
        const fp32 eps = vath::Epsilon<fp32>();
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

    inline vath::Vector3f Refract(const vath::Vector3f& a_unitVector, const vath::Vector3f& a_normal, const fp32 a_eta)
    {
        const fp32 cosTheta = vath::Min<fp32>(vath::Dot(-a_unitVector, a_normal), 1.0f);
        vath::Vector3f perpendicular = a_eta * (a_unitVector + cosTheta * a_normal);
        vath::Vector3f parallel = -std::sqrt(vath::Abs<fp32>(1.0f - SqrMagnitude(perpendicular))) * a_normal;
        return perpendicular + parallel;
    }

    inline fp32 SchlickApprox(const fp32 a_cosTheta, fp32 a_refractionIndex)
    {
        fp32 r0 = (1.0f - a_refractionIndex) / (1.0f + a_refractionIndex);
        r0 *= r0;
        return r0 + (1.0f - r0) * std::powf(1.0f - a_cosTheta, 5);
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
            vath::Vector3f scatterDirection = a_hitInfo.Normal + RandomUnitVector();
			if (IsVectorNearZero(scatterDirection))
            {
                scatterDirection = a_hitInfo.Normal;
            }

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
    class Metallic final : public Material
    {
    public:
        Metallic(const Color& a_albedoColor, const fp32 a_glossynessFactor) :
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


    /// <summary>
    /// Material that representing refractive mediums.
    /// </summary>
    class Dialectric final : public Material
    {
    public:
        Dialectric(const fp32 a_refractiveIndex) :
            m_refractiveIndex(a_refractiveIndex)
        {
        }

        bool Scatter(const Ray& a_ray, const IntersectionInfo& a_hitInfo, Color& a_attenuation, Ray& a_scatteredRay) const override
        {
            a_attenuation = Color(1.0f); //Color of this material is currently white.
            const fp32 ri = a_hitInfo.FrontFace ? (1.0f / m_refractiveIndex) : m_refractiveIndex;

            const vath::Vector3f unitDirection = Normalize(a_ray.GetDirection());
            const fp32 cosTheta = vath::Min<fp32>(vath::Dot(-unitDirection, a_hitInfo.Normal), 1.0f);
            const fp32 sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

            //If the ray bounces back into the hemisphere it came from there is case for internal reflection.
            const vath::Vector3f scatterDirection = (ri * sinTheta > 1.0f) || SchlickApprox(cosTheta, ri) > vath::RandomNumber<fp32>()
                ? Reflect(unitDirection, a_hitInfo.Normal)
                : Refract(unitDirection, a_hitInfo.Normal, ri);

            a_scatteredRay = Ray(a_hitInfo.Point, scatterDirection);
            return true;
        }

    private:
        fp32 m_refractiveIndex;
    };
}