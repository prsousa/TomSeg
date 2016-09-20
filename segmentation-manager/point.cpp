#include <cmath>

#include "point.h"

float Point::distance(Point b)
{
    float distanceX = (b.x - this->x) * (b.x - this->x);
    float distanceY = (b.y - this->y) * (b.y - this->y);

    return sqrt( distanceX + distanceY );
}
