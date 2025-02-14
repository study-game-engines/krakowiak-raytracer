#include "uchar4.h"

#include "float4.h"
#include "uint4.h"

using namespace Engine1;

const uchar4 uchar4::ZERO( 0, 0, 0, 0 );

uchar4 Engine1::operator * (const unsigned char value, const uchar4& vec)
{
    return uchar4( vec.x * value, vec.y * value, vec.z * value, vec.w * value );
}

uchar4::operator float4() const
{
    return float4(
        (float)x,
        (float)y,
        (float)z,
        (float)w
    );
}

uchar4::operator uint4() const
{
    return uint4(
        (unsigned int)x,
        (unsigned int)y,
        (unsigned int)z,
        (unsigned int)w
    );
}

uchar4 max( const uchar4& a, const uchar4& b )
{
    return uchar4(
        a.x > b.x ? a.x : b.x,
        a.y > b.y ? a.y : b.y,
        a.z > b.z ? a.z : b.z,
        a.w > b.w ? a.w : b.w
        );
}

uchar4 min( const uchar4& a, const uchar4& b )
{
    return uchar4(
        a.x < b.x ? a.x : b.x,
        a.y < b.y ? a.y : b.y,
        a.z < b.z ? a.z : b.z,
        a.w < b.w ? a.w : b.w
        );
}
