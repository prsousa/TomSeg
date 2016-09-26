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

        colorItem->setData(Qt::BackgroundRole, getColor(seed.id));
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

    showSlice(currentSliceIndex);
}

void MainWindow::on_removeSeedButton_released()
{
    QModelIndexList  selectedRowIndexes = ui->seedsTableWidget->selectionModel()->selectedRows();

    Slice* slice = segManager.getSlice(currentSliceIndex);

    for( int i = selectedRowIndexes.size() - 1; i >= 0; i-- ) {
        int rowIndex = selectedRowIndexes.at(i).row();
        slice->removeSeed(rowIndex); // possible bug: seeds indexes will chage after erase.
    }

    showSlice(currentSliceIndex);
    updateSeedsTable();
}

void MainWindow::on_minimumFeatureSizeSpinBox_valueChanged(int newMinimumFeatureSize)
{
    if( segManager.isEmpty() ) return;

    Slice* slice = segManager.getSlice(currentSliceIndex);
    slice->setMinimumFeatureSize(newMinimumFeatureSize);
}

void MainWindow::drawSeeds()
{
    sliceScene->resetSeedsDisplayer();
    Slice* slice = segManager.getSlice(currentSliceIndex);
    std::vector<Seed> seeds = slice->getSeeds();

    for( int i = 0; i < seeds.size(); i++ ) {
        Seed seed = seeds[i];
        if( seed.active ) {
            QPen pen( getColor(seed.id), 4 );
            sliceScene->addSeed(seed.a.x, seed.a.y, seed.b.x - seed.a.x, seed.b.y - seed.a.y, pen);
        }
    }
}

QPixmap MainWindow::convertSegmentationResult(cv::Mat labels) {
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

void MainWindow::showSlice(int sliceNumber = 0)
{
    qDebug() << "Show Slice";

    currentSliceIndex = sliceNumber;
    ui->currentSliceNumberSpinner->setValue( sliceNumber + 1 );

    Slice* slice = segManager.getSlice(sliceNumber);

    QPixmap image( slice->getFilename().data() );
    this->slicePixmapItem = sliceScene->setSlicePixmap( image );

    QPixmap segmentationResult = convertSegmentationResult(slice->getSegmentationResult());
    QGraphicsPixmapItem* r = sliceScene->setResultPixmap( segmentationResult );

    drawSeeds();

    ui->sliceView->fitInView(slicePixmapItem, Qt::KeepAspectRatio);

    ui->currentSliceWidthLabel->setText( QString::number( image.width() ) );
    ui->currentSliceHeightLabel->setText( QString::number( image.height() ) );
    ui->minimumFeatureSizeSpinBox->setValue( slice->getMinimumFeatureSize() );
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

        ui->currentSliceNumberSpinner->setMaximum( filenames.size() );
        ui->currentSliceNumberSpinner->setMinimum( 1 );
        ui->sliceTotalLabel->setText( QString::number(filenames.size()) );

        ui->seedsTableWidget->setEnabled(true);
        ui->addSeedButton->setEnabled(true);

        // showSlice();
        updateSeedsTable();
    }
}

void MainWindow::on_actionOpen_triggered()
{
    openFileDialog();
}

void MainWindow::on_nextSliceButton_released()
{
    if( !segManager.isEmpty() ) {
        currentSliceIndex = std::min( currentSliceIndex + 1, (int) segManager.size() - 1 );
        showSlice(currentSliceIndex);
        updateSeedsTable();
    }
}

void MainWindow::on_previousSliceButton_released()
{
    if( !segManager.isEmpty() ) {
        currentSliceIndex = std::max( 0, currentSliceIndex - 1);
        showSlice(currentSliceIndex);
        updateSeedsTable();
    }
}

void MainWindow::on_currentSliceNumberSpinner_valueChanged(int newSliceNumber)
{
    if( newSliceNumber > 0 && newSliceNumber <= segManager.size()) {
        currentSliceIndex = newSliceNumber - 1;
        showSlice(currentSliceIndex);
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

    showSlice(currentSliceIndex);
}

void MainWindow::on_resetButton_released()
{
    if( !segManager.isEmpty() ) {
        Slice* slice = segManager.getSlice(currentSliceIndex);
        slice->resetSegmentationResult();
        showSlice(currentSliceIndex);
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
       ui->sliceView->fitInView(slicePixmapItem, Qt::KeepAspectRatio);
   }
}

void MainWindow::seedCreated( float x, float y, float width, float height )
{
    Slice* slice = segManager.getSlice(currentSliceIndex);
    std::vector<Seed>& seeds = slice->getSeeds();

    Seed newSeed( slice->getImg(), seeds.size(), Point(x, y), Point(x + width, y + height) );
    seeds.push_back(newSeed);

    showSlice(currentSliceIndex);
    updateSeedsTable();
}

void MainWindow::sliceSceneMouseMoved(QPointF mousePosition)
{
    ui->statusBar->showMessage( QString::number( mousePosition.x(), 'f', 0) + "x" + QString::number( mousePosition.y(), 'f', 0) );
    ui->statusBar->update();
}
