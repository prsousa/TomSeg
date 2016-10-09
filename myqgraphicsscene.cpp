#include "myqgraphicsscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItemGroup>
#include <QPainter>
#include <QTime>
#include <QDebug>

MyQGraphicsScene::MyQGraphicsScene(QObject *parent) :
    QGraphicsScene(parent)
{
    slice = NULL;

    firstClick = true;
    gridVisible = false;

    slicePixmapItem = new QGraphicsPixmapItem();
    slicePixmapItem->setZValue(1);
    this->addItem(slicePixmapItem);

    seedsZone = createItemGroup(seedsDraw);
    seedsZone->setZValue(2);

    resultZone = createItemGroup(resultDraw);
    resultZone->setZValue(3);

    cropZone = createItemGroup(cropDraw);
    cropZone->setZValue(4);

    drawZone = createItemGroup(itemDraw);
    drawZone->setZValue(5);

    QGraphicsTextItem* placeHolder = new QGraphicsTextItem();
    placeHolder->setHtml("<h3 style='color: #999'><i>Slice Viewer</i></h3>");
    this->addItem(placeHolder);
}

void MyQGraphicsScene::setSlice(Slice *slice)
{
    this->slice = slice;

    this->updateSliceDisplayer();
    this->updateSeedsDisplayer();
    this->updateResultDisplayer();
}

QPixmap convertSliceImage(cv::Mat& image) {
    static QVector<QRgb> sColorTable_Gray(256);
    static bool tableColorsAlreadyLoaded_Gray = false;

    if( !tableColorsAlreadyLoaded_Gray ) {
        tableColorsAlreadyLoaded_Gray = true;
        for ( int i = 0; i < 256; i++ ) {
            sColorTable_Gray[i] = qRgb( i, i, i );
        }
    }

    QImage result( image.data,
                  image.cols, image.rows,
                  static_cast<int>(image.step),
                  QImage::Format_Indexed8 );

    result.setColorTable( sColorTable_Gray );

    return QPixmap::fromImage(result);
}

void MyQGraphicsScene::updateSliceDisplayer()
{
    if( slice ) {
        this->slicePixmapItem->setPixmap( convertSliceImage( slice->getImg() ) );
    }
}

void MyQGraphicsScene::updateCropDisplayer(size_t left, size_t right, size_t top, size_t bottom)
{
    QList<QGraphicsItem *>::iterator i;
    for( i = cropDraw.begin(); i != cropDraw.end(); i++) {
        QGraphicsItem *oldArea = *i;
        cropZone->removeFromGroup(oldArea);
        cropDraw.removeOne(oldArea);
        delete oldArea;
    }

    if( slice ) {
        cv::Mat& sliceImg = slice->getImg();

        QBrush brush( QColor(0, 0, 150, 100) );
        QPen pen( QColor(0, 0, 0, 0) );
        pen.setWidth(0);


        QGraphicsRectItem* leftRect = new QGraphicsRectItem(0, 0, left, sliceImg.rows);
        QGraphicsRectItem* rightRect = new QGraphicsRectItem(sliceImg.cols - right, 0, right, sliceImg.rows);
        QGraphicsRectItem* topRect = new QGraphicsRectItem(left, 0, std::max(0, sliceImg.cols - (int) (right + left)), top);
        QGraphicsRectItem* bottomRect = new QGraphicsRectItem(left, sliceImg.rows - bottom, std::max(0, sliceImg.cols - (int) (right + left)), bottom);

        leftRect->setBrush(brush);
        rightRect->setBrush(brush);
        topRect->setBrush(brush);
        bottomRect->setBrush(brush);

        leftRect->setPen(pen);
        rightRect->setPen(pen);
        topRect->setPen(pen);
        bottomRect->setPen(pen);

        cropDraw.append(leftRect);
        cropDraw.append(rightRect);
        cropDraw.append(topRect);
        cropDraw.append(bottomRect);

        cropZone->addToGroup(leftRect);
        cropZone->addToGroup(rightRect);
        cropZone->addToGroup(topRect);
        cropZone->addToGroup(bottomRect);
    }
}

void MyQGraphicsScene::updateSeedsDisplayer()
{
    QList<QGraphicsItem *>::iterator i;
    for( i = seedsDraw.begin(); i != seedsDraw.end(); i++) {
        QGraphicsItem *oldSeed = *i;
        seedsZone->removeFromGroup(oldSeed);
        seedsDraw.removeOne(oldSeed);
        delete oldSeed;
    }

    if( slice ) {
        std::vector<Seed>& seeds = slice->getSeeds();
        uchar color[3];

        for( int i = 0; i < seeds.size(); i++ ) {
            Seed seed = seeds[i];
            if( seed.active ) {
                Seed::getColor(seed.getId(), color);
                QColor seedColor(color[0], color[1], color[2]);
                this->addSeed(seed.a.x, seed.a.y, seed.b.x - seed.a.x, seed.b.y - seed.a.y, seedColor);
            }
        }
    }
}

QGraphicsRectItem *MyQGraphicsScene::addSeed(qreal x, qreal y, qreal w, qreal h, const QColor& seedColor)
{
    QPen pen( seedColor, 3 );
    pen.setCosmetic(true);

    QGraphicsRectItem* r = new QGraphicsRectItem(x, y, w, h);
    r->setPen(pen);
    seedsDraw.append(r);
    seedsZone->addToGroup(r);

    return r;
}

QPixmap convertSegmentationResult(cv::Mat& labels) {
    static QVector<QRgb> sColorTable_Labels(256);
    static bool colorsAlreadyLoaded_Labels = false;

    if( !colorsAlreadyLoaded_Labels ) {
        colorsAlreadyLoaded_Labels = true;
        uchar color[3];
        for ( int i = 0; i < 256; i++ ) {
            Seed::getColor(i, color);
            sColorTable_Labels[i] = qRgb(color[0], color[1], color[2]);
        }
    }

    QImage result( labels.data,
                  labels.cols, labels.rows,
                  static_cast<int>(labels.step),
                  QImage::Format_Indexed8 );

    result.setColorTable(sColorTable_Labels);

    return QPixmap::fromImage(result);
}

void MyQGraphicsScene::updateResultDisplayer()
{
    QList<QGraphicsItem *>::iterator i;
    for( i = resultDraw.begin(); i != resultDraw.end(); i++) {
        QGraphicsItem *oldResult = *i;
        resultZone->removeFromGroup(oldResult);
        resultDraw.removeOne(oldResult);
        delete oldResult;
    }

    if( slice ) {
        QTime myTimer;
        myTimer.start();
        QPixmap segmentationResult = convertSegmentationResult(slice->getSegmentationResult());
        QGraphicsPixmapItem* r = this->setResultPixmap( segmentationResult );
        qDebug() << "Conversion Time: " << myTimer.elapsed() << " ms";
    }
}

QGraphicsPixmapItem *MyQGraphicsScene::setResultPixmap(QPixmap result)
{
    QGraphicsPixmapItem* r = new QGraphicsPixmapItem(result);
    resultDraw.append(r);
    resultZone->addToGroup(r);

    return r;
}

void MyQGraphicsScene::setGridVisibility(bool visible)
{
    this->gridVisible = visible;
}

QGraphicsItemGroup *MyQGraphicsScene::getResultItemGroup()
{
    return this->resultZone;
}

QGraphicsPixmapItem *MyQGraphicsScene::getSlicePixmapItem()
{
    return this->slicePixmapItem;
}

void MyQGraphicsScene::mouseMoveEvent( QGraphicsSceneMouseEvent * mouseEvent )
{
    QPointF currentPosition = mouseEvent->scenePos();

    if( !this->slicePixmapItem->boundingRect().contains(currentPosition) ) return;

    if( mouseEvent->buttons() & Qt::LeftButton ) {
        if( firstClick ) {
            firstClick = false;
            firstPoint = currentPosition;
        } else {
            QGraphicsRectItem *rectangle = drawRectangle(firstPoint, currentPosition);
            QColor rectangleColor(0, 114, 202);
            QPen pen(rectangleColor, 2);
            pen.setCosmetic(true);
            rectangle->setPen(pen);

            rectangleColor.setAlpha(30);
            QBrush brush(rectangleColor);
            rectangle->setBrush(brush);

            if(itemDraw.size() > 0){
                QGraphicsItem *item = itemDraw.at(0);
                itemDraw.clear();
                drawZone->removeFromGroup(item);
                itemDraw.removeOne(item);
                delete item;
            }

            itemDraw.append(rectangle);
            drawZone->addToGroup(rectangle);

            lastPoint = currentPosition;
        }
    }

    emit mouseMoved( currentPosition );
}


void MyQGraphicsScene::mouseReleaseEvent( QGraphicsSceneMouseEvent * mouseEvent )
{
    if(!firstClick && itemDraw.size() > 0){

        float width = abs( lastPoint.x() - firstPoint.x() );
        float height = abs( lastPoint.y() - firstPoint.y() );

        if( width > 2 && height > 2 ) {
            float x = std::min( lastPoint.x(), firstPoint.x() );
            float y = std::min( lastPoint.y(), firstPoint.y() );

            emit drawnRectangle( x, y, width, height );
        }

        QGraphicsItem *addedRectangle = itemDraw.at(0);
        itemDraw.clear();
        drawZone->removeFromGroup(addedRectangle);
        itemDraw.removeOne(addedRectangle);

        delete addedRectangle;
        firstClick = true;
    }
}

inline qreal round(qreal val, int step) {
   int tmp = int(val) + step /2;
   tmp -= tmp % step;
   return qreal(tmp);
}

void MyQGraphicsScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    if( slice && gridVisible ) {
        int step = slice->getMinimumFeatureSize();
        QPen pen(QColor(200, 200, 255, 100), 2);
        pen.setCosmetic(true);
        painter->setPen(pen);
        // draw horizontal grid
        qreal start = round(rect.top(), step);
        if (start > rect.top()) {
            start -= step;
        }
        for (qreal y = start - step; y < rect.bottom(); ) {
            y += step;
            painter->drawLine(rect.left(), y, rect.right(), y);
        }
        // now draw vertical grid
        start = round(rect.left(), step);
        if (start > rect.left()) {
            start -= step;
        }
        for (qreal x = start - step; x < rect.right(); ) {
            x += step;
            painter->drawLine(x, rect.top(), x, rect.bottom());
        }
    }
}

QGraphicsRectItem *MyQGraphicsScene::drawRectangle(QPointF pointHG, QPointF pointBD)
{
    qreal x = pointHG.x();
    qreal y = pointHG.y();
    qreal w = pointBD.x() - x;
    qreal h = pointBD.y() - y;

    if(w < 0){
        x = pointBD.x();
        w = -w;
    }
    if(h < 0){
        y = pointBD.y();
        h = - h;
    }
    return new QGraphicsRectItem(x, y, w, h);
}
