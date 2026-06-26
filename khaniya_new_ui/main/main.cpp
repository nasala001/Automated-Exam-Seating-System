#include <QApplication>
#include <QFont>
#include <QPalette>
#include <QStyleFactory>
#include "../include/mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Force Fusion style + light palette to ignore macOS system dark mode
    app.setStyle(QStyleFactory::create("Fusion"));
    QPalette lightPal;
    lightPal.setColor(QPalette::Window,          QColor("#f5f7fa"));
    lightPal.setColor(QPalette::WindowText,       QColor("#1a2332"));
    lightPal.setColor(QPalette::Base,             QColor("#ffffff"));
    lightPal.setColor(QPalette::AlternateBase,    QColor("#f8fafc"));
    lightPal.setColor(QPalette::ToolTipBase,      QColor("#003366"));
    lightPal.setColor(QPalette::ToolTipText,      QColor("#ffffff"));
    lightPal.setColor(QPalette::Text,             QColor("#1a2332"));
    lightPal.setColor(QPalette::Button,           QColor("#f0f4fa"));
    lightPal.setColor(QPalette::ButtonText,       QColor("#1a2332"));
    lightPal.setColor(QPalette::BrightText,       QColor("#8b0000"));
    lightPal.setColor(QPalette::Link,             QColor("#003366"));
    lightPal.setColor(QPalette::Highlight,        QColor("#dce8f7"));
    lightPal.setColor(QPalette::HighlightedText,  QColor("#003366"));
    lightPal.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#9ab0c8"));
    lightPal.setColor(QPalette::Disabled, QPalette::Text,       QColor("#9ab0c8"));
    lightPal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#9ab0c8"));
    app.setPalette(lightPal);

    // Application-wide font
    QFont appFont("Segoe UI", 10);
    appFont.setStyleHint(QFont::SansSerif);
    app.setFont(appFont);

    // Application metadata
    app.setApplicationName("KU Examination Seating System");
    app.setOrganizationName("Kathmandu University");
    app.setApplicationVersion("1.0.0");

    // Global stylesheet for basic Qt widgets
    app.setStyleSheet(R"(
        QWidget {
            font-family: 'Segoe UI', Arial, sans-serif;
        }
        QMainWindow {
            background-color: #f5f7fa;
        }
        QScrollBar:vertical {
            background: #f0f4fa;
            width: 8px;
            border-radius: 4px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: #9ab0c8;
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #003366;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical { height: 0px; }
        QScrollBar:horizontal {
            background: #f0f4fa;
            height: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal {
            background: #9ab0c8;
            border-radius: 4px;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #003366;
        }
        QToolTip {
            background-color: #003366;
            color: white;
            border: 1px solid #1a4a7a;
            border-radius: 4px;
            padding: 5px 8px;
            font-size: 12px;
        }
        QSplitter::handle {
            background: #d0d9e8;
        }
    )");

    MainWindow w;
    w.show();
    return app.exec();
}
