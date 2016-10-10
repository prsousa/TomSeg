#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myqgraphicsscene.h"
#include "segmenterthread.h"

#include <Qt>
#include <QFileDialog>
#include <QDebug>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QLineEdit>
#include <QMessageBox>
#include <QTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    sliceScene = new MyQGraphicsScene(this);
    slicePixmapItem = sliceScene->getSlicePixmapItem();
    ui->sliceView->setScene(sliceScene);
    ui->sliceView->setMouseTracking(true);
    ui->sliceView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    ui->splitter->setStretchFactor(0,1);

    QObject::connect(sliceScene, SIGNAL(drawnRectangle(float, float, float, float)),
                         this, SLOT(seedCreated(float, float, float, float)));

    QObject::connect(sliceScene, SIGNAL(mouseMoved(QPointF)),
                         this, SLOT(sliceSceneMouseMoved(QPointF)));

    currentSliceIndex = 0;
    ui->currentSliceNumberSpinner->setMaximum( 0 );
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateSeedsTable()
{
    ui->seedsTableWidget->setRowCount(0);

    Slice* slice = segManager.getSlice(currentSliceIndex);
    std::vector<Seed> seeds = slice->getSeeds();

    uchar color[3];

    for( size_t i = 0; i < seeds.size(); i++ ) {
        Seed seed = seeds[i];

        QTableWidgetItem* colorItem = new QTableWidgetItem();
        QTableWidgetItem* xItem = new QTableWidgetItem();
        QTableWidgetItem* yItem = new QTableWidgetItem();
        QTableWidgetItem* widthItem = new QTableWidgetItem();
        QTableWidgetItem* heightItem = new QTableWidgetItem();

        Seed::getColor( seed.getId(), color );
        colorItem->setData(Qt::BackgroundRole, QColor( color[0], color[1], color[2] ));
        colorItem->setCheckState( seed.active ? Qt::Checked : Qt::Unchecked );
        xItem->setText( QString::number(seed.a.x) );
        yItem->setText( QString::number(seed.a.y) );
        widthItem->setText( QString::number(seed.b.x - seed.a.x) );
        heightItem->setText( QString::number(seed.b.y - seed.a.y) );

        ui->seedsTableWidget->insertRow(i);

        ui->seedsTableWidget->setItem(i, 0, colorItem);
        ui->seedsTableWidget->setItem(i, 1, xItem);
        ui->seedsTableWidget->setItem(i, 2, yItem);
        ui->seedsTableWidget->setItem(i, 3, widthItem);
        ui->seedsTableWidget->setItem(i, 4, heightItem);
    }
}

void MainWindow::on_seedsTableWidget_itemSelectionChanged()
{
    // activates and desactivates Remove button
    int nSelectedRows = ui->seedsTableWidget->selectionModel()->selectedRows().size();
    ui->removeSeedButton->setEnabled( nSelectedRows > 0 );
}

void MainWindow::on_seedsTableWidget_itemChanged(QTableWidgetItem* item)
{
    int rowIndex = item->row();
    int colIndex = item->column();

    Slice* slice = segManager.getSlice(currentSliceIndex);
    Seed& seed = slice->getSeeds()[rowIndex];

    switch (colIndex) {
    case 0:
    {
        seed.active = item->checkState() == Qt::Checked;
        break;
    }
    case 1:
    {
        seed.a.x = item->text().toInt();
        break;
    }
    case 2:
    {
        seed.a.y = item->text().toInt();
        break;
    }
    case 3:
    {
        int width = item->text().toInt();
        seed.b.x = seed.a.x + width;
        break;
    }
    case 4:
    {
        int height = item->text().toInt();
        seed.b.y = seed.a.y + height;
        break;
    }
    default:
        break;
    }

    sliceScene->updateSeedsDisplayer();
}

void MainWindow::on_removeSeedButton_released()
{
    QModelIndexList  selectedRowIndexes = ui->seedsTableWidget->selectionModel()->selectedRows();

    Slice* slice = segManager.getSlice(currentSliceIndex);

    for( int i = selectedRowIndexes.size() - 1; i >= 0; i-- ) {
        int rowIndex = selectedRowIndexes.at(i).row();
        slice->removeSeed(rowIndex); // possible bug: seeds indexes will chage after erase.
    }

    updateSeedsTable();
    sliceScene->updateSeedsDisplayer();
}

void MainWindow::on_gridVisibleCheckBox_toggled(bool gridVisible)
{
    sliceScene->setGridVisibility(gridVisible);
    sliceScene->update();
}

void MainWindow::on_minimumFeatureSizeSpinBox_valueChanged(int newMinimumFeatureSize)
{
    if( segManager.isEmpty() ) return;

    Slice* slice = segManager.getSlice(currentSliceIndex);
    slice->setMinimumFeatureSize(newMinimumFeatureSize);

    if( ui->gridVisibleCheckBox->checkState() == Qt::Checked ) {
        sliceScene->update();
    }
}

void MainWindow::setCurrentSlice(int sliceNumber = 0)
{
    qDebug() << "Set Current Slice";

    currentSliceIndex = sliceNumber;
    ui->currentSliceNumberSpinner->setValue( sliceNumber + 1 );
    ui->sliceSlider->setValue( sliceNumber + 1 );

    Slice* slice = segManager.getSlice(sliceNumber);
    cv::Mat& image = slice->getImg();

    sliceScene->setSlice(slice);
    if( autoFitScreen ) {
        zoomFit();
    }

    ui->currentSliceSizeLabel->setText( QString::number( image.cols ) + " x " + QString::number( image.rows ) );
    ui->minimumFeatureSizeSpinBox->setValue( slice->getMinimumFeatureSize() );

    ui->leftCropSpinBox->setMaximum( image.cols );
    ui->rightCropSpinBox->setMaximum( image.cols );
    ui->topCropSpinBox->setMaximum( image.rows );
    ui->bottomCropSpinBox->setMaximum( image.rows );

    updateSeedsTable();
}

void MainWindow::updateAlign()
{
    size_t x = ui->referenceAreaInitXSpinBox->value();
    size_t y = ui->referenceAreaInitYSpinBox->value();
    size_t width = ui->referenceAreaWidthSpinBox->value();
    size_t height = ui->referenceAreaHeightSpinBox->value();

    sliceScene->updateAlignDisplayer(x, y, width, height);
}

void MainWindow::resetAlign()
{
    ui->referenceAreaInitXSpinBox->setValue( 0 );
    ui->referenceAreaInitYSpinBox->setValue( 0 );
    ui->referenceAreaWidthSpinBox->setValue( 0 );
    ui->referenceAreaHeightSpinBox->setValue( 0 );
}

void MainWindow::updateCrop()
{
    qDebug() << "Update Crop";
    size_t left = ui->leftCropSpinBox->value();
    size_t right = ui->rightCropSpinBox->value();
    size_t top = ui->topCropSpinBox->value();
    size_t bottom = ui->bottomCropSpinBox->value();
    this->sliceScene->updateCropDisplayer(left, right, top, bottom);
}

void MainWindow::resetCrop()
{
    size_t numberOfSlices = segManager.size();

    ui->firstCropSliceSlider->setEnabled(true);
    ui->lastCropSliceSlider->setEnabled(true);
    ui->leftCropSpinBox->setEnabled(true);
    ui->rightCropSpinBox->setEnabled(true);
    ui->topCropSpinBox->setEnabled(true);
    ui->bottomCropSpinBox->setEnabled(true);

    ui->lastCropSliceSlider->setMaximum( numberOfSlices );
    ui->firstCropSliceSlider->setMaximum( numberOfSlices );
    ui->firstCropSliceSlider->setValue( 1 );
    ui->firstCropSliceSlider->setMinimum( 1 );
    ui->lastCropSliceSlider->setValue( numberOfSlices );


    // Disable value changed signal :: prevent redrawing ROI 4x
    ui->leftCropSpinBox->blockSignals(true);
    ui->rightCropSpinBox->blockSignals(true);
    ui->topCropSpinBox->blockSignals(true);
    ui->bottomCropSpinBox->blockSignals(true);

    // Set new values
    ui->leftCropSpinBox->setValue( 0 );
    ui->rightCropSpinBox->setValue( 0 );
    ui->topCropSpinBox->setValue( 0 );
    ui->bottomCropSpinBox->setValue( 0 );

    // Enable back signals
    ui->leftCropSpinBox->blockSignals(false);
    ui->rightCropSpinBox->blockSignals(false);
    ui->topCropSpinBox->blockSignals(false);
    ui->bottomCropSpinBox->blockSignals(false);

    updateCrop();
}

void MainWindow::on_addSeedButton_released()
{
    this->seedCreated(0, 0, 10, 10);
}

void MainWindow::updateSlicesUI() {
    size_t numberOfSlices = segManager.size();

    ui->currentSliceNumberSpinner->setMaximum( numberOfSlices );
    ui->currentSliceNumberSpinner->setMinimum( 1 );

    ui->sliceSlider->setMaximum( numberOfSlices );
    ui->sliceSlider->setMinimum( 1 );

    ui->firstExportSliceBox->setMinimum( 1 );
    ui->firstExportSliceBox->setMaximum( numberOfSlices );
    ui->lastExportSliceBox->setMinimum( 1 );
    ui->lastExportSliceBox->setMaximum( numberOfSlices );
    ui->lastExportSliceBox->setValue( numberOfSlices );

    ui->sliceTotalLabel->setText( QString::number(numberOfSlices) );

    ui->seedsTableWidget->setEnabled(true);
    ui->addSeedButton->setEnabled(true);
    autoFitScreen = true;

    this->setCurrentSlice(0);

    resetCrop();
}

void MainWindow::openFileDialog()
{
    QStringList filenames = QFileDialog::getOpenFileNames( this,
                                                           tr("Open Image"),
                                                           QDir::homePath(),
                                                           tr("Image Files (*.png *.jpg *.bmp *.tif)")
                                                          );

    if( !filenames.isEmpty() ) {
        std::vector<std::string> filenamesSegManager;
        for (int i = 0; i < filenames.count(); i++) {
            filenamesSegManager.push_back( filenames[i].toStdString() );
        }

        segManager.setSlices( filenamesSegManager );
        updateSlicesUI();
    }
}

void MainWindow::zoomIn()
{
    ui->sliceView->scale(1.15, 1.15);
    this->updateCurrentZoomInfo();
    autoFitScreen = false;
}

void MainWindow::zoomOut()
{
    ui->sliceView->scale(0.85, 0.85);
    this->updateCurrentZoomInfo();
    autoFitScreen = false;
}

void MainWindow::zoomZero()
{
    ui->sliceView->resetMatrix();
    this->updateCurrentZoomInfo();
    autoFitScreen = false;
}

void MainWindow::zoomFit()
{
    // scroll bars interfer with fitInView, so it has to be called twice
    // http://stackoverflow.com/questions/22614337/qt-qgraphicsscene-does-not-fitinview-with-scrollbars

    ui->sliceView->fitInView(slicePixmapItem, Qt::KeepAspectRatio);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    ui->sliceView->fitInView(slicePixmapItem, Qt::KeepAspectRatio);

    this->updateCurrentZoomInfo();
    autoFitScreen = true;
}

void MainWindow::updateCurrentZoomInfo() {
    int currentZoom = ui->sliceView->matrix().m11() * 100;
    ui->currentZoomFactorLabel->setText( QString::number(currentZoom) +  "%");
}

void MainWindow::on_actionOpen_triggered()
{
    openFileDialog();
}

void MainWindow::on_actionZoom_In_triggered()
{
    zoomIn();
}

void MainWindow::on_actionZoom_Out_triggered()
{
    zoomOut();
}

void MainWindow::on_action100_triggered()
{
    zoomZero();
}

void MainWindow::on_actionFit_on_Screen_triggered()
{
    zoomFit();
}

void MainWindow::on_lastSliceButton_released()
{
    if( !segManager.isEmpty() && currentSliceIndex < segManager.size() - 1 ) {
        setCurrentSlice(segManager.size() - 1);
    }
}

void MainWindow::on_firstSliceButton_released()
{
    if( !segManager.isEmpty() && currentSliceIndex > 0 ) {
        setCurrentSlice(0);
    }
}

void MainWindow::on_sliceSlider_valueChanged(int newSliceNumber)
{
    if( newSliceNumber > 0 && newSliceNumber <= segManager.size() && (newSliceNumber - 1) != currentSliceIndex ) {
        setCurrentSlice(newSliceNumber - 1);
    }
}

void MainWindow::on_currentSliceNumberSpinner_valueChanged(int newSliceNumber)
{
    if( newSliceNumber > 0 && newSliceNumber <= segManager.size() && newSliceNumber != (currentSliceIndex + 1) ) {
        currentSliceIndex = newSliceNumber - 1;
        setCurrentSlice(currentSliceIndex);
        updateSeedsTable();
    } else {
        ui->currentSliceNumberSpinner->setValue(currentSliceIndex + 1);
    }
}

void MainWindow::on_goButton_released()
{
    QMessageBox messageBox;
    if( segManager.isEmpty() ) {
        messageBox.critical(this, "Error", "You must open the slice(s) first!");
        return;
    }

    Slice* slice = segManager.getSlice(currentSliceIndex);
    if( slice->getSeeds().size() < 2 ) {
        messageBox.critical(this, "Error", "You must define at least two seeds");
        return;
    }

    //SegmenterThread* segThread = new SegmenterThread(&this->segManager, currentSliceIndex); // TODO: memory leak on thread's object
    //segThread->start();
    QTime myTimer;
    myTimer.start();

    segManager.apply(currentSliceIndex);

    qDebug() << "Segmentation Time: " << myTimer.elapsed() << " ms";

    sliceScene->updateResultDisplayer();
}

void MainWindow::on_exportSegmentationButton_released()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Destination Directory"),
                                                QDir::homePath(),
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);

    if( !dir.isEmpty() ) {
        bool exportAllSlices = ui->exportAllSlicesCheckBox->isChecked();

        if( exportAllSlices ) {
            segManager.exportResult( dir.toStdString() );
        } else {
            size_t firstSlice = ui->firstExportSliceBox->value();
            size_t lastSlice = ui->lastExportSliceBox->value();
            segManager.exportResult( dir.toStdString(), firstSlice, lastSlice );
        }
    }
}

void MainWindow::on_resetSegmentationButton_released()
{
    segManager.resetResults();
    sliceScene->updateResultDisplayer();
}

void MainWindow::on_referenceAreaInitXSpinBox_valueChanged(int newInitX)
{
    updateAlign();
}

void MainWindow::on_referenceAreaInitYSpinBox_valueChanged(int newInitY)
{
    updateAlign();
}

void MainWindow::on_referenceAreaWidthSpinBox_valueChanged(int newWidth)
{
    updateAlign();
}

void MainWindow::on_referenceAreaHeightSpinBox_valueChanged(int newHeight)
{
    updateAlign();
}

void MainWindow::on_resetAlignButton_released()
{
    resetAlign();
}

void MainWindow::on_exportSlicesButton_released()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Destination Directory"),
                                                QDir::homePath(),
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);

    if( !dir.isEmpty() ) {
        bool exportAllSlices = ui->exportAllSlicesCheckBox->isChecked();

        if( exportAllSlices ) {
            segManager.exportSlicesImages( dir.toStdString() );
        } else {
            size_t firstSlice = ui->firstExportSliceBox->value();
            size_t lastSlice = ui->lastExportSliceBox->value();
            segManager.exportSlicesImages( dir.toStdString(), firstSlice, lastSlice );
        }
    }
}

void MainWindow::on_alignButton_released()
{
    Point a;
    a.x = ui->referenceAreaInitXSpinBox->value();
    a.y = ui->referenceAreaInitYSpinBox->value();
    int width = ui->referenceAreaWidthSpinBox->value();
    int height = ui->referenceAreaHeightSpinBox->value();
    int maxDeltaX = ui->referenceAreaMaxDeltaXSpinBox->value();
    int maxDeltaY = ui->referenceAreaMaxDeltaYSpinBox->value();

    if( a.x > 0 && a.y > 0 && width > 0 && height > 0 ) {

        QTime myTimer;
        myTimer.start();

        segManager.alignSlices(currentSliceIndex, a, width, height, maxDeltaX, maxDeltaY);

        qDebug() << "Align Time: " << myTimer.elapsed() << " ms";

        this->sliceScene->updateSliceDisplayer();
        if( autoFitScreen ) {
            zoomFit();
        }
    }
}

void MainWindow::on_leftCropSpinBox_valueChanged(int newCropLeft)
{
    this->updateCrop();
}

void MainWindow::on_rightCropSpinBox_valueChanged(int newCropRight)
{
    this->updateCrop();
}

void MainWindow::on_topCropSpinBox_valueChanged(int newCropTop)
{
    this->updateCrop();
}

void MainWindow::on_bottomCropSpinBox_valueChanged(int newCropBottom)
{
    this->updateCrop();
}

void MainWindow::on_resetROIButton_released()
{
    resetCrop();
}

void MainWindow::on_cropButton_released()
{
    if( !segManager.isEmpty() ) {
        Slice* slice = segManager.getSlice(currentSliceIndex);
        cv::Mat& sliceImg = slice->getImg();

        size_t firstSlice = ui->firstCropSliceSlider->value();
        size_t lastSlice = ui->lastCropSliceSlider->value();
        size_t left = ui->leftCropSpinBox->value();
        size_t right = ui->rightCropSpinBox->value();
        size_t top = ui->topCropSpinBox->value();
        size_t bottom = ui->bottomCropSpinBox->value();

        segManager.cropSlices(firstSlice, lastSlice, Point(left, top), sliceImg.cols - left - right, sliceImg.rows - top - bottom);

        updateSlicesUI();
    }
}

void MainWindow::on_exportAllSlicesCheckBox_toggled(bool exportAllSlices)
{
    ui->toExportLabel->setEnabled( !exportAllSlices );
    ui->firstExportSliceBox->setEnabled( !exportAllSlices );
    ui->lastExportSliceBox->setEnabled( !exportAllSlices );
}

void MainWindow::on_toolsTab_currentChanged(int newTabIndex)
{
    sliceScene->setReferenceAreaVisibility( newTabIndex == 0 );
    sliceScene->setROIVisibility( newTabIndex == 1 );
    sliceScene->setResultVisibility( newTabIndex >= 2 );
}

void MainWindow::on_moreZoomButton_released()
{
    zoomIn();
}

void MainWindow::on_lessZoomButton_released()
{
    zoomOut();
}

void MainWindow::on_noZoomButton_released()
{
    zoomZero();
}

void MainWindow::on_fitZoomButton_released()
{
    zoomFit();
}

void MainWindow::on_resultOpacitySlider_valueChanged( int newOpacity )
{
    sliceScene->getResultItemGroup()->setOpacity(newOpacity / 100.f);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    if ( autoFitScreen ){
       zoomFit();
    }
}

void MainWindow::seedCreated( float x, float y, float width, float height )
{
    if( segManager.isEmpty() ) return;

    Slice* slice = segManager.getSlice(currentSliceIndex);

    switch ( ui->toolsTab->currentIndex() ) {
    case 2:
    {
        // Segmentation Tab is selected
        std::vector<Seed>& seeds = slice->getSeeds();

        Seed newSeed( slice->getImg(), seeds.size(), Point(x, y), Point(x + width, y + height) );
        seeds.push_back(newSeed);

        sliceScene->updateSeedsDisplayer();
        updateSeedsTable();

        break;
    }
    case 1:
    {
        cv::Mat& sliceImg = slice->getImg();

        // Disable value changed signal :: prevent redrawing ROI 4x
        ui->leftCropSpinBox->blockSignals(true);
        ui->rightCropSpinBox->blockSignals(true);
        ui->topCropSpinBox->blockSignals(true);
        ui->bottomCropSpinBox->blockSignals(true);

        // Set new values
        ui->leftCropSpinBox->setValue( x );
        ui->rightCropSpinBox->setValue( sliceImg.cols - x - width );
        ui->topCropSpinBox->setValue( y );
        ui->bottomCropSpinBox->setValue( sliceImg.rows - y - height );

        // Enable back signals
        ui->leftCropSpinBox->blockSignals(false);
        ui->rightCropSpinBox->blockSignals(false);
        ui->topCropSpinBox->blockSignals(false);
        ui->bottomCropSpinBox->blockSignals(false);

        updateCrop();

        break;
    }
    case 0:
    {
        // Align Tab is selected
        ui->referenceAreaInitXSpinBox->setValue(x);
        ui->referenceAreaInitYSpinBox->setValue(y);
        ui->referenceAreaWidthSpinBox->setValue(width);
        ui->referenceAreaHeightSpinBox->setValue(height);

        break;
    }
    default:
        break;
    }
}

void MainWindow::sliceSceneMouseMoved(QPointF mousePosition)
{
    ui->statusBar->showMessage( QString::number( mousePosition.x(), 'f', 0) + "x" + QString::number( mousePosition.y(), 'f', 0) );
    ui->statusBar->update();
}
