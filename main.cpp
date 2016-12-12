#define GUI 0

#if GUI
#include "mainwindow.h"
#include <QApplication>
#else
#include "cliapplication.h"
#endif

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
