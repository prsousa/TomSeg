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

const char* SeedInfo::colors[] = {
    "#FF0000", "#00FF00", "#0000FF", "#FFFF00", "#FF00FF", "#00FFFF",
    "#800000", "#008000", "#000080", "#808000", "#800080", "#008080", "#808080",
    "#C00000", "#00C000", "#0000C0", "#C0C000", "#C000C0", "#00C0C0", "#C0C0C0",
    "#400000", "#004000", "#000040", "#404000", "#400040", "#004040", "#404040",
    "#200000", "#002000", "#000020", "#202000", "#200020", "#002020", "#202020",
    "#600000", "#006000", "#000060", "#606000", "#600060", "#006060", "#606060",
    "#A00000", "#00A000", "#0000A0", "#A0A000", "#A000A0", "#00A0A0", "#A0A0A0",
    "#E00000", "#00E000", "#0000E0", "#E0E000", "#E000E0", "#00E0E0", "#E0E0E0"
};

QColor SeedInfo::getColor() {

//    QColor color;
//    int hue = ( this->id * (255 / EXPECTED_SEEDS) ) % 255;
//    color.setHsl( hue, 175, 175 );
    int colorIndex = this->id % 55;
    QColor color;
    color.setNamedColor( colors[colorIndex] );
    return color;
}
