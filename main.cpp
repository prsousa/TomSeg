// #define CLI

#ifdef CLI
#include "cliapplication.h"
#else
#include "mainwindow.h"
#include <QApplication>
#endif

int main(int argc, char *argv[])
{
#ifdef CLI
    CliApplication cliApp(argc, argv);
    return cliApp.exec();
#else
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
#endif
}
