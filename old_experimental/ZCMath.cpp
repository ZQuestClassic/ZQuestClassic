
#include "ZCMath.h"


const Vector2i Vector2i::Zero = Vector2i(0, 0);
const Vector2i Vector2i::One = Vector2i(1, 1);

const Vector2f Vector2f::Zero = Vector2f(0.0f, 0.0f);
const Vector2f Vector2f::One = Vector2f(1.0f, 1.0f);

const Rect Rect::Zero = Rect(0, 0, 0, 0);
const Rect Rect::One = Rect(0, 0, 1, 1);


Vector2f Vector2f::Refract(const Vector2f& unit, float eta) const
{
	const float dot = Dot(unit);
	const float k = 1.0f - eta * eta * (1.0f - dot * dot);
	if(k < 0)
		return Vector2f::Zero;

	return Vector2f((*this * eta) - (unit * (eta * dot + sqrtf(k))));
}


float DistancetoLineSegment(const Vector2f& a, const Vector2f& b, const Vector2f& point)
{
	Vector2f dist = b - a;
	float length = dist.Length();
	float t = (point - a).Dot(dist);

	if(t < 0.0f)
		return (point - a).Length();
	if(t > length)
		return (point - b).Length();

	dist.Normalize();
	if(dist == Vector2f::Zero)
		return (point - a).Length();

	return (point - (a + dist * t)).Length();
}
