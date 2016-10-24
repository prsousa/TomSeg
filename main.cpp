#include "mainwindow.h"
#include "cliapplication.h"
#include <QApplication>

#define GUI 1

int main(int argc, char *argv[])
{
#if GUI
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
#else
    CliApplication cliApp(argc, argv);
    return cliApp.exec();
#endif
}
