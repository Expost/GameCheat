#pragma once
#include <windows.h>
#include <stdio.h>

namespace Utils {

_inline void dbg_out(const char *str, ...)
{
    char szMsg[1024] = { 0 };

    va_list ap;
    va_start(ap, str);
    _vsnprintf_s(szMsg, 1024 - 1, str, ap);
    va_end(ap);

    OutputDebugStringA(szMsg);
}

_inline float get_distance_3d(FVector x, FVector y)
{
    FVector z;
    z.X = x.X - y.X;
    z.Y = x.Y - y.Y;
    z.Z = x.Z - y.Z;

    return sqrt(z.X * z.X + z.Y * z.Y + z.Z * z.Z);
}

_inline float get_distance_2d(FVector2D point1, FVector2D point2)
{
    FVector2D heading;
    heading.X = point1.X - point2.X;
    heading.Y = point1.Y - point2.Y;

    float distanceSquared;
    float distance;

    distanceSquared = heading.X * heading.X + heading.Y * heading.Y;
    distance = sqrt(distanceSquared);

    return distance;
}
}