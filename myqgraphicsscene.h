#ifndef MYQGRAPHICSSCENE_H
#define MYQGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

class MyQGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
public:
    MyQGraphicsScene(QObject *parent = 0);

    void resetSliceDisplayer();
    QGraphicsPixmapItem* setSlicePixmap( QPixmap pix );

    void resetSeedsDisplayer();
    QGraphicsRectItem* addSeed( qreal x, qreal y, qreal w, qreal h, const QPen &pen = QPen(), const QBrush &brush = QBrush() );

    void resetResultDisplayer();
    QGraphicsPixmapItem* setResultPixmap( QPixmap pix );

public slots:
    void mouseMoveEvent( QGraphicsSceneMouseEvent * mouseEvent );
    void mouseReleaseEvent( QGraphicsSceneMouseEvent * mouseEvent );

private:
    bool firstClick;
    QPointF firstPoint;
    QPointF lastPoint;

    QList<QGraphicsItem *> resultDraw;
    QGraphicsItemGroup * resultZone;

    QList<QGraphicsItem *> sliceDraw;
    QGraphicsItemGroup * sliceZone;

    QList<QGraphicsItem *> seedsDraw;
    QGraphicsItemGroup * seedsZone;

    QList<QGraphicsItem *> itemDraw;
    QGraphicsItemGroup * drawZone;


    QGraphicsRectItem *drawRectangle( QPointF pointHG, QPointF pointBD );

signals:
    void drawnRectangle( float x, float y, float width, float height );
    void mouseMoved( QPointF mousePosition );
};

#endif // MYQGRAPHICSSCENE_H
