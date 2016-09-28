#include "myqgraphicsscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItemGroup>

MyQGraphicsScene::MyQGraphicsScene(QObject *parent) :
    QGraphicsScene(parent)
{
    slice = NULL;

    firstClick = true;

    sliceZone = createItemGroup(sliceDraw);
    sliceZone->setZValue(1);

    seedsZone = createItemGroup(seedsDraw);
    seedsZone->setZValue(2);

    drawZone = createItemGroup(itemDraw);
    drawZone->setZValue(3);

    resultZone = createItemGroup(resultDraw);
    resultZone->setZValue(4);

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

void MyQGraphicsScene::updateSliceDisplayer()
{
    QList<QGraphicsItem *>::iterator i;
    for( i = sliceDraw.begin(); i != sliceDraw.end(); i++) {
        QGraphicsItem *oldSlice = *i;
        sliceZone->removeFromGroup(oldSlice);
        sliceDraw.removeOne(oldSlice);
        delete oldSlice;
    }

    if( slice ) {
        QPixmap image( slice->getFilename().data() );
        this->slicePixmapItem = this->setSlicePixmap( image );
    }
}

QGraphicsPixmapItem *MyQGraphicsScene::setSlicePixmap(QPixmap pix)
{
    QGraphicsPixmapItem* p = new QGraphicsPixmapItem(pix);
    sliceDraw.append(p);
    sliceZone->addToGroup(p);

    return p;
}

const char* colors[] = {
    "#FF0000", "#00FF00", "#0000FF", "#FFFF00", "#FF00FF", "#00FFFF",
    "#800000", "#008000", "#000080", "#808000", "#800080", "#008080", "#808080",
    "#C00000", "#00C000", "#0000C0", "#C0C000", "#C000C0", "#00C0C0", "#C0C0C0",
    "#400000", "#004000", "#000040", "#404000", "#400040", "#004040", "#404040",
    "#200000", "#002000", "#000020", "#202000", "#200020", "#002020", "#202020",
    "#600000", "#006000", "#000060", "#606000", "#600060", "#006060", "#606060",
    "#A00000", "#00A000", "#0000A0", "#A0A000", "#A000A0", "#00A0A0", "#A0A0A0",
    "#E00000", "#00E000", "#0000E0", "#E0E000", "#E000E0", "#00E0E0", "#E0E0E0"
};

QColor getColor(int seedId)
{
    int colorIndex = seedId % 55;
    QColor color;
    color.setNamedColor( colors[colorIndex] );
    return color;
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

        for( int i = 0; i < seeds.size(); i++ ) {
            Seed seed = seeds[i];
            if( seed.active ) {
                QPen pen( getColor(seed.id), 4 );
                this->addSeed(seed.a.x, seed.a.y, seed.b.x - seed.a.x, seed.b.y - seed.a.y, pen);
            }
        }
    }
}

QGraphicsRectItem *MyQGraphicsScene::addSeed(qreal x, qreal y, qreal w, qreal h, const QPen &pen, const QBrush &brush)
{
    QGraphicsRectItem* r = new QGraphicsRectItem(x, y, w, h);
    r->setPen(pen);
    r->setBrush(brush);
    seedsDraw.append(r);
    seedsZone->addToGroup(r);

    return r;
}

QPixmap convertSegmentationResult(cv::Mat labels) {
    QImage result(labels.cols, labels.rows, QImage::Format_ARGB32);

    for( int y = 0; y < labels.rows; y++ ) {
        for( int x = 0; x < labels.cols; x++ ) {
            uchar label = labels.at<uchar>(y, x);
            if( label != EMPTY ) {
                result.setPixel(x, y, getColor(label).rgb());
            }
        }
    }

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
        QPixmap segmentationResult = convertSegmentationResult(slice->getSegmentationResult());
        QGraphicsPixmapItem* r = this->setResultPixmap( segmentationResult );
    }
}

QGraphicsPixmapItem *MyQGraphicsScene::setResultPixmap(QPixmap result)
{
    QGraphicsPixmapItem* r = new QGraphicsPixmapItem(result);
    resultDraw.append(r);
    resultZone->addToGroup(r);

    return r;
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

    if( sliceDraw.isEmpty() || !sliceDraw.back()->boundingRect().contains(currentPosition) ) return;

    if( mouseEvent->buttons() & Qt::LeftButton ) {
        if( firstClick ) {
            firstClick = false;
            firstPoint = currentPosition;
        } else {
            QGraphicsRectItem *rectangle = drawRectangle(firstPoint, currentPosition);
            rectangle->setPen(QPen(QColor(0, 200, 0), 3));
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
