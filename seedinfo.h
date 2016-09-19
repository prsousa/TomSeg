#ifndef SEEDINFO_H
#define SEEDINFO_H

#include <QColor>

class SeedInfo
{
public:
    SeedInfo();
    SeedInfo(int id, int x, int y, int width, int height);

    QColor color;
    int id;
    int x, y, width, height;
    bool active;

private:
    QColor getColor();
    static const char* colors[];
};

#endif // SEEDINFO_H
