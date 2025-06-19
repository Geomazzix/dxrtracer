#pragma once
#include "core/vath/vathUtility.h"
#include "core/vath/vector2.h"

//#Todo: Add comparison operators for rect.

#pragma warning(push)
#pragma warning(disable:4201)

namespace dxray::vath
{
    /**
     * @brief Utility class for rects being used by various APIs.
     * @tparam T 
     */
    template<Arithmetic T>
    class Rect final
    {
    public:
        using ValueType = T;

        constexpr Rect();
        constexpr Rect(const T a_x, const T a_y, const T a_width, const T a_height);
        ~Rect() = default;

        union
        {
            T Data[4];
            struct { T x, y, Width, Height; };
            struct { Vector<2, T> Offset; Vector<2, T> Dimensions; };
        };
    };


    //--- Rect definitions ---

	using Rectu8 = Rect<u8>;
	using Rectu16 = Rect<u16>;
	using Rectu32 = Rect<u32>;
	using Rectu64 = Rect<u64>;
	using Recti8 = Rect<i8>;
	using Recti16 = Rect<i16>;
	using Recti32 = Rect<i32>;
	using Recti64 = Rect<i64>;
	using Rectf = Rect<fp32>;
	using Rectd = Rect<fp64>;


    //--- Rect construction/destruction ---

    template<Arithmetic T>
    constexpr Rect<T>::Rect() :
        x(0),
        y(0),
        Width(0),
        Height(0)
    {}

    template<Arithmetic T>
    constexpr Rect<T>::Rect(const T a_x, const T a_y, const T a_width, const T a_height) :
        x(a_x),
        y(a_y),
        Width(a_width),
        Height(a_height)
    {}
}

#pragma warning(pop)