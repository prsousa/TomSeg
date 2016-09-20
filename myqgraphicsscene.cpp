#include "myqgraphicsscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItemGroup>

MyQGraphicsScene::MyQGraphicsScene(QObject *parent) :
    QGraphicsScene(parent)
{
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

void MyQGraphicsScene::resetSliceDisplayer()
{
    QList<QGraphicsItem *>::iterator i;
    for( i = sliceDraw.begin(); i != sliceDraw.end(); i++) {
        QGraphicsItem *oldSlice = *i;
        sliceZone->removeFromGroup(oldSlice);
        sliceDraw.removeOne(oldSlice);
        delete oldSlice;
    }
}

QGraphicsPixmapItem *MyQGraphicsScene::setSlicePixmap(QPixmap pix)
{
    this->resetSliceDisplayer();
    this->resetResultDisplayer();
    this->resetSeedsDisplayer();

    QGraphicsPixmapItem* p = new QGraphicsPixmapItem(pix);
    sliceDraw.append(p);
    sliceZone->addToGroup(p);

    return p;
}

void MyQGraphicsScene::resetSeedsDisplayer()
{
    QList<QGraphicsItem *>::iterator i;
    for( i = seedsDraw.begin(); i != seedsDraw.end(); i++) {
        QGraphicsItem *oldSeed = *i;
        seedsZone->removeFromGroup(oldSeed);
        seedsDraw.removeOne(oldSeed);
        delete oldSeed;
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

void MyQGraphicsScene::resetResultDisplayer()
{
    // setResultPixmap
    QList<QGraphicsItem *>::iterator i;
    for( i = resultDraw.begin(); i != resultDraw.end(); i++) {
        QGraphicsItem *oldResult = *i;
        resultZone->removeFromGroup(oldResult);
        resultDraw.removeOne(oldResult);
        delete oldResult;
    }
}

QGraphicsPixmapItem *MyQGraphicsScene::setResultPixmap(QPixmap result)
{
    this->resetResultDisplayer();

    QGraphicsPixmapItem* r = new QGraphicsPixmapItem(result);
    resultDraw.append(r);
    resultZone->addToGroup(r);

    return r;
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
