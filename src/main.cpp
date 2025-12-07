#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    QCoreApplication::setOrganizationName("Antigravity");
    QCoreApplication::setApplicationName("SmoothSlideshow");
    
    MainWindow w;
    w.resize(1024, 768);
    w.show();
    
    return app.exec();
}
