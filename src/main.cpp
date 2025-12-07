#include <QApplication>
#include "MainWindow.h"

#include <QStyleFactory>
#include <QPalette>

#include <QDebug>

int main(int argc, char *argv[]) {
    qDebug() << "Starting application...";
    QApplication app(argc, argv);
    
    // Set Dark Theme (Fusion)
    qDebug() << "Setting style...";
    app.setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    app.setPalette(darkPalette);
    qDebug() << "Style set.";
    
    app.setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");

    QCoreApplication::setOrganizationName("Antigravity");
    QCoreApplication::setApplicationName("SmoothSlideshow");
    
    qDebug() << "Creating MainWindow...";
    MainWindow w;
    w.resize(1024, 768);
    w.show();
    
    qDebug() << "Entering event loop...";
    return app.exec();
}
