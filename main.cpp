#include "mainwindow.h"
#include "devicefinder.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    DeviceFinder finder;

    return a.exec();
}
