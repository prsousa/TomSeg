#include "seedinfo.h"

#define EXPECTED_SEEDS 10

SeedInfo::SeedInfo()
{
    this->color = QColor();
    this->x = 0;
    this->y = 0;
    this->width = 0;
    this->height = 0;
    this->active = true;
}

SeedInfo::SeedInfo(int id, int x, int y, int width, int height)
{
    this->id = id;
    this->color = this->getColor();
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->active = true;
}

QColor SeedInfo::getColor() {
    QColor color;
    int hue = ( this->id * (255 / EXPECTED_SEEDS) ) % 255;
    color.setHsl( hue, 175, 175 );

    return color;
}
