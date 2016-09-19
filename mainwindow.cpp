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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    sliceScene = new MyQGraphicsScene(this);
    ui->sliceView->setScene(sliceScene);

    QObject::connect(sliceScene, SIGNAL(drawnRectangle(float, float, float, float)),
                         this, SLOT(seedCreated(float, float, float, float)));

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

    QVector<SeedInfo> seeds = slices[currentSliceIndex].seedInfos;
    std::vector<SeedInfo> seedsSegManager;

    for(int i = 0; i < seeds.size(); i++) {
        SeedInfo seedInfo = seeds[i];

        QTableWidgetItem* colorItem = new QTableWidgetItem();
        QTableWidgetItem* xItem = new QTableWidgetItem();
        QTableWidgetItem* yItem = new QTableWidgetItem();
        QTableWidgetItem* widthItem = new QTableWidgetItem();
        QTableWidgetItem* heightItem = new QTableWidgetItem();

        colorItem->setData(Qt::BackgroundRole, seedInfo.color);
        colorItem->setCheckState( seedInfo.active ? Qt::Checked : Qt::Unchecked );
        xItem->setText( QString::number(seedInfo.x) );
        yItem->setText( QString::number(seedInfo.y) );
        widthItem->setText( QString::number(seedInfo.width) );
        heightItem->setText( QString::number(seedInfo.height) );

        ui->seedsTableWidget->insertRow(i);

        ui->seedsTableWidget->setItem(i, 0, colorItem);
        ui->seedsTableWidget->setItem(i, 1, xItem);
        ui->seedsTableWidget->setItem(i, 2, yItem);
        ui->seedsTableWidget->setItem(i, 3, widthItem);
        ui->seedsTableWidget->setItem(i, 4, heightItem);

        seedsSegManager.push_back(seedInfo);
    }

    segManager.setSliceSeeds(currentSliceIndex, seedsSegManager);
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

    QVector<SeedInfo>& seeds = slices[currentSliceIndex].seedInfos;
    SeedInfo& info = seeds[rowIndex];

    switch (colIndex) {
    case 0:
        info.active = item->checkState() == Qt::Checked;
        break;
    case 1:
        info.x = item->text().toInt();
        break;
    case 2:
        info.y = item->text().toInt();
        break;
    case 3:
        info.width = item->text().toInt();
        break;
    case 4:
        info.height = item->text().toInt();
        break;
    default:
        break;
    }

    showSlice(currentSliceIndex);
}

void MainWindow::on_removeSeedButton_released()
{
    QModelIndexList  selectedRowIndexes = ui->seedsTableWidget->selectionModel()->selectedRows();

    QVector<SeedInfo>& seeds = slices[currentSliceIndex].seedInfos;

    for( int i = 0; i < selectedRowIndexes.size(); i++) {
        QModelIndex rowIndex = selectedRowIndexes.at(i);
        seeds.erase( seeds.begin() + rowIndex.row());
    }

    showSlice(currentSliceIndex);
    updateSeedsTable();
}

void MainWindow::drawSeeds()
{
    sliceScene->resetSeedsDisplayer();
    QVector<SeedInfo>& seeds = slices[currentSliceIndex].seedInfos;

    for( int i = 0; i < seeds.size(); i++ ) {
        SeedInfo seedInfo = seeds[i];
        if( seedInfo.active ) {
            QPen pen(seedInfo.color, 4);
            sliceScene->addSeed(seedInfo.x, seedInfo.y, seedInfo.width, seedInfo.height, pen);
        }
    }
}

void MainWindow::showSlice(int sliceNumber = 0)
{
    currentSliceIndex = sliceNumber;
    ui->currentSliceNumberSpinner->setValue( sliceNumber + 1 );

    SliceInfo& slice = slices[sliceNumber];

    QGraphicsPixmapItem* p = sliceScene->setSlicePixmap( slice.image );
    QGraphicsPixmapItem* r = sliceScene->setResultPixmap( slice.segmentationResult );

    drawSeeds();

    ui->sliceView->fitInView(p, Qt::KeepAspectRatio);

    ui->currentSliceWidthLabel->setText( QString::number( slice.image.width() ) );
    ui->currentSliceHeightLabel->setText( QString::number( slice.image.height() ) );
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
        slices.clear();

        std::vector<std::string> filenamesSegManager;
        for (int i = 0; i < filenames.count(); i++) {
            QString filename = filenames[i];
            this->slices.push_back( SliceInfo(filename) );
            filenamesSegManager.push_back( filename.toStdString() );
        }

        segManager.setSlices( filenamesSegManager );

        ui->currentSliceNumberSpinner->setMaximum( filenames.size() );
        ui->currentSliceNumberSpinner->setMinimum( 1 );
        ui->sliceTotalLabel->setText( QString::number(filenames.size()) );

        ui->seedsTableWidget->setEnabled(true);
        ui->addSeedButton->setEnabled(true);

        showSlice();
        updateSeedsTable();
    }
}

void MainWindow::on_actionOpen_triggered()
{
    openFileDialog();
}

void MainWindow::on_nextSliceButton_released()
{
    if( !slices.isEmpty() ) {
        currentSliceIndex = std::min( currentSliceIndex + 1, slices.size() - 1 );
        showSlice(currentSliceIndex);
        updateSeedsTable();
    }
}

void MainWindow::on_previousSliceButton_released()
{
    if( !slices.isEmpty() ) {
        currentSliceIndex = std::max( 0, currentSliceIndex - 1);
        showSlice(currentSliceIndex);
        updateSeedsTable();
    }
}

void MainWindow::on_currentSliceNumberSpinner_valueChanged(int newSliceNumber)
{
    if( newSliceNumber > 0 && newSliceNumber <= slices.size()) {
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
    if( slices.isEmpty() ) {
        messageBox.critical(this, "Error", "You must open the slice(s) first!");
        return;
    }

    SliceInfo& sliceInfo = slices[currentSliceIndex];
    if( sliceInfo.seedInfos.size() < 2 ) {
        messageBox.critical(this, "Error", "You must define at least two seeds");
        return;
    }

    //SegmenterThread* segThread = new SegmenterThread(&this->segManager, currentSliceIndex); // TODO: memory leak on thread's object
    //segThread->start();
    int* labels = segManager.apply(currentSliceIndex);

    QPixmap& image = sliceInfo.image;
    QImage result(image.width(), image.height(), QImage::Format_RGB888);

    for( int y = 0; y < result.height(); y++ ) {
        for( int x = 0; x < result.width(); x++ ) {
            int label = labels[ y*result.width() + x ];
            QColor color = sliceInfo.seedInfos[label].color;
            result.setPixel(x, y, color.rgb());
        }
    }

    sliceInfo.segmentationResult = QPixmap::fromImage(result);

    // sliceScene->setResultPixmap(QPixmap::fromImage(result));

    delete labels;
    showSlice(currentSliceIndex);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
   QMainWindow::resizeEvent(event);

   if ( !slices.empty() ){
       showSlice(currentSliceIndex);
   }
}

void MainWindow::seedCreated( float x, float y, float width, float height )
{
    QVector<SeedInfo>& seeds = slices[currentSliceIndex].seedInfos;

    SeedInfo newSeed(seeds.size(), x, y, width, height);
    seeds.push_back(newSeed);

    showSlice(currentSliceIndex);
    updateSeedsTable();
}
