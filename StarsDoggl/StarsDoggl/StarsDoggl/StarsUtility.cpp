#include "StarsUtility.h"

float GetDistance(float fX1, float fY1, float fX2, float fY2)
{
	return sqrtf((fX1 - fX2) * (fX1 - fX2) + (fY1 - fY2) * (fY1 - fY2));
}