#pragma once

static const float Pi = 3.14159265f; // #Todo: move this into a separate shader file (perhaps a bridge to vath?)

inline float DegToRad(in float a_deg)
{
    return a_deg * (Pi / 180);
};

inline float RadToDeg(in float a_rad)
{
    return a_rad * (180 / Pi);
};