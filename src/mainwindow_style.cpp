// mainwindow_style.cpp  — palette-only dark theme (no per-widget QSS)

#include <QApplication>
#include <QToolTip>
#include <QPalette>
#include <QIcon>
#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"

// If you want dark Windows title bar (optional)
#ifdef Q_OS_WIN
#include <dwmapi.h>
#pragma comment (lib,"Dwmapi.lib")

enum : WORD {
    DwmwaUseImmersiveDarkMode           = 20,
    DwmwaUseImmersiveDarkModeBefore20h1 = 19
};
static bool setDarkBorderToWindow(HWND hwnd, bool dark)
{
    const BOOL darkBorder = dark ? TRUE : FALSE;
    const bool ok =
        SUCCEEDED(DwmSetWindowAttribute(hwnd, DwmwaUseImmersiveDarkMode, &darkBorder, sizeof(darkBorder))) ||
        SUCCEEDED(DwmSetWindowAttribute(hwnd, DwmwaUseImmersiveDarkModeBefore20h1, &darkBorder, sizeof(darkBorder)));
    if (!ok) {
        qDebug() << __FUNCTION__ << ": Unable to set" << (dark ? "dark" : "light") << "window border.";
    }
    return ok;
}
#endif

// Kept the same signature so you don't have to touch headers/slots.
// The 'dark' flag is ignored—this always applies your single dark palette.
void MainWindow::themeChanged(bool /*dark*/)
{
    // ---- Global palette (Fusion reads these roles) ----
    // Window gray and FLAT_BLACK / INVICTUS_GREEN accents only via palette.
    QPalette pal;

    // Window / Base family
    pal.setColor(QPalette::Window,           QColor(94, 97, 105));    // FS36321 base window gray
    pal.setColor(QPalette::Button,           QColor(36, 39, 49, 80)); // semi over Window
    pal.setColor(QPalette::Disabled, QPalette::Button, QColor(36, 39, 49, 80));

    pal.setColor(QPalette::Base,             QColor(36, 39, 49));     // FLAT_BLACK as input backgrounds
    pal.setColor(QPalette::Disabled, QPalette::Base, QColor(35, 36, 40));
    pal.setColor(QPalette::AlternateBase,    QColor(66, 66, 66));     // tables/alternating rows

    // Frames / bevels
    pal.setColor(QPalette::Dark,             QColor(66, 66, 66));
    pal.setColor(QPalette::Light,            QColor(66, 66, 66));
    pal.setColor(QPalette::Shadow,           QColor(20, 20, 20));

    // Text / links / selection
    pal.setColor(QPalette::Text,             QColor(255, 255, 255));
    pal.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    pal.setColor(QPalette::WindowText,       QColor(255, 255, 255));
    pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    pal.setColor(QPalette::ButtonText,       QColor(255, 255, 255));
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));

    pal.setColor(QPalette::Link,             QColor(5, 170, 61));     // INVICTUS_GREEN
    pal.setColor(QPalette::Highlight,        QColor(5, 170, 61));     // selection fill
    pal.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    pal.setColor(QPalette::HighlightedText,  QColor(255, 255, 255));
    pal.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

    // Tooltips
    pal.setColor(QPalette::ToolTipBase,      QColor(75, 76, 77));
    pal.setColor(QPalette::ToolTipText,      QColor(230, 231, 232));

    // Apply globally
    QToolTip::setPalette(pal);
    qApp->setPalette(pal);

    // No per-widget setStyleSheet calls.
    // If anything in the UI still looks “immune”, it means that widget (or a parent)
    // still has a local stylesheet set somewhere; clear it in Designer or in code.

    // Set the dark icon variant; no stylesheet needed
    ui->pushButton_Wiki->setIcon(QIcon(":/Images/ST_wiki_dark.png"));

    // Sync monochrome icons to current text color
    updateColor();

#ifdef Q_OS_WIN
    setDarkBorderToWindow((HWND)window()->winId(), true);
#endif
}
