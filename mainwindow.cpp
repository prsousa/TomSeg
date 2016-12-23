#include <chrono>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myqgraphicsscene.h"
#include "segmenterthread.h"

#include <Qt>
#include <QFileDialog>
#include <QInputDialog>
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
                         this, SLOT(areaSelected(float, float, float, float)));

    QObject::connect(sliceScene, SIGNAL(mouseMoved(QPointF)),
                         this, SLOT(sliceSceneMouseMoved(QPointF)));

    currentSliceIndex = 0;

    // charts
    histogramView = new HistogramView(this);
    ui->histogramVerticalLayout->removeWidget( ui->histogramDisplayer );
    ui->histogramVerticalLayout->addWidget( histogramView );

    // status bar
    infoLabel = new QLabel(this);
    ui->statusBar->addPermanentWidget( infoLabel );

    autoFitScreen = true;

    updateSlicesUI();
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
        colorItem->setText( QString::number(seed.getId()) );
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
    if( ui->seedsTableWidget->selectedItems().isEmpty() ) return;

    int rowIndex = item->row();
    int colIndex = item->column();

    Slice* slice = segManager.getSlice(currentSliceIndex);
    Seed& seed = slice->getSeeds()[rowIndex];

    int id = ui->seedsTableWidget->item( rowIndex, 0 )->text().toInt();
    int x = ui->seedsTableWidget->item( rowIndex, 1 )->text().toInt();
    int y = ui->seedsTableWidget->item( rowIndex, 2 )->text().toInt();
    int width = ui->seedsTableWidget->item( rowIndex, 3 )->text().toInt();
    int height = ui->seedsTableWidget->item( rowIndex, 4 )->text().toInt();
    Point a( x, y );
    Point b( x + width, y + height );
    Seed newSeed(slice->getImg(), id, a, b);

    slice->getSeeds()[rowIndex] = newSeed;

    sliceScene->updateSeedsDisplayer();
    this->updateSeedsTable();
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
    sliceScene->setGridSize( segManager.getMinimumFeatureSize() );
    sliceScene->update();
}

void MainWindow::on_minimumFeatureSizeSpinBox_valueChanged(int newMinimumFeatureSize)
{
    segManager.setMinimumFeatureSize(newMinimumFeatureSize);
    sliceScene->setGridSize(newMinimumFeatureSize);

    if( ui->gridVisibleCheckBox->checkState() == Qt::Checked ) {
        sliceScene->update();
    }
}

void MainWindow::on_morphologicalCheckBox_toggled(bool morphActive)
{
    ui->morphologicalSizeSpinBox->setEnabled( morphActive );
}

void MainWindow::setCurrentSlice(int sliceNumber = 0)
{
    if ( sliceNumber >= segManager.size() ) return;
    // qDebug() << "Set Current Slice";

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

    ui->leftCropSpinBox->setMaximum( image.cols );
    ui->rightCropSpinBox->setMaximum( image.cols );
    ui->topCropSpinBox->setMaximum( image.rows );
    ui->bottomCropSpinBox->setMaximum( image.rows );

    updateSeedsTable();

    histogramView->setSlice( slice );
    histogramView->update();
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
//    qDebug() << "Update Crop";
    size_t left = ui->leftCropSpinBox->value();
    size_t right = ui->rightCropSpinBox->value();
    size_t top = ui->topCropSpinBox->value();
    size_t bottom = ui->bottomCropSpinBox->value();
    this->sliceScene->updateCropDisplayer(left, right, top, bottom);
}

void MainWindow::resetCrop()
{
    size_t numberOfSlices = segManager.size();

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
    this->areaSelected(0, 0, 10, 10);
}

void MainWindow::updateSlicesUI() {
    size_t numberOfSlices = segManager.size();

    ui->splitter->setEnabled( numberOfSlices > 0 );
    if( numberOfSlices ) {
        ui->currentSliceNumberSpinner->setMaximum( numberOfSlices );
        ui->sliceTotalLabel->setText( QString::number(numberOfSlices) );
        ui->sliceSlider->setMaximum( numberOfSlices );

        ui->firstExportSliceBox->setMaximum( numberOfSlices );
        ui->lastExportSliceBox->setMaximum( numberOfSlices );
        ui->lastExportSliceBox->setValue( numberOfSlices );

        this->setCurrentSlice(0);

        ui->minimumFeatureSizeSpinBox->setValue( segManager.getMinimumFeatureSize() );
        ui->morphologicalSizeSpinBox->setValue( segManager.getMorphologicalSize() );

        ui->actionUseGPU->triggered( segManager.getUseGPU() );

        resetCrop();
    }

    updateSaveStatus();
}

void MainWindow::importFileDialog()
{
    std::chrono::steady_clock::time_point beginFilePicking = std::chrono::steady_clock::now();
    QStringList filenames = QFileDialog::getOpenFileNames( this,
                                                           tr("Import Images"),
                                                           QDir::homePath(),
                                                           tr("Image Files (*.png *.jpg *.bmp *.tif)")
                                                          );
    std::chrono::steady_clock::time_point endFilePicking = std::chrono::steady_clock::now();
    qDebug() << "SelectFiles:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(endFilePicking - beginFilePicking).count();


    if( !filenames.isEmpty() ) {
        std::vector<std::string> filenamesSegManager;
        for (int i = 0; i < filenames.count(); i++) {
            filenamesSegManager.push_back( filenames[i].toStdString() );
        }

        segManager = SegmentationManager();
        segManager.setSlices( filenamesSegManager );
        updateSlicesUI();
    }
}

void MainWindow::openProjectDialog()
{
    std::chrono::steady_clock::time_point beginFilePicking = std::chrono::steady_clock::now();

    std::string projectFolderPath = segManager.getProjectFolderPath();
    QString defaultDir = (projectFolderPath.empty()) ? QDir::homePath() : QString::fromStdString(projectFolderPath);
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Project"),
                                                    defaultDir,
                                                    tr("TomSeg (*.tms)"));
    std::chrono::steady_clock::time_point endFilePicking = std::chrono::steady_clock::now();
    qDebug() << "SelectProjectFile:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(endFilePicking - beginFilePicking).count();

    if( !filename.isEmpty() ) {
        segManager = SegmentationManager( filename.toStdString() );

        autoFitScreen = true;
        updateSlicesUI();
    }
}

void MainWindow::saveProjectDialog()
{
    std::string projectFolderPath = segManager.getProjectFolderPath();
    QString defaultDir = (projectFolderPath.empty()) ? QDir::homePath() : QString::fromStdString(projectFolderPath);
    QString filename = QFileDialog::getSaveFileName(
            this,
            tr("Save Project As"),
            defaultDir,
            tr("TomSeg (*.tms)") );

    if( !filename.isEmpty() ) {
        bool exportAllSlices = ui->exportAllSlicesCheckBox->isChecked();

        if( exportAllSlices ) {
            segManager.exportProject( filename.toStdString() );
        } else {
            size_t firstSlice = ui->firstExportSliceBox->value();
            size_t lastSlice = ui->lastExportSliceBox->value();
            segManager.exportSlicesImages( filename.toStdString(), firstSlice, lastSlice );
        }
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

void MainWindow::updateSaveStatus()
{
    std::string projectFilename = segManager.getProjectFilename();
    ui->actionSave->setEnabled( !projectFilename.empty() );
    ui->actionSave_As->setEnabled( segManager.size() > 0 );

    std::string projectName = projectFilename.empty() ? "Untitled" : projectFilename;
    this->setWindowTitle("Tom Seg - " + QString::fromStdString(projectName) );
}

void MainWindow::updateCurrentZoomInfo() {
    int currentZoom = ui->sliceView->matrix().m11() * 100;
    ui->currentZoomFactorLabel->setText( QString::number(currentZoom) +  "%");
}

void MainWindow::on_actionImport_triggered()
{
    importFileDialog();
}

void MainWindow::on_actionOpen_triggered()
{
    openProjectDialog();
}

void MainWindow::on_actionSave_triggered()
{
    segManager.exportProject( segManager.getProjectPath() );
    updateSaveStatus();
}

void MainWindow::on_actionSave_As_triggered()
{
    saveProjectDialog();
    updateSaveStatus();
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

void MainWindow::on_actionUseGPU_triggered(bool selected)
{
    this->segManager.setUseGPU(selected);
    ui->actionUseGPU->setChecked( selected );
    this->infoLabel->setText( selected ? "GPU" : "CPU" );
    ui->statusBar->update();
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

void MainWindow::on_resetSeedsButton_released()
{
    QMessageBox msgBox(QMessageBox::Question,
                       "Clear Seeds", "Clear Seeds");

    QAbstractButton* resetAllSeedsButton = msgBox.addButton(tr("All"), QMessageBox::YesRole);
    QAbstractButton* startFromCurrentButton = msgBox.addButton(tr("Next Slices"), QMessageBox::NoRole);
    QAbstractButton* justCurrentButton = msgBox.addButton(tr("This Slice"), QMessageBox::NoRole);

    msgBox.exec();

    QAbstractButton* clickedButton = msgBox.clickedButton();

    if ( clickedButton == resetAllSeedsButton ) {
        segManager.resetSeeds();
    } else if( clickedButton == justCurrentButton ) {
        segManager.resetSeeds( currentSliceIndex );
    } else if( clickedButton == startFromCurrentButton ) {
        segManager.resetSeedsFrom( currentSliceIndex );
    }

    sliceScene->updateSeedsDisplayer();
    this->updateSeedsTable();
}

void MainWindow::on_propagateSeedsButton_released()
{
    bool ok;
    int stride = QInputDialog::getInt(this, tr("Propagate Seeds"),
                                 tr("Interval:"), 1, 1, 100, 1, &ok);
    if ( ok ) {
        segManager.propagateSeeds(currentSliceIndex, stride);
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

    segManager.setMorphologicalSize( 0 );
    if( ui->morphologicalCheckBox->isChecked() ) {
        segManager.setMorphologicalSize( ui->morphologicalSizeSpinBox->value() );
    }

    //SegmenterThread* segThread = new SegmenterThread(&this->segManager, currentSliceIndex); // TODO: memory leak on thread's object
    //segThread->start();

    segManager.segment();

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

    QTime myTimer;
    myTimer.start();

    if( a.x > 0 && a.y > 0 && width > 0 && height > 0 ) {
        segManager.alignSlices(currentSliceIndex, a, width, height, maxDeltaX, maxDeltaY);
    } else {
        segManager.alignSlices();
    }

    qDebug() << "Align Time: " << myTimer.elapsed() << " ms";

    resetAlign();

    this->sliceScene->updateSliceDisplayer();
    if( autoFitScreen ) {
        zoomFit();
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

void MainWindow::on_setCurrentIndexAsFirstCutSliceButton_released()
{
    ui->firstCropSliceSlider->setValue( currentSliceIndex + 1 );
}

void MainWindow::on_setCurrentIndexAsLastCutSliceButton_released()
{
    ui->lastCropSliceSlider->setValue( currentSliceIndex + 1 );
}

void MainWindow::on_firstCropSliceSlider_valueChanged(int newFirstSlice)
{
    int lastCropSlice = ui->lastCropSliceSlider->value();

    if( newFirstSlice > lastCropSlice ) {
        ui->firstCropSliceSlider->setValue( lastCropSlice );
    } else if( ui->firstSliceROICheckBox->isChecked() ) {
        ui->sliceSlider->setValue( newFirstSlice );
    }
}

void MainWindow::on_lastCropSliceSlider_valueChanged(int newLastSlice)
{
    int firstCropSlice = ui->firstCropSliceSlider->value();

    if( newLastSlice < firstCropSlice ) {
        ui->lastCropSliceSlider->setValue( firstCropSlice );
    } else if( ui->lastSliceROICheckBox->isChecked() ) {
        ui->sliceSlider->setValue( newLastSlice );
    }
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

void MainWindow::on_xLenSpinBox_valueChanged(double newValue)
{
    segManager.setXLen( newValue );
}

void MainWindow::on_yLenSpinBox_valueChanged(double newValue)
{
    segManager.setYLen( newValue );
}

void MainWindow::on_zLenSpinBox_valueChanged(double newValue)
{
    segManager.setZLen( newValue );
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

void MainWindow::areaSelected( float x, float y, float width, float height )
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
    if( segManager.isEmpty() ) return;

    ui->statusBar->showMessage( QString::number( mousePosition.x(), 'f', 0) + "x" + QString::number( mousePosition.y(), 'f', 0) );
    ui->statusBar->update();
}
