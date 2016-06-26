#ifndef POINT_H
#define POINT_H

class Point
{
public:
    int x, y, z;

    Point() { }

    Point(int x, int y) {
        this->x = x;
        this->y = y;
    }

    Point(int x, int y, int z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

#endif // POINT_H
