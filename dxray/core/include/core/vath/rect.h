#pragma once
#include "core/vath/vathUtility.h"
#include "core/vath/vector2.h"

//#Todo: Add comparison operators for rect.

#pragma warning(push)
#pragma warning(disable:4201)

namespace dxray::vath
{
    template<typename T> requires std::arithmetic<T>
    class Rect final
    {
    public:
        using ValueType = T;

        Rect();
        Rect(const T a_x, const T a_y, const T a_width, const T a_height);
        ~Rect() = default;

        union
        {
            T Data[4];
            struct { T x, y, Width, Height; };
            struct { Vector<2, T> Offset; Vector<2, T> Dimensions; };
        };
    };

    template<typename T> requires std::arithmetic<T>
    Rect<T>::Rect() :
        x(0),
        y(0),
        Width(0),
        Height(0)
    {}

    template<typename T> requires std::arithmetic<T>
    Rect<T>::Rect(const T a_x, const T a_y, const T a_width, const T a_height) :
        x(a_x),
        y(a_y),
        Width(a_width),
        Height(a_height)
    {}
}

#pragma warning(pop)