// mainwindow_style.cpp
#include <QApplication>
#include <QToolTip>
#include <QDebug>

#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "global.h"
#include "deviceconfig.h"

#include "axesextended.h"
#include "axesconfig.h"

#ifdef Q_OS_WIN
#include <dwmapi.h>
#pragma comment (lib,"Dwmapi.lib")
enum : WORD {
    DwmwaUseImmersiveDarkMode = 20,
    DwmwaUseImmersiveDarkModeBefore20h1 = 19
};
static bool setDarkBorderToWindow(HWND hwnd, bool dark)
{
    const BOOL darkBorder = dark ? TRUE : FALSE;
    const bool ok =
        SUCCEEDED(DwmSetWindowAttribute(hwnd, DwmwaUseImmersiveDarkMode, &darkBorder, sizeof(darkBorder))) ||
        SUCCEEDED(DwmSetWindowAttribute(hwnd, DwmwaUseImmersiveDarkModeBefore20h1, &darkBorder, sizeof(darkBorder)));
    return ok;
}
#endif

// Dark-only. No global qApp stylesheet. Skip Logical Buttons subtree for performance.
void MainWindow::themeChanged(bool /*dark*/)
{
    // ---------- Dark palette ----------
    QPalette pal;
    pal.setColor(QPalette::Window, QColor(94,97,105));            // FS36321
    pal.setColor(QPalette::Button, QColor(36,39,49,80));          // semi over Window → lighter tile
    pal.setColor(QPalette::Disabled, QPalette::Button, QColor(36,39,49,80));
    pal.setColor(QPalette::Base, QColor(36,39,49));
    pal.setColor(QPalette::Disabled, QPalette::Base, QColor(35,36,40));
    pal.setColor(QPalette::AlternateBase, QColor(66,66,66));
    pal.setColor(QPalette::ToolTipBase, QColor(75,76,77));

    pal.setColor(QPalette::Dark, QColor(66,66,66));
    pal.setColor(QPalette::Light, QColor(66,66,66));
    pal.setColor(QPalette::Shadow, QColor(20,20,20));

    pal.setColor(QPalette::Text, QColor(255,255,255));
    pal.setColor(QPalette::Disabled, QPalette::Text, QColor(127,127,127));
    pal.setColor(QPalette::WindowText, QColor(255,255,255));
    pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127,127,127));
    pal.setColor(QPalette::ToolTipText, QColor(230,231,232));
    pal.setColor(QPalette::ButtonText, QColor(255,255,255));
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127,127,127));
    pal.setColor(QPalette::BrightText, Qt::red);
    pal.setColor(QPalette::Link, QColor(5,170,61));
    pal.setColor(QPalette::Highlight, QColor(5,170,61));
    pal.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80,80,80));
    pal.setColor(QPalette::HighlightedText, QColor(255,255,255));
    pal.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127,127,127));

    QToolTip::setPalette(pal);
    qApp->setPalette(pal);

#ifdef Q_OS_WIN
    setDarkBorderToWindow(reinterpret_cast<HWND>(window()->winId()), true);
#endif

    // ---------- GroupBox chrome (unchanged) ----------
    static const QString kDarkBox = QStringLiteral(
        "QGroupBox{"
        " font-weight:bold; background:rgb(36,39,49);"
        " border:1px solid rgb(255,255,255); border-radius:3px;"
        " margin-top:2ex; padding:6px 0 0 0; }"
        "QGroupBox::title{ color:#fff; subcontrol-position:top center;"
        " subcontrol-origin:margin; left:-3px; padding:-2px 0 0 2px; background:transparent; }"
        );

    // Re-collect every time (fast & avoids stale cache)
    const auto gbs = window()->findChildren<QGroupBox*>();
    for (QGroupBox* gb : gbs) {
        const QString obj = gb->objectName();
        if (obj == QLatin1String("groupBox_LogicalButtons")) continue; // perf
        const bool isAxisBox = (obj == QLatin1String("groupBox_AxixName"));
        gb->setStyleSheet(kDarkBox + (isAxisBox ? QStringLiteral("QGroupBox{padding:8px;}") : QString()));
    }

    // ---------- Enhanced CSS with !important for better override ----------
    static const QString kIndicatorCss =
        "QCheckBox::indicator, QRadioButton::indicator {"
        " width:12px; height:12px; border:1px solid rgb(66,66,66); }"
        "QCheckBox::indicator:unchecked, QRadioButton::indicator:unchecked {"
        " background-color: rgb(76,79,88) !important; }"
        "QCheckBox::indicator:checked, QRadioButton::indicator:checked {"
        " background-color: rgb(5,170,61) !important; border:1px solid rgb(66,66,66); }";

    static const QString kSpinCss =
        "QSpinBox, QDoubleSpinBox {"
        " background-color: rgb(76,79,88) !important;"
        " color: white !important;"
        " border: 1px solid rgb(66,66,66);"
        " border-radius: 2px;"
        " padding: 2px 6px;"
        " selection-background-color: rgb(5,170,61); }"
        "QSpinBox:focus, QDoubleSpinBox:focus {"
        " border: 1px solid rgb(5,170,61); }"
        "QSpinBox::up-button, QSpinBox::down-button,"
        "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
        " background-color: rgb(76,79,88) !important;"
        " border: none;"
        " width: 14px; }"
        "QSpinBox::up-button:hover, QSpinBox::down-button:hover,"
        "QDoubleSpinBox::up-button:hover, QDoubleSpinBox::down-button:hover {"
        " background-color: rgb(66,66,66) !important; }";

    // ---------- Create custom palette for spinboxes ----------
    QPalette spinPalette = pal;
    spinPalette.setColor(QPalette::Base, QColor(76,79,88));
    spinPalette.setColor(QPalette::Window, QColor(76,79,88));
    spinPalette.setColor(QPalette::Button, QColor(76,79,88));

    // ---------- Apply per-widget where groupbox scope doesn't reach ----------

    // 1) Hidden-axes checkboxes live directly under AxesConfig (not in a groupbox)
    if (m_axesConfig) {
        const auto hiddenCbs = m_axesConfig->findChildren<QCheckBox*>();
        for (QCheckBox* cb : hiddenCbs) {
            cb->setStyleSheet(QString());         // clear any stale sheet first
            cb->setStyleSheet(kIndicatorCss);     // per-widget wins
        }
        m_axesConfig->update();
        qDebug() << "Styled checkboxes in AxesConfig:" << hiddenCbs.size();
    }

    // 2) AxesExtended panels contain spinboxes/indicators (own subtree) - Enhanced approach
    const auto exts = findChildren<AxesExtended*>();
    qDebug() << "Found AxesExtended panels:" << exts.size();

    for (AxesExtended* ax : exts) {
        // Clear any existing stylesheets from the entire subtree first
        const auto allWidgets = ax->findChildren<QWidget*>();
        for (QWidget* widget : allWidgets) {
            widget->setStyleSheet("");
        }

        // Apply spinbox styling with multiple approaches for maximum compatibility
        const auto spinBoxes = ax->findChildren<QSpinBox*>();
        const auto doubleSpinBoxes = ax->findChildren<QDoubleSpinBox*>();

        qDebug() << "  Panel" << ax->objectName() << "- SpinBoxes:" << spinBoxes.size()
                 << "DoubleSpinBoxes:" << doubleSpinBoxes.size();

        for (QSpinBox* sp : spinBoxes) {
            // Method 1: Clear and apply stylesheet
            sp->setStyleSheet("");
            sp->setAutoFillBackground(true);
            sp->setStyleSheet(kSpinCss);

            // Method 2: Also apply palette as backup
            sp->setPalette(spinPalette);

            qDebug() << "    Styled SpinBox:" << sp->objectName()
                     << "Stylesheet length:" << sp->styleSheet().length();

            // Force immediate visual update
            sp->update();
            sp->repaint();
        }

        for (QDoubleSpinBox* dsp : doubleSpinBoxes) {
            // Method 1: Clear and apply stylesheet
            dsp->setStyleSheet("");
            dsp->setAutoFillBackground(true);
            dsp->setStyleSheet(kSpinCss);

            // Method 2: Also apply palette as backup
            dsp->setPalette(spinPalette);

            qDebug() << "    Styled DoubleSpinBox:" << dsp->objectName()
                     << "Stylesheet length:" << dsp->styleSheet().length();

            // Force immediate visual update
            dsp->update();
            dsp->repaint();
        }

        // Apply checkbox styling
        for (QCheckBox* cb : ax->findChildren<QCheckBox*>()) {
            cb->setStyleSheet(QString());
            cb->setStyleSheet(kIndicatorCss);
        }

        // Force update of the entire container
        ax->update();
        ax->repaint();
    }

    // Optional: wiki hover accent
    ui->pushButton_Wiki->setStyleSheet(
        "QPushButton#pushButton_Wiki {"
        " padding:0; margin:0;"
        " min-width:150; max-width:150; min-height:60; max-height:60;"
        " width:160; height:70; }"
        "QPushButton#pushButton_Wiki:hover { border-color: rgb(5,170,61); }"
        );
    ui->pushButton_Wiki->setIcon(QIcon(":/Images/ST_wiki_dark.png"));

    updateColor();

    // Debug summary
    qDebug() << "=== Theme Change Complete ===";
    qDebug() << "Total AxesExtended panels processed:" << exts.size();
    qDebug() << "AxesConfig checkboxes:"
             << (m_axesConfig ? m_axesConfig->findChildren<QCheckBox*>().size() : -1);
}
