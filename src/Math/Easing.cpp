#include "Easing.h"

Vector2 Lerp(Vector2 start, Vector2 end, float t)
{
	return start + (end - start) * t;
}

Vector2 EaseIn(Vector2 start, Vector2 end, float t)
{
	return start + (end - start) * t * t * t;
}

Vector2 EaseOut(Vector2 start, Vector2 end, float t)
{
	return start + (end - start) * (1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t));
}