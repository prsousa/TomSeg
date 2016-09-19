#ifndef SLICEINFO_H
#define SLICEINFO_H

#include <QVector>
#include <QPixmap>

#include "seedinfo.h"

class SliceInfo
{
public:
    SliceInfo();
    SliceInfo(QString filename);

    QString fileName;
    QPixmap image;
    QPixmap segmentationResult;
    QVector<SeedInfo> seedInfos;
};

#endif // SLICEINFO_H
