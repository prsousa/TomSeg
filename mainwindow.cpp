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
    ui->sliceView->setScene(sliceScene);
    ui->sliceView->setMouseTracking(true);

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

    for( size_t i = 0; i < seeds.size(); i++ ) {
        Seed seed = seeds[i];

        QTableWidgetItem* colorItem = new QTableWidgetItem();
        QTableWidgetItem* xItem = new QTableWidgetItem();
        QTableWidgetItem* yItem = new QTableWidgetItem();
        QTableWidgetItem* widthItem = new QTableWidgetItem();
        QTableWidgetItem* heightItem = new QTableWidgetItem();

        // colorItem->setData(Qt::BackgroundRole, getColor(seed.id));
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

void MainWindow::on_minimumFeatureSizeSpinBox_valueChanged(int newMinimumFeatureSize)
{
    if( segManager.isEmpty() ) return;

    Slice* slice = segManager.getSlice(currentSliceIndex);
    slice->setMinimumFeatureSize(newMinimumFeatureSize);
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
    ui->sliceView->fitInView(sliceScene->getSlicePixmapItem(), Qt::KeepAspectRatio);

    ui->currentSliceWidthLabel->setText( QString::number( image.cols ) );
    ui->currentSliceHeightLabel->setText( QString::number( image.rows ) );
    ui->minimumFeatureSizeSpinBox->setValue( slice->getMinimumFeatureSize() );

    updateSeedsTable();
}

void MainWindow::on_addSeedButton_released()
{
    this->seedCreated(0, 0, 10, 10);
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

        ui->currentSliceNumberSpinner->setMaximum( segManager.size() );
        ui->currentSliceNumberSpinner->setMinimum( 1 );

        ui->sliceSlider->setMaximum( segManager.size() );
        ui->sliceSlider->setMinimum( 1 );

        ui->sliceTotalLabel->setText( QString::number(segManager.size()) );

        ui->seedsTableWidget->setEnabled(true);
        ui->addSeedButton->setEnabled(true);

        this->setCurrentSlice(0);
    }
}

void MainWindow::on_actionOpen_triggered()
{
    openFileDialog();
}

void MainWindow::on_nextSliceButton_released()
{
    if( segManager.size() > (currentSliceIndex + 1) ) {
        setCurrentSlice(++currentSliceIndex);
    }
}

void MainWindow::on_previousSliceButton_released()
{
    if( !segManager.isEmpty() && currentSliceIndex > 0 ) {
        setCurrentSlice(--currentSliceIndex);
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

void MainWindow::on_resetButton_released()
{
    if( !segManager.isEmpty() ) {
        Slice* slice = segManager.getSlice(currentSliceIndex);
        slice->resetSegmentationResult();
        sliceScene->updateResultDisplayer();
    }
}

void MainWindow::on_moreZoomButton_released()
{
    ui->sliceView->scale(1.15, 1.15);
}

void MainWindow::on_lessZoomButton_released()
{
    ui->sliceView->scale(0.75, 0.75);
}

void MainWindow::on_resultOpacitySlider_valueChanged( int newOpacity )
{
    sliceScene->getResultItemGroup()->setOpacity(newOpacity / 100.f);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    if ( !segManager.isEmpty() ){
       ui->sliceView->fitInView(sliceScene->getSlicePixmapItem(), Qt::KeepAspectRatio);
    }
}

void MainWindow::seedCreated( float x, float y, float width, float height )
{
    Slice* slice = segManager.getSlice(currentSliceIndex);
    std::vector<Seed>& seeds = slice->getSeeds();

    Seed newSeed( slice->getImg(), seeds.size(), Point(x, y), Point(x + width, y + height) );
    seeds.push_back(newSeed);

    sliceScene->updateSeedsDisplayer();
    updateSeedsTable();
}

void MainWindow::sliceSceneMouseMoved(QPointF mousePosition)
{
    ui->statusBar->showMessage( QString::number( mousePosition.x(), 'f', 0) + "x" + QString::number( mousePosition.y(), 'f', 0) );
    ui->statusBar->update();
}
