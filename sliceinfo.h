#ifndef SLICEINFO_H
#define SLICEINFO_H

#include <QVector>

#include "seedinfo.h"

class SliceInfo
{
public:
    SliceInfo();
    SliceInfo(QString filename);

    QString fileName;
    QVector<SeedInfo> seedInfos;
};

#endif // SLICEINFO_H
