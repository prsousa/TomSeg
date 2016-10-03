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
    void on_goButton_released();
    void on_resetButton_released();
    void on_exportSlicesButton_released();
    void on_alignButton_released();
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
    bool autoFitScreen;

    SegmentationManager segManager;

    int currentSliceIndex;

    void openFileDialog();
    void zoomIn();
    void zoomOut();
    void zoomZero();
    void zoomFit();
    void updateCurrentZoomInfo();
    void setCurrentSlice(int sliceNumber);
    void updateSeedsTable();
    QPixmap convertSegmentationResult(cv::Mat labels);
};

#endif // MAINWINDOW_H
