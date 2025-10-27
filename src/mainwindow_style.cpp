// mainwindow_style.cpp  — palette-only dark theme (no per-widget QSS)

#include <QApplication>
#include <QToolTip>
#include <QPalette>
#include <QIcon>
#include <QDebug>
#include <QScrollArea>

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
    pal.setColor(QPalette::Window,           QColor(157, 162, 168));    // FS36321 base window gray
    pal.setColor(QPalette::Button,           QColor(50,53,63,80)); // semi over Window
    pal.setColor(QPalette::Disabled, QPalette::Button, QColor(36, 39, 49, 80));

    pal.setColor(QPalette::Base,             QColor(36, 39, 49));     // FLAT_BLACK as input backgrounds
    pal.setColor(QPalette::Disabled, QPalette::Base, QColor(35, 36, 40));
    pal.setColor(QPalette::AlternateBase,    QColor(66, 66, 66));     // tables/alternating rows

    // Frames / bevels
    pal.setColor(QPalette::Dark,             QColor(30, 30, 30));    // Much darker
    pal.setColor(QPalette::Light,            QColor(40, 40, 40));    // Darker
    pal.setColor(QPalette::Shadow,           QColor(10, 10, 10));    // Almost black
    pal.setColor(QPalette::Mid,              QColor(35, 35, 35));    // Add mid tone

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

    qApp->setStyleSheet(qApp->styleSheet() + R"(


        QGroupBox {
            background-color: transparent;
            border: 1px solid rgba(255,255,255,0.30);
            margin-top: 1.3em;

        }

        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 8px;
            padding: 0 4px;
            background-color: transparent;
            color: palette(window-text);
        }

        QGroupBox#groupBox_DeviceSettings {
            background-color: rgb(57, 60, 70);
            border: 1px solid rgba(255,255,255,0.25);
            margin-top: -.5em;
        }


        QComboBox {
            background-color: transparent;
            border: 1px solid rgba(255, 255, 255, 0.25);
            padding: 4px;
        }

        QComboBox:hover {
            background-color: transparent;
            border: 1px solid rgba(255, 255, 255, 0.25);
        }

        QComboBox:focus {
            background-color: transparent;
            border: 1px solid rgb(5, 170, 61);  /* INVICTUS GREEN */
        }

        QComboBox QAbstractItemView {
            border: 1px solid rgba(255, 255, 255, 0.05);
            background-color: rgb(57, 60, 70);  /* Dropdown list background */
            selection-background-color: rgb(5, 170, 61);  /* INVICTUS GREEN selection */
        }
        QPushButton {
            background-color: rgb(50,53,63);
            border: 1px solid rgba(255, 255, 255, 0.25);
            padding: 4px;
        }

        QPushButton:hover {
            border: 1px solid rgba(255, 255, 255, 0.35);
        }

        QPushButton:pressed, QPushButton:checked {
            border: 1px solid rgb(5, 170, 61);  /* INVICTUS GREEN */
        }

        /* Remove borders from scroll areas */
        QScrollArea {
            border: none;
        }

        /* QMessageBox and QDialog styling */
        QMessageBox, QDialog {
            background-color: rgb(36, 39, 49);  /* FLAT_BLACK */
        }

        /* Make icon and text labels transparent in message boxes */
        QMessageBox QLabel {
            background-color: transparent;
        }


)");
    {
        // Set scroll areas to match tab widget background color
        const char* kScrollBg = "background-color: rgb(57, 60, 70);"; // Same as tab background

        for (QScrollArea* sa : this->findChildren<QScrollArea*>()) {
            // Set the scroll area itself
            sa->setStyleSheet("QScrollArea { background-color: rgb(57, 60, 70); border: none; }");

            // Don't style the contents widget - let child widgets use their own styles
            if (QWidget* contents = sa->widget()) {
                contents->setAttribute(Qt::WA_StyledBackground, false);
                contents->setAutoFillBackground(false);
                contents->setStyleSheet(""); // Remove any stylesheet
            }

            // Style only the viewport background
            if (QWidget* vp = sa->viewport()) {
                vp->setAutoFillBackground(true);
                vp->setStyleSheet(kScrollBg);
            }
        }
    }
    ui->pushButton_Wiki->setIcon(QIcon(":/Images/ST_wiki_dark.png"));
    updateColor();

#ifdef Q_OS_WIN
    setDarkBorderToWindow((HWND)window()->winId(), true);
#endif
}
