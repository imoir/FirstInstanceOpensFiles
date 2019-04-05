#include "mainwindow.h"
#include <QApplication>
#include "RunGuard.h"

int main(int argc, char *argv[])
{
    RunGuard runGuard("FirstInstanceOpensFiles");
    if ( !runGuard.tryToRun() ) {
        if ( argc > 1 ) {
            runGuard.sendFileToOpen(argv[1]);
        }
        return 0;
    }
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    runGuard.runFileOpenThread();
    QObject::connect(&runGuard, &RunGuard::OpenFileReceivedEvent, &w, &MainWindow::OpenFile);

    return a.exec();
}
