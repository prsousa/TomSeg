#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "myqgraphicsscene.h"

#include "segmentation-manager/segmentationmanager.h"

#include <QMainWindow>
#include <QVector>
#include <QGraphicsScene>
#include <QTableWidgetItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_actionOpen_triggered();
    void on_actionZoom_In_triggered();
    void on_actionZoom_Out_triggered();
    void on_action100_triggered();
    void on_actionFit_on_Screen_triggered();
    void on_addSeedButton_released();
    void on_seedsTableWidget_itemSelectionChanged();
    void on_seedsTableWidget_itemChanged(QTableWidgetItem* item);
    void on_removeSeedButton_released();
    void on_gridVisibleCheckBox_toggled(bool gridVisible);
    void on_minimumFeatureSizeSpinBox_valueChanged(int newMinimumFeatureSize);
    void on_lastSliceButton_released();
    void on_firstSliceButton_released();
    void on_sliceSlider_valueChanged(int newSliceNumber);
    void on_currentSliceNumberSpinner_valueChanged(int newSliceNumber);

    void on_resetSeedsButton_released();

    void on_propagateSeedsButton_released();
    void on_goButton_released();
    void on_resetSegmentationButton_released();

    void on_referenceAreaInitXSpinBox_valueChanged( int newInitX );
    void on_referenceAreaInitYSpinBox_valueChanged( int newInitY );
    void on_referenceAreaWidthSpinBox_valueChanged( int newWidth );
    void on_referenceAreaHeightSpinBox_valueChanged( int newHeight );
    void on_resetAlignButton_released();
    void on_alignButton_released();

    void on_leftCropSpinBox_valueChanged(int newCropLeft);
    void on_rightCropSpinBox_valueChanged(int newCropRight);
    void on_topCropSpinBox_valueChanged(int newCropTop);
    void on_bottomCropSpinBox_valueChanged(int newCropBottom);
    void on_setCurrentIndexAsFirstCutSliceButton_released();
    void on_setCurrentIndexAsLastCutSliceButton_released();
    void on_firstCropSliceSlider_valueChanged( int newFirstSlice );
    void on_lastCropSliceSlider_valueChanged( int newLastSlice );
    void on_resetROIButton_released();
    void on_cropButton_released();

    void on_exportAllSlicesCheckBox_toggled(bool exportAllSlices);
    void on_exportSlicesButton_released();
    void on_exportSegmentationButton_released();

    void on_toolsTab_currentChanged(int newTabIndex);

    void on_moreZoomButton_released();
    void on_lessZoomButton_released();
    void on_noZoomButton_released();
    void on_fitZoomButton_released();
    void on_resultOpacitySlider_valueChanged(int newOpacity);
    void resizeEvent(QResizeEvent* event);
    void seedCreated(float x, float y, float width, float height);
    void sliceSceneMouseMoved(QPointF mousePosition);

private:
    Ui::MainWindow *ui;
    MyQGraphicsScene* sliceScene;
    QGraphicsPixmapItem* slicePixmapItem;
    bool autoFitScreen;

    SegmentationManager segManager;

    int currentSliceIndex;

    void updateSlicesUI();
    void openFileDialog();
    void zoomIn();
    void zoomOut();
    void zoomZero();
    void zoomFit();
    void updateCurrentZoomInfo();
    void setCurrentSlice(int sliceNumber);
    void updateAlign();
    void resetAlign();
    void updateCrop();
    void resetCrop();
    void updateSeedsTable();
    QPixmap convertSegmentationResult(cv::Mat labels);
};

#endif // MAINWINDOW_H
