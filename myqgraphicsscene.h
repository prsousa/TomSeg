#ifndef MYQGRAPHICSSCENE_H
#define MYQGRAPHICSSCENE_H

#include "segmentation-manager/slice.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

class MyQGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    MyQGraphicsScene(QObject *parent = 0);

    void setSlice( Slice* slice );
    void updateSliceDisplayer();
    void updateAlignDisplayer(size_t left, size_t right, size_t top, size_t bottom);
    void updateCropDisplayer(size_t left, size_t right, size_t top, size_t bottom);
    void updateSeedsDisplayer();
    void updateResultDisplayer();
    QGraphicsItemGroup* getResultItemGroup();
    QGraphicsPixmapItem* getSlicePixmapItem();
    void setROIVisibility( bool visible );
    void setGridVisibility( bool visible );
    void setResultVisibility( bool visible );
    void setReferenceAreaVisibility( bool visible );

    void setGridSize(int value);

public slots:
    void mouseMoveEvent( QGraphicsSceneMouseEvent * mouseEvent );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent * mouseEvent );
    void drawForeground( QPainter *painter, const QRectF &rect );

private:
    Slice* slice;
    bool firstClick;
    QPointF firstPoint;
    QPointF lastPoint;
    bool gridVisible;
    int gridSize;

    QGraphicsPixmapItem* slicePixmapItem;
    QGraphicsRectItem* referenceAreaItem;

    QList<QGraphicsItem *> resultDraw;
    QGraphicsItemGroup * resultZone;

    QList<QGraphicsItem *> seedsDraw;
    QGraphicsItemGroup * seedsZone;

    QList<QGraphicsItem *> cropDraw;
    QGraphicsItemGroup * cropZone;

    QList<QGraphicsItem *> itemDraw;
    QGraphicsItemGroup * drawZone;


    QGraphicsRectItem* addSeed( qreal x, qreal y, qreal w, qreal h, const QColor& seedColor = QColor() );
    QGraphicsPixmapItem* setResultPixmap( QPixmap pix );

    QGraphicsRectItem *drawRectangle( QPointF pointHG, QPointF pointBD );

signals:
    void drawnRectangle( float x, float y, float width, float height );
    void mouseMoved( QPointF mousePosition );
};

#endif // MYQGRAPHICSSCENE_H
